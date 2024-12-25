#include "devBaro.h"

#if defined(OPT_HAS_THERMAL)

#include "logging.h"
#include "baro_spl06.h"
#include "baro_bmp280.h"
// #include "baro_bmp085.h"

#define BARO_STARTUP_INTERVAL       100

/* Shameful externs */

/* Local statics */
static BaroBase *baro;
static eBaroReadState BaroReadState;

extern bool i2c_enabled;

static bool Baro_Detect()
{
    // I2C Baros
#if defined(USE_I2C)
    if (i2c_enabled)
    {
        if (SPL06::detect())
        {
            DBGLN("Detected baro: SPL06");
            baro = new SPL06();
            return true;
        }
        if (BMP280::detect())
        {
            DBGLN("Detected baro: BMP280");
            baro = new BMP280();
            return true;
        }
        // Untested
        // if (BMP085::detect())
        // {
        //     DBGLN("Detected baro: BMP085");
        //     baro = new BMP085();
        //     return true;
        // }
        // DBGLN("No baro detected");
    } // I2C
#endif
    return false;
}

static int Baro_Init()
{
    baro->initialize();
    if (baro->isInitialized())
    {
        // Slow down Vbat updates to save bandwidth
        Vbat_enableSlowUpdate(true);
        BaroReadState = brsReadTemp;
        return DURATION_IMMEDIATELY;
    }

    // Did not init, try again later
    return BARO_STARTUP_INTERVAL;
}

static int start()
{
    if (Baro_Detect())
    {
        BaroReadState = brsUninitialized;
        return BARO_STARTUP_INTERVAL;
    }

    BaroReadState = brsNoBaro;
    return DURATION_NEVER;
}

static int timeout()
{
    if (connectionState >= MODE_STATES)
        return DURATION_NEVER;

    switch (BaroReadState)
    {
        default: // fallthrough
        case brsNoBaro:
            return DURATION_NEVER;

        case brsUninitialized:
            return Baro_Init();

        case brsWaitingTemp:
            {
                int32_t temp = baro->getTemperature();
                if (temp == BaroBase::TEMPERATURE_INVALID)
                    return DURATION_IMMEDIATELY;
            }
            // fallthrough

        case brsReadPres:
            {
                uint8_t pressDuration = baro->getPressureDuration();
                BaroReadState = brsWaitingPress;
                if (pressDuration != 0)
                {
                    baro->startPressure();
                    return pressDuration;
                }
            }
            // fallthrough

        case brsWaitingPress:
            {
                uint32_t press = baro->getPressure();
                if (press == BaroBase::PRESSURE_INVALID)
                    return DURATION_IMMEDIATELY;
                // Baro_PublishPressure(press);
            }
            // fallthrough

        case brsReadTemp:
            {
                uint8_t tempDuration = baro->getTemperatureDuration();
                if (tempDuration == 0)
                {
                    BaroReadState = brsReadPres;
                    return DURATION_IMMEDIATELY;
                }
                BaroReadState = brsWaitingTemp;
                baro->startTemperature();
                return tempDuration;
            }
    }
}

device_t Baro_device = {
    .initialize = nullptr,
    .start = start,
    .event = nullptr,
    .timeout = timeout,
};

#endif