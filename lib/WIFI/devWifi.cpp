#include "device.h"

#if defined(PLATFORM_ESP8266) || defined(PLATFORM_ESP32)
#include <WiFi.h>
#include <DNSServer.h>


#include "hwTimer.h"
#include "logging.h"
#include "options.h"
#include "helpers.h"
#include "config.h"

static char station_ssid[33];
static char station_password[65];

static bool wifiStarted = false;
bool webserverPreventAutoStart = false;

static const byte DNS_PORT = 53;
static IPAddress netMsk(255, 255, 255, 0);
static DNSServer dnsServer;
static IPAddress ipAddress;

static wl_status_t laststatus = WL_IDLE_STATUS;
volatile WiFiMode_t wifiMode = WIFI_OFF;
static volatile WiFiMode_t changeMode = WIFI_OFF;
static volatile unsigned long changeTime = 0;


void setWifiUpdateMode()
{
  // No need to ExitBindingMode(), the radio will be stopped stopped when start the Wifi service.
  // Need to change this before the mode change event so the LED is updated
//   InBindingMode = false;
  connectionState = wifiUpdate;
}
static void startWiFi(unsigned long now)
{
  if (wifiStarted) {
    return;
  }

  if (connectionState < FAILURE_STATES) {
    hwTimer::stop();
    // // Set transmit power to minimum
    // POWERMGNT::setPower(MinPower);

    setWifiUpdateMode();

    DBGLN("Stopping Radio");
    // Radio.End();
  }

  DBGLN("Begin Webupdater");

  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  strcpy(station_ssid, firmwareOptions.home_wifi_ssid);
  strcpy(station_password, firmwareOptions.home_wifi_password);
  if (station_ssid[0] == 0) {
    changeTime = now;
    changeMode = WIFI_AP;
  }
  else {
    changeTime = now;
    changeMode = WIFI_STA;
  }
  laststatus = WL_DISCONNECTED;
  wifiStarted = true;
}

// Hàm khởi tạo Wi-Fi
static void initialize()
{
  wifiStarted = false;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  #if defined(PLATFORM_ESP8266)
  WiFi.forceSleepBegin();
  #endif
  // Nếu cần, có thể thêm chức năng button để kích hoạt Wi-Fi
  // registerButtonFunction(ACTION_START_WIFI, [](){
  //   setWifiUpdateMode();
  // });
}

// Hàm bắt đầu Wi-Fi
static int start()
{
  ipAddress.fromString(wifi_ap_address);
  return firmwareOptions.wifi_auto_on_interval;
}

static int event()
{
  // Kiểm tra trạng thái kết nối và điều kiện để bật Wi-Fi
  if (connectionState == wifiUpdate || connectionState > FAILURE_STATES)
  {
    if (!wifiStarted) {
      startWiFi(millis());  // Giả định startWiFi khởi động kết nối Wi-Fi
      return DURATION_IMMEDIATELY;
    }
  }
  else if (wifiStarted)
  {
    wifiStarted = false;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    #if defined(PLATFORM_ESP8266)
    WiFi.forceSleepBegin();
    #endif
  }
  return DURATION_IGNORE;
}

// Đặt thời gian chờ cho Wi-Fi
static int timeout()
{
  return 2;
}

// Khai báo device_t cho Wi-Fi
device_t WIFI_device = {
  .initialize = initialize,
  .start = start,
  .event = event,
  .timeout = timeout
};

#endif // Kết thúc kiểm tra PLATFORM_ESP8266 || PLATFORM_ESP32
