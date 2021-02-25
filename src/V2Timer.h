#pragma once

#include <Arduino.h>
#include <wiring_private.h>

namespace V2Timer {
// // TC3, 1kHz
// V2Timer::Periodic Timer(3, 1000);
//
// void TC3_Handler() {
//  Main.tick();
//  Timer.clear();
// }
//
// Timer.begin();
//
class Periodic {
public:
  constexpr Periodic(uint8_t id, uint32_t frequency) : _id{id}, _frequency{frequency} {}
  void begin();

  void clear() {
    _tc->COUNT16.INTFLAG.bit.MC0 = 1;
  }

private:
  const uint8_t _id{};
  const uint32_t _frequency{};
  Tc *_tc{};
};

// // TCC from pin, 20kHz
// V2Timer::PWM PWM(V2Timer::PWM::getId(PIN_PWM), 20000);
//
// // Switch pin to TCC
// V2Timer::PWM::setupPin(PIN_PWM);
//
// PWM.begin();
//
class PWM {
public:
  constexpr PWM(uint8_t id, uint32_t frequency) : _id{id}, _frequency{frequency} {}
  void begin();

  void setDuty(uint8_t pin, float duty) {
    _tcc->CC[getChannel(pin)].reg = (float)_period * duty;
    while (_tcc->SYNCBUSY.bit.CC0 || _tcc->SYNCBUSY.bit.CC1)
      ;
  }

  static uint8_t getId(uint8_t pin) {
    return GetTCNumber(g_APinDescription[pin].ulTCChannel);
  }

  static uint8_t getChannel(uint8_t pin) {
    return GetTCChannelNumber(g_APinDescription[pin].ulTCChannel);
  }

  static void setupPin(uint8_t pin) {
    pinPeripheral(pin, g_APinDescription[pin].ulPinType);
  }

private:
  const uint8_t _id{};
  const uint32_t _frequency{};
  Tcc *_tcc{};
  uint32_t _period{};
};
};
