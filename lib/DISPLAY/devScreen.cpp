#include "targets.h"

#if defined(PLATFORM_ESP32)

#include "devScreen.h"
#include "common.h"
#include "logging.h"
#include "button.h"
#include "OLED/oleddisplay.h"

Display *display;
static bool initialize()
{
    if (OPT_HAS_SCREEN)
    {
        if (OPT_HAS_TFT_SCREEN)
        {
            // display = new TFTDisplay();
        }
        else
        {
            display = new OLEDDisplay();
        }
        display->init();
    }
    return OPT_HAS_SCREEN;
}
static int start()
{
    if (OPT_HAS_SCREEN)
    {
        return DURATION_IMMEDIATELY;
    }
    return DURATION_NEVER;
}
static int event()
{
    if (OPT_HAS_SCREEN)
    {
        return 100;
    }
    return DURATION_NEVER;
}
static int timeout()
{
    if (OPT_HAS_SCREEN)
    {
        display->display_wifi_status();
        return 500;
    }
    return DURATION_NEVER;
}

device_t Screen_device = {
    .initialize = initialize,
    .start = start,
    .event = event,
    .timeout = timeout};
#endif
