
#pragma once

#include "i2s.h"
#include "marcro.h"
#define IS_I2S_EXPANDER_PIN(IO) TEST(IO, 7)
#define I2S_EXPANDER_PIN_INDEX(IO) (IO & 0x7F)

// Set pin as input
#define _SET_INPUT(IO) pinMode(IO, INPUT)

// Set pin as output
#define _SET_OUTPUT(IO) pinMode(IO, OUTPUT)

// Set pin as input with pullup mode
#define _PULLUP(IO, v) pinMode(IO, v ? INPUT_PULLUP : INPUT)


#define READ(IO) (IS_I2S_EXPANDER_PIN(IO) ? i2s_state(I2S_EXPANDER_PIN_INDEX(IO)) : digitalRead(IO))
#define WRITE(IO, v) (IS_I2S_EXPANDER_PIN(IO) ? i2s_write(I2S_EXPANDER_PIN_INDEX(IO), v) : digitalWrite(IO, v))
// #define READ(IO) (digitalRead(IO))
// #define WRITE(IO, v) (digitalWrite(IO, v))
// Set pin as input wrapper (0x80 | (v << 5) | (IO - 100))
#define SET_INPUT(IO) _SET_INPUT(IO)

// Set pin as input with pullup wrapper
#define SET_INPUT_PULLUP(IO) \
  do                         \
  {                          \
    _SET_INPUT(IO);          \
    _PULLUP(IO, HIGH);       \
  } while (0)

// Set pin as input with pulldown (substitution)
#define SET_INPUT_PULLDOWN SET_INPUT

// Set pin as output wrapper
#define SET_OUTPUT(IO) \
  do                   \
  {                    \
    _SET_OUTPUT(IO);   \
  } while (0)

// Set pin as PWM
#define SET_PWM SET_OUTPUT

// Set pin as output and init
#define OUT_WRITE(IO, V) \
  do                     \
  {                      \
    _SET_OUTPUT(IO);     \
    WRITE(IO, V);        \
  } while (0)

// digitalRead/Write wrappers
#define extDigitalRead(IO) digitalRead(IO)
#define extDigitalWrite(IO, V) digitalWrite(IO, V)

// PWM outputs
#define PWM_PIN(P) (P < 34 || P > 127) // NOTE Pins >= 34 are input only on ESP32, so they can't be used for output.

// Toggle pin value
#define TOGGLE(IO) WRITE(IO, !READ(IO))

//
// Ports and functions
//

