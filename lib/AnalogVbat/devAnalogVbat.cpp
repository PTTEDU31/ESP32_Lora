#include "devAnalogVbat.h"

#if defined(USE_ANALOG_VBAT)
#include <Arduino.h>
#include "median.h"
#include "logging.h"

// Sample 5x samples over 500ms (unless SlowUpdate)
#define VBAT_SMOOTH_CNT 5
#if defined(DEBUG_VBAT_ADC)
#define VBAT_SAMPLE_INTERVAL 20U // faster updates in debug mode
#else
#define VBAT_SAMPLE_INTERVAL 100U
#endif

typedef uint16_t vbatAnalogStorage_t;
static MedianAvgFilter<vbatAnalogStorage_t, VBAT_SMOOTH_CNT> vbatSmooth;
static uint8_t vbatUpdateScale;

#if defined(PLATFORM_ESP32)
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

adc_cali_handle_t adc_cali_handle = NULL;
#endif

/* Shameful externs */
// extern Telemetry telemetry;

/**
 * @brief: Enable SlowUpdate mode to reduce the frequency Vbat telemetry is sent
 ***/
void Vbat_enableSlowUpdate(bool enable)
{
    vbatUpdateScale = enable ? 2 : 1;
}

static int start()
{
    if (GPIO_ANALOG_VBAT == UNDEF_PIN)
    {
        return DURATION_NEVER;
    }
    vbatUpdateScale = 1;
#if defined(PLATFORM_ESP32)
    analogReadResolution(12);

    int atten = hardware_int(HARDWARE_vbat_atten);
    if (atten != -1)
    {
        // if the configured value is higher than the max item (11dB, it indicates to use cal_characterize)
        bool useCal = atten > ADC_11db;
        if (useCal)
        {
            atten -= (ADC_11db + 1);
            int8_t channel = digitalPinToAnalogChannel(GPIO_ANALOG_VBAT);
            adc_unit_t unit = (channel > (SOC_ADC_MAX_CHANNEL_NUM - 1)) ? ADC_UNIT_2 : ADC_UNIT_1;
            adc_cali_curve_fitting_config_t cali_config = {
                .unit_id = unit,
                .chan = (adc_channel_t)channel,
                .atten = (adc_atten_t)atten,
                .bitwidth = ADC_BITWIDTH_12,
            };

            // Tạo handle hiệu chuẩn bằng scheme line-fitting
            esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config,&adc_cali_handle);
        }
        analogSetPinAttenuation(GPIO_ANALOG_VBAT, (adc_attenuation_t)atten);
    }
#endif

    return VBAT_SAMPLE_INTERVAL;
}

static void reportVbat()
{
    int adc = vbatSmooth.calc();

    int vbat;
#if defined(PLATFORM_ESP32) && !defined(DEBUG_VBAT_ADC)
    if (adc_cali_handle)
        // adc = esp_adc_cal_raw_to_voltage(adc, vbatAdcUnitCharacterics);
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc, &vbat));
    DBGLN("Cali Voltage: %d mV", vbat);
#endif

    // For negative offsets, anything between abs(OFFSET) and 0 is considered 0
    if (ANALOG_VBAT_OFFSET < 0 && adc <= -ANALOG_VBAT_OFFSET)
        vbat = 0;
    else
        vbat = ((int32_t)adc - ANALOG_VBAT_OFFSET) * 100 / ANALOG_VBAT_SCALE;
}

static int timeout()
{
    if (GPIO_ANALOG_VBAT == UNDEF_PIN)
    {
        return DURATION_NEVER;
    }

    uint32_t adc = analogRead(GPIO_ANALOG_VBAT);
#if defined(PLATFORM_ESP32) && defined(DEBUG_VBAT_ADC)
    // When doing DEBUG_VBAT_ADC, every value is adjusted (for logging)
    // in normal mode only the final value is adjusted to save CPU cycles
    if (vbatAdcUnitCharacterics)
        adc = esp_adc_cal_raw_to_voltage(adc, vbatAdcUnitCharacterics);
    DBGLN("$ADC,%u", adc);
#endif

    unsigned int idx = vbatSmooth.add(adc);
    if (idx == 0 && connectionState == connected)
        reportVbat();

    return VBAT_SAMPLE_INTERVAL * vbatUpdateScale;
}

device_t AnalogVbat_device = {
    .initialize = nullptr,
    .start = start,
    .event = nullptr,
    .timeout = timeout,
    .id = deviceId::nodev,
};

#endif /* if USE_ANALOG_VCC */