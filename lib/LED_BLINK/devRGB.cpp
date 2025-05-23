#include "targets.h"
#include "common.h"
#include "devLED.h"
#ifdef HAS_RGB
static uint8_t pixelCount;
static uint8_t *statusLEDs;
static uint8_t statusLEDcount;

static uint8_t *bootLEDs;
static uint8_t bootLEDcount;

#if defined(PLATFORM_ESP32)
#include "esp32rgb.h"
static ESP32S3LedDriverGRB *stripgrb;
static ESP32S3LedDriverRGB *striprgb;
#else
#include <NeoPixelBus.h>
#define METHOD NeoEsp8266Uart1800KbpsMethod
static NeoPixelBus<NeoGrbFeature, METHOD> *stripgrb;
static NeoPixelBus<NeoRgbFeature, METHOD> *striprgb;
#endif

typedef struct
{
    uint8_t h, s, v;
} blinkyColor_t;

static blinkyColor_t blinkyColor;
void WS281Binit()
{
    if (OPT_WS2812_IS_GRB)
    {
#if defined(PLATFORM_ESP32)
        stripgrb = new ESP32S3LedDriverGRB(pixelCount, GPIO_PIN_LED_WS2812);
#else
        stripgrb = new NeoPixelBus<NeoGrbFeature, METHOD>(pixelCount, GPIO_PIN_LED_WS2812);
#endif
        stripgrb->Begin();
        stripgrb->ClearTo(RgbColor(0), 0, pixelCount - 1);
        stripgrb->Show();
    }
    else
    {
#if defined(PLATFORM_ESP32)
        striprgb = new ESP32S3LedDriverRGB(pixelCount, GPIO_PIN_LED_WS2812);
#else
        striprgb = new NeoPixelBus<NeoRgbFeature, METHOD>(pixelCount, GPIO_PIN_LED_WS2812);
#endif
        striprgb->Begin();
        striprgb->ClearTo(RgbColor(0), 0, pixelCount - 1);
        striprgb->Show();
    }
}

void WS281BsetLED(int index, uint32_t color)
{
    if (OPT_WS2812_IS_GRB)
    {
        stripgrb->SetPixelColor(index, RgbColor(color >> 16, color >> 8, color));
    }
    else
    {
        striprgb->SetPixelColor(index, RgbColor(color >> 16, color >> 8, color));
    }
}

void WS281BsetLED(uint32_t color)
{
    for (int i = 0; i < statusLEDcount; i++)
    {
        if (OPT_WS2812_IS_GRB)
        {
            stripgrb->SetPixelColor(statusLEDs[i], RgbColor(color >> 16, color >> 8, color));
        }
        else
        {
            striprgb->SetPixelColor(statusLEDs[i], RgbColor(color >> 16, color >> 8, color));
        }
    }
    if (OPT_WS2812_IS_GRB)
    {
        stripgrb->Show();
    }
    else
    {
        striprgb->Show();
    }
}

uint32_t HsvToRgb(const blinkyColor_t &blinkyColor)
{
    uint8_t region, remainder, p, q, t;

    if (blinkyColor.s == 0)
    {
        return blinkyColor.v << 16 | blinkyColor.v << 8 | blinkyColor.v;
    }

    region = blinkyColor.h / 43;
    remainder = (blinkyColor.h - (region * 43)) * 6;

    p = (blinkyColor.v * (255 - blinkyColor.s)) >> 8;
    q = (blinkyColor.v * (255 - ((blinkyColor.s * remainder) >> 8))) >> 8;
    t = (blinkyColor.v * (255 - ((blinkyColor.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:
        return blinkyColor.v << 16 | t << 8 | p;
    case 1:
        return q << 16 | blinkyColor.v << 8 | p;
    case 2:
        return p << 16 | blinkyColor.v << 8 | t;
    case 3:
        return p << 16 | q << 8 | blinkyColor.v;
    case 4:
        return t << 16 | p << 8 | blinkyColor.v;
    default:
        return blinkyColor.v << 16 | p << 8 | q;
    }
}

void brightnessFadeLED(blinkyColor_t &blinkyColor, uint8_t start, uint8_t end)
{
    static uint8_t lightness = 0;
    static int8_t dir = 1;

    if (lightness <= start)
    {
        lightness = start;
        dir = 1;
    }
    else if (lightness >= end)
    {
        lightness = end;
        dir = -1;
    }

    lightness += dir;

    blinkyColor.v = lightness;
    WS281BsetLED(HsvToRgb(blinkyColor));
}

void hueFadeLED(blinkyColor_t &blinkyColor, uint16_t start, uint16_t end, uint8_t lightness, uint8_t count)
{
    static bool hueMode = true;

    if (!hueMode)
    {
        blinkyColor.v--;
        if (blinkyColor.v == 0)
        {
            hueMode = true;
        }
        WS281BsetLED(HsvToRgb(blinkyColor));
    }
    else
    {
        static uint16_t hue = 0;
        static int8_t dir = 1;
        if (start < end)
        {
            if (hue <= start)
            {
                hue = start;
                dir = 1;
            }
            else if (hue >= end)
            {
                hue = end;
                dir = -1;
            }
        }
        else
        {
            if (hue >= start)
            {
                hue = start;
                dir = -1;
            }
            else if (hue <= end)
            {
                hue = end;
                dir = 1;
            }
        }

        blinkyColor.h = hue % 256;
        blinkyColor.v = lightness;
        WS281BsetLED(HsvToRgb(blinkyColor));
        hue += dir;
        if (count != 0 && hue == start)
        {
            static uint8_t counter = 0;
            counter++;
            if (counter >= count)
            {
                counter = 0;
                hueMode = false;
            }
        }
    }
}

uint16_t flashLED(blinkyColor_t &blinkyColor, uint8_t onLightness, uint8_t offLightness, const uint8_t durations[], uint8_t durationCounts)
{
    static int counter = 0;

    blinkyColor.v = counter % 2 == 0 ? onLightness : offLightness;
    WS281BsetLED(HsvToRgb(blinkyColor));
    if (counter >= durationCounts)
    {
        counter = 0;
    }
    return durations[counter++] * 10;
}

uint32_t toRGB(uint8_t c)
{
    uint32_t r = c & 0xE0;
    r = ((r << 16) + (r << 13) + (r << 10)) & 0xFF0000;
    uint32_t g = c & 0x1C;
    g = ((g << 11) + (g << 8) + (g << 5)) & 0xFF00;
    uint32_t b = ((c & 0x3) << 1) + ((c & 0x3) >> 1);
    b = (b << 5) + (b << 2) + (b >> 1);
    return r + g + b;
}

void setButtonColors(uint8_t b1, uint8_t b2)
{
#if defined(PLATFORM_ESP32) && defined(TARGET_TX)
    if (USER_BUTTON_LED != -1)
    {
        WS281BsetLED(USER_BUTTON_LED, toRGB(b1));
    }
    if (USER_BUTTON2_LED != -1)
    {
        WS281BsetLED(USER_BUTTON2_LED, toRGB(b2));
    }
#endif
}

static enum {
    STARTUP = 0,
    NORMAL = 1
} blinkyState;

constexpr uint8_t LEDSEQ_RADIO_FAILED[] = {10, 10};                    // 100ms on, 100ms off (fast blink)
constexpr uint8_t LEDSEQ_DISCONNECTED[] = {50, 50};                    // 500ms on, 500ms off
constexpr uint8_t LEDSEQ_NO_CROSSFIRE[] = {10, 100};                   // 1 blink, 1s pause (one blink/s)
constexpr uint8_t LEDSEQ_BINDING[] = {10, 10, 10, 100};                // 2x 100ms blink, 1s pause
constexpr uint8_t LEDSEQ_MODEL_MISMATCH[] = {10, 10, 10, 10, 10, 100}; // 3x 100ms blink, 1s pause
constexpr uint8_t LEDSEQ_UPDATE[] = {20, 5, 5, 5, 5, 40};              // 200ms on, 2x 50ms off/on, 400ms off

#define NORMAL_UPDATE_INTERVAL 50

static int blinkyUpdate()
{
    static constexpr uint8_t hueStepValue = 1;
    static constexpr uint8_t lightnessStep = 5;

    if (pixelCount == 1)
    {
        WS281BsetLED(HsvToRgb(blinkyColor));
    }
    else
    {
        blinkyColor_t c = blinkyColor;
        for (int i = 0; i < bootLEDcount; i++)
        {
            c.h += 16;
            auto color = HsvToRgb(c);
            if (OPT_WS2812_IS_GRB)
            {
                stripgrb->SetPixelColor(bootLEDs[i], RgbColor(color >> 16, color >> 8, color));
            }
            else
            {
                striprgb->SetPixelColor(bootLEDs[i], RgbColor(color >> 16, color >> 8, color));
            }
        }
        if (OPT_WS2812_IS_GRB)
        {
            stripgrb->Show();
        }
        else
        {
            striprgb->Show();
        }
    }
    if ((int)blinkyColor.h + hueStepValue > 255)
    {
        if ((int)blinkyColor.v - lightnessStep < 0)
        {
            blinkyState = NORMAL;
            if (pixelCount != 1)
            {
                if (OPT_WS2812_IS_GRB)
                {
                    stripgrb->ClearTo(RgbColor(0), 0, pixelCount - 1);
                }
                else
                {
                    striprgb->ClearTo(RgbColor(0), 0, pixelCount - 1);
                }
            }
#if defined(TARGET_TX)
            // setButtonColors(config.GetButtonActions(0)->val.color, config.GetButtonActions(1)->val.color);
#endif
            if (OPT_WS2812_IS_GRB)
            {
                stripgrb->Show();
            }
            else
            {
                striprgb->Show();
            }
            return NORMAL_UPDATE_INTERVAL;
        }
        blinkyColor.v -= lightnessStep;
    }
    else
    {
        blinkyColor.h += hueStepValue;
    }
    return 3000 / (256 / hueStepValue);
}

static bool initialize()
{
    if (GPIO_PIN_LED_WS2812 != UNDEF_PIN)
    {
        pixelCount = 1;
        statusLEDcount = WS2812_STATUS_LEDS_COUNT;
        if (statusLEDcount == 0)
        {
            statusLEDs = new uint8_t[1];
            statusLEDs[0] = 0;
            statusLEDcount = 1;
        }
        else
        {
            statusLEDs = new uint8_t[statusLEDcount];
            for (int i = 0; i < statusLEDcount; i++)
            {
                statusLEDs[i] = WS2812_STATUS_LEDS[i];
                pixelCount = max((int)pixelCount, statusLEDs[i] + 1);
            }
        }
        bootLEDcount = WS2812_BOOT_LEDS_COUNT;
        if (bootLEDcount == 0)
        {
            bootLEDs = statusLEDs;
            bootLEDcount = statusLEDcount;
        }
        else
        {
            bootLEDs = new uint8_t[bootLEDcount];
            for (int i = 0; i < bootLEDcount; i++)
            {
                bootLEDs[i] = WS2812_BOOT_LEDS[i];
                pixelCount = max((int)pixelCount, bootLEDs[i] + 1);
            }
        }
        WS281Binit();
        blinkyColor.h = 0;
        blinkyColor.s = 255;
        blinkyColor.v = 128;
    }
    return GPIO_PIN_LED_WS2812 != UNDEF_PIN;
}
static int start()
{
    blinkyState = STARTUP;
#if defined(PLATFORM_ESP32)
    // Only do the blinkies if it was NOT a software reboot
    if (esp_reset_reason() == ESP_RST_SW)
    {
        blinkyState = NORMAL;
        return NORMAL_UPDATE_INTERVAL;
    }
#endif
    return DURATION_IMMEDIATELY;
}
static int timeout()
{
    if (blinkyState == STARTUP && connectionState < FAILURE_STATES)
    {
        return blinkyUpdate();
    }
    switch (connectionState)
    {
    case connected:
        blinkyColor.h = 172;
        return flashLED(blinkyColor, 192, 0, LEDSEQ_UPDATE, sizeof(LEDSEQ_UPDATE));
    case connected_STA:
        blinkyColor.h = 80;blinkyColor.v = 60;
        return flashLED(blinkyColor, 192, 0, LEDSEQ_BINDING, sizeof(LEDSEQ_UPDATE));
    case disconnected_STA:
        blinkyColor.h = 60;blinkyColor.v = 60;
        return flashLED(blinkyColor, 192, 0, LEDSEQ_BINDING, sizeof(LEDSEQ_UPDATE));
    case wifiUpdate:
        hueFadeLED(blinkyColor, 85, 85 - 30, 128, 2); // Yellow->Green cross-fade
        return 5;
    case radioFailed:
        blinkyColor.h = 0;
        return flashLED(blinkyColor, 192, 0, LEDSEQ_RADIO_FAILED, sizeof(LEDSEQ_RADIO_FAILED));
    default:
        return DURATION_NEVER;
    }
}
device_t RGB_device = {
    .initialize = initialize,
    .start = start,
    .event = timeout,
    .timeout = timeout,
    .subscribe = EVENT_CONNECTION_CHANGED | EVENT_ENTER_BIND_MODE | EVENT_EXIT_BIND_MODE};

#endif
