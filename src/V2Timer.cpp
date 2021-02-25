#include "V2Timer.h"

static const IRQn_Type _irq[TC_INST_NUM] = {
  TC0_IRQn,
  TC1_IRQn,
  TC2_IRQn,
  TC3_IRQn,
#if (TC_INST_NUM > 4)
  TC4_IRQn,
  TC5_IRQn,
#endif
};

void V2Timer::Periodic::begin() {
  _tc = (Tc *)g_apTCInstances[TCC_INST_NUM + _id];

  // Clock generator 0 == 120MHz.
  GCLK->PCHCTRL[GCLK_CLKCTRL_IDs[TCC_INST_NUM + _id]].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  while (GCLK->SYNCBUSY.reg > 0)
    ;

  _tc->COUNT16.CTRLA.bit.ENABLE = 0;
  while (_tc->COUNT16.SYNCBUSY.reg != 0)
    ;

  // Enable match frequency mode.
  _tc->COUNT16.WAVE.bit.WAVEGEN = TC_WAVE_WAVEGEN_MFRQ;
  while (_tc->COUNT16.SYNCBUSY.reg != 0)
    ;

  // Enable the compare interrupt.
  _tc->COUNT16.INTENSET.reg     = 0;
  _tc->COUNT16.INTENSET.bit.MC0 = 1;

  // Example:
  // 120MHz/8 == 15Mhz, 500Hz timer == 30000 ticks / period.
  const uint32_t clock = 120000000;
  uint32_t prescaler   = TC_CTRLA_PRESCALER_DIV1;
  uint32_t period      = (clock / 1) / _frequency;
  if (period > 0xffff) {
    prescaler = TC_CTRLA_PRESCALER_DIV8;
    period    = (clock / 8) / _frequency;

  } else if (period > 0xffff) {
    prescaler = TC_CTRLA_PRESCALER_DIV16;
    period    = (clock / 16) / _frequency;

  } else if (period > 0xffff) {
    prescaler = TC_CTRLA_PRESCALER_DIV64;
    period    = (clock / 64) / _frequency;

  } else if (period > 0xffff) {
    prescaler = TC_CTRLA_PRESCALER_DIV256;
    period    = (clock / 256) / _frequency;

  } else if (period > 0xffff) {
    prescaler = TC_CTRLA_PRESCALER_DIV1024;
    period    = (clock / 1024) / _frequency;
  }

  _tc->COUNT16.CTRLA.reg = prescaler | TCC_CTRLA_PRESCSYNC_GCLK;
  while (_tc->COUNT16.SYNCBUSY.reg != 0)
    ;

  _tc->COUNT16.CC[0].reg = period;
  while (_tc->COUNT16.SYNCBUSY.reg != 0)
    ;

  NVIC_EnableIRQ(_irq[_id]);

  _tc->COUNT16.CTRLA.bit.ENABLE = 1;
  while (_tc->COUNT16.SYNCBUSY.reg != 0)
    ;
}

void V2Timer::PWM::begin() {
  _tcc = (Tcc *)g_apTCInstances[_id];

  // Clock generator 0 == 120MHz.
  GCLK->PCHCTRL[GCLK_CLKCTRL_IDs[_id]].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

  _tcc->CTRLA.bit.SWRST = 1;
  while (_tcc->SYNCBUSY.bit.SWRST)
    ;

  _tcc->CTRLA.bit.ENABLE = 0;
  while (_tcc->SYNCBUSY.bit.ENABLE)
    ;

  // Normal PWM
  _tcc->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  while (_tcc->SYNCBUSY.bit.WAVE)
    ;
  while (_tcc->SYNCBUSY.bit.CC0 || _tcc->SYNCBUSY.bit.CC1)
    ;

  // Example:
  // 120MHz, 20 kHz timer == 6000 ticks / period.
  const uint32_t clock = 120000000;
  uint32_t prescaler   = TCC_CTRLA_PRESCALER_DIV1;
  _period              = (clock / 1) / _frequency;
  if (_period > 0xffff) {
    prescaler = TCC_CTRLA_PRESCALER_DIV2;
    _period   = (clock / 2) / _frequency;

  } else if (_period > 0xffff) {
    prescaler = TCC_CTRLA_PRESCALER_DIV4;
    _period   = (clock / 4) / _frequency;

  } else if (_period > 0xffff) {
    prescaler = TCC_CTRLA_PRESCALER_DIV8;
    _period   = (clock / 8) / _frequency;

  } else if (_period > 0xffff) {
    prescaler = TCC_CTRLA_PRESCALER_DIV16;
    _period   = (clock / 16) / _frequency;

  } else if (_period > 0xffff) {
    prescaler = TCC_CTRLA_PRESCALER_DIV64;
    _period   = (clock / 64) / _frequency;

  } else if (_period > 0xffff) {
    prescaler = TCC_CTRLA_PRESCALER_DIV256;
    _period   = (clock / 256) / _frequency;

  } else if (_period > 0xffff) {
    prescaler = TCC_CTRLA_PRESCALER_DIV1024;
    _period   = (clock / 1024) / _frequency;
  }

  _tcc->CTRLA.reg = prescaler | TCC_CTRLA_PRESCSYNC_GCLK;
  while (_tcc->SYNCBUSY.reg != 0)
    ;

  _tcc->PER.reg = _period;
  while (_tcc->SYNCBUSY.bit.PER)
    ;

  // Reset all channels.
  static const uint8_t n_channels[]{6, 4, 3, 2, 2};
  for (uint8_t i = 0; i < n_channels[_id]; i++) {
    _tcc->CC[i].reg = 0;
    while (_tcc->SYNCBUSY.bit.CC0 || _tcc->SYNCBUSY.bit.CC1)
      ;
  }

  // Enable TCC
  _tcc->CTRLA.bit.ENABLE = 1;
  while (_tcc->SYNCBUSY.bit.ENABLE)
    ;
}
