#include "targets.h"
#include "common.h"
#include "devLED.h"

#ifdef HAS_LED

/* Set red led to default */
#ifndef GPIO_PIN_LED_RED
#ifdef GPIO_PIN_LED
#define GPIO_PIN_LED_RED GPIO_PIN_LED
#else
#define GPIO_PIN_LED_RED UNDEF_PIN
#endif
#endif
#ifndef GPIO_PIN_LED
#define GPIO_PIN_LED GPIO_PIN_LED_RED
#endif
#ifndef GPIO_PIN_LED_GREEN
#define GPIO_PIN_LED_GREEN UNDEF_PIN
#endif
#ifndef GPIO_PIN_LED_BLUE
#define GPIO_PIN_LED_BLUE UNDEF_PIN
#endif
#ifndef GPIO_LED_RED_INVERTED
#define GPIO_LED_RED_INVERTED 0
#endif
#ifndef GPIO_LED_GREEN_INVERTED
#define GPIO_LED_GREEN_INVERTED 0
#endif
#ifndef GPIO_LED_BLUE_INVERTED
#define GPIO_LED_BLUE_INVERTED 0
#endif
#endif