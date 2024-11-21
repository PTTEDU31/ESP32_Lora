#include "device.h"

#if defined(PLATFORM_ESP8266) || defined(PLATFORM_ESP32)

<<<<<<< Updated upstream
#include <DNSServer.h>
#ifdef ESP32
#include <AsyncTCP.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#elif defined(TARGET_RP2040)
#include <WebServer.h>
#include <WiFi.h>
#endif
#include "ESPAsyncWebServer.h"

=======
>>>>>>> Stashed changes
#include "hwTimer.h"
#include "logging.h"
#include "options.h"
#include "helpers.h"
#include "config.h"
#if defined(PLATFORM_ESP8266)
#include <FS.h>
#else
#include <SPIFFS.h>
#endif

#include "WebContent.h"

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
<<<<<<< Updated upstream

static AsyncWebServer server(80);
static bool servicesStarted = false;
static constexpr uint32_t STALE_WIFI_SCAN = 20000;
static uint32_t lastScanTimeMS = 0;

static bool force_update = false;

static struct {
  const char *url;
  const char *contentType;
  const uint8_t* content;
  const size_t size;
} files[] = {
  {"/scan.js", "text/javascript", (uint8_t *)SCAN_JS, sizeof(SCAN_JS)},
  {"/mui.js", "text/javascript", (uint8_t *)MUI_JS, sizeof(MUI_JS)},
  {"/elrs.css", "text/css", (uint8_t *)ELRS_CSS, sizeof(ELRS_CSS)},
  {"/hardware.html", "text/html", (uint8_t *)HARDWARE_HTML, sizeof(HARDWARE_HTML)},
  {"/hardware.js", "text/javascript", (uint8_t *)HARDWARE_JS, sizeof(HARDWARE_JS)},
  {"/cw.html", "text/html", (uint8_t *)CW_HTML, sizeof(CW_HTML)},
  {"/cw.js", "text/javascript", (uint8_t *)CW_JS, sizeof(CW_JS)},
#if defined(RADIO_LR1121)
  {"/lr1121.html", "text/html", (uint8_t *)LR1121_HTML, sizeof(LR1121_HTML)},
  {"/lr1121.js", "text/javascript", (uint8_t *)LR1121_JS, sizeof(LR1121_JS)},
#endif
};

void setWifiUpdateMode()
{
=======
static bool servicesStarted = false;

void setWifiUpdateMode()
{
  // No need to ExitBindingMode(), the radio will be stopped stopped when start the Wifi service.
  // Need to change this before the mode change event so the LED is updated
  //   InBindingMode = false;
>>>>>>> Stashed changes
  connectionState = wifiUpdate;
}
void HandleMSP2WIFI()
{
#if defined(USE_MSP_WIFI) && defined(TARGET_RX)
  // check is there is any data to write out
  if (crsf2msp.FIFOout.peekSize() > 0)
  {
    const uint16_t len = crsf2msp.FIFOout.popSize();
    uint8_t data[len];
    crsf2msp.FIFOout.popBytes(data, len);
    wifi2tcp.write(data, len);
  }

  // check if there is any data to read in
  const uint16_t bytesReady = wifi2tcp.bytesReady();
  if (bytesReady > 0)
  {
    uint8_t data[bytesReady];
    wifi2tcp.read(data);
    msp2crsf.parse(data, bytesReady);
  }

  wifi2tcp.handle();
#endif
}
static void startServices()
{
  if (servicesStarted) {
    // #if defined(PLATFORM_ESP32)
    //   MDNS.end();
    //   startMDNS();
    // #endif
    return;
  }
  return;
}
static void HandleWebUpdate()
{
  unsigned long now = millis();
  wl_status_t status = WiFi.status();

  if (status != laststatus && wifiMode == WIFI_STA)
  {
    DBGLN("WiFi status %d", status);
    switch (status)
    {
    case WL_NO_SSID_AVAIL:
    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
      changeTime = now;
      changeMode = WIFI_AP;
      break;
    case WL_DISCONNECTED: // try reconnection
      changeTime = now;
      break;
    default:
      break;
    }
    laststatus = status;
  }
  if (status != WL_CONNECTED && wifiMode == WIFI_STA && (now - changeTime) > 30000)
  {
    changeTime = now;
    changeMode = WIFI_AP;
    DBGLN("Connection failed %d", status);
  }
  if (changeMode != wifiMode && changeMode != WIFI_OFF && (now - changeTime) > 500)
  {
    switch (changeMode)
    {
    case WIFI_AP:
      DBGLN("Changing to AP mode");
      WiFi.disconnect();
      wifiMode = WIFI_AP;
#if defined(PLATFORM_ESP32)
      WiFi.setHostname(wifi_hostname); // hostname must be set before the mode is set to STA
#endif
      WiFi.mode(wifiMode);
#if defined(PLATFORM_ESP8266)
      WiFi.setHostname(wifi_hostname); // hostname must be set before the mode is set to STA
#endif
      changeTime = now;
#if defined(PLATFORM_ESP8266)
      WiFi.setOutputPower(13.5);
      WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#elif defined(PLATFORM_ESP32)
      WiFi.setTxPower(WIFI_POWER_19_5dBm);
#endif
      WiFi.softAPConfig(ipAddress, ipAddress, netMsk);
      WiFi.softAP(wifi_ap_ssid, wifi_ap_password);
      startServices();
      break;
    case WIFI_STA:
      DBGLN("Connecting to network '%s'", station_ssid);
      wifiMode = WIFI_STA;
#if defined(PLATFORM_ESP32)
      WiFi.setHostname(wifi_hostname); // hostname must be set before the mode is set to STA
#endif
      WiFi.mode(wifiMode);
#if defined(PLATFORM_ESP8266)
      WiFi.setHostname(wifi_hostname); // hostname must be set after the mode is set to STA
#endif
      changeTime = now;
#if defined(PLATFORM_ESP8266)
      WiFi.setOutputPower(13.5);
      WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#elif defined(PLATFORM_ESP32)
      WiFi.setTxPower(WIFI_POWER_19_5dBm);
      WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
      WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
#endif
      WiFi.begin(station_ssid, station_password);
      startServices();
    default:
      break;
    }
#if defined(PLATFORM_ESP8266)
    MDNS.notifyAPChange();
#endif
    changeMode = WIFI_OFF;
  }

#if defined(PLATFORM_ESP8266)
  if (scanComplete)
  {
    WiFi.mode(wifiMode);
    scanComplete = false;
  }
#endif

  if (servicesStarted)
  {
    dnsServer.processNextRequest();
#if defined(PLATFORM_ESP8266)
    MDNS.update();
#endif

// #if defined(TARGET_TX) && defined(PLATFORM_ESP32)
//     WifiJoystick::Loop(now);
// #endif
  }
}
static void startWiFi(unsigned long now)
{
  if (wifiStarted)
  {
    return;
  }

  if (connectionState < FAILURE_STATES)
  {
<<<<<<< Updated upstream
=======
    hwTimer::stop();
    // // Set transmit power to minimum
    // POWERMGNT::setPower(MinPower);

>>>>>>> Stashed changes
    setWifiUpdateMode();
  }

  DBGLN("Begin Webupdater");

  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  strcpy(station_ssid, firmwareOptions.home_wifi_ssid);
  strcpy(station_password, firmwareOptions.home_wifi_password);
  if (station_ssid[0] == 0)
  {
    changeTime = now;
    changeMode = WIFI_AP;
  }
  else
  {
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
<<<<<<< Updated upstream
=======
  // Nếu cần, có thể thêm chức năng button để kích hoạt Wi-Fi
  // registerButtonFunction(ACTION_START_WIFI, [](){
  //   setWifiUpdateMode();
  // });
>>>>>>> Stashed changes
}

// Hàm bắt đầu Wi-Fi
static int start()
{
  ipAddress.fromString(wifi_ap_address);
  return firmwareOptions.wifi_auto_on_interval;
}

static int event()
{
  // // Kiểm tra trạng thái kết nối và điều kiện để bật Wi-Fi
  if (connectionState == wifiUpdate || connectionState > FAILURE_STATES)
  {
    if (!wifiStarted)
    {
      startWiFi(millis()); // Giả định startWiFi khởi động kết nối Wi-Fi
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
/** IP to String? */
static String toStringIp(IPAddress ip)
{
  String res = "";
  for (int i = 0; i < 3; i++)
  {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
static boolean isIp(String str)
{
  for (size_t i = 0; i < str.length(); i++)
  {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9'))
    {
      return false;
    }
  }
  return true;
}

static bool captivePortal(AsyncWebServerRequest *request)
{
  extern const char *wifi_hostname;

  if (!isIp(request->host()) && request->host() != (String(wifi_hostname) + ".local"))
  {
    DBGLN("Request redirected to captive portal");
    request->redirect(String("http://") + toStringIp(request->client()->localIP()));
    return true;
  }
  return false;
}
static void WebUpdateHandleNotFound(AsyncWebServerRequest *request)
{
  if (captivePortal(request))
  { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += request->url();
  message += F("\nMethod: ");
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += request->args();
  message += F("\n");

  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += String(F(" ")) + request->argName(i) + F(": ") + request->arg(i) + F("\n");
  }
  AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", message);
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  request->send(response);
}

static void WebUpdateHandleRoot(AsyncWebServerRequest *request)
{
  if (captivePortal(request))
  { // If captive portal redirect instead of displaying the page.
    return;
  }
  force_update = request->hasArg("force");
  AsyncWebServerResponse *response;
  response = request->beginResponse_P(200, "text/html", (uint8_t *)INDEX_HTML, sizeof(INDEX_HTML));
  // if (connectionState == hardwareUndefined)
  // {
  //   response = request->beginResponse_P(200, "text/html", (uint8_t *)HARDWARE_HTML, sizeof(HARDWARE_HTML));
  // }
  // else
  // {
  //   response = request->beginResponse_P(200, "text/html", (uint8_t *)INDEX_HTML, sizeof(INDEX_HTML));
  // }
  response->addHeader("Content-Encoding", "gzip");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  request->send(response);

  // Bắt đầu server
  server.begin();
}
static void WebUpdateSendContent(AsyncWebServerRequest *request)
{
  for (size_t i = 0; i < ARRAY_SIZE(files); i++)
  {
    if (request->url().equals(files[i].url))
    {
      AsyncWebServerResponse *response = request->beginResponse_P(200, files[i].contentType, files[i].content, files[i].size);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
      return;
    }
  }
  request->send(404, "text/plain", "File not found");
}
static void startServices()
{
  server.on("/", WebUpdateHandleRoot);
  server.on("/elrs.css", WebUpdateSendContent);
  server.on("/mui.js", WebUpdateSendContent);
  server.on("/scan.js", WebUpdateSendContent);
  server.onNotFound(WebUpdateHandleNotFound);
  server.begin();
  servicesStarted = true;

  DBGLN("HTTPUpdateServer ready! Open http://%s.local in your browser", wifi_hostname);
}
static void HandleWebUpdate()
{
  unsigned long now = millis();
  wl_status_t status = WiFi.status();

  if (status != laststatus && wifiMode == WIFI_STA)
  {
    DBGLN("WiFi status %d", status);
    switch (status)
    {
    case WL_NO_SSID_AVAIL:
    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
      changeTime = now;
      changeMode = WIFI_AP;
      break;
    case WL_DISCONNECTED: // try reconnection
      changeTime = now;
      break;
    default:
      break;
    }
    laststatus = status;
  }
  if (status != WL_CONNECTED && wifiMode == WIFI_STA && (now - changeTime) > 30000)
  {
    changeTime = now;
    changeMode = WIFI_AP;
    DBGLN("Connection failed %d", status);
  }
  if (changeMode != wifiMode && changeMode != WIFI_OFF && (now - changeTime) > 500)
  {
    switch (changeMode)
    {
    case WIFI_AP:
      DBGLN("Changing to AP mode");
      WiFi.disconnect();
      wifiMode = WIFI_AP;
#if defined(PLATFORM_ESP32)
      WiFi.setHostname(wifi_hostname); // hostname must be set before the mode is set to STA
#endif
      WiFi.mode(wifiMode);
#if defined(PLATFORM_ESP8266)
      WiFi.setHostname(wifi_hostname); // hostname must be set before the mode is set to STA
#endif
      changeTime = now;
#if defined(PLATFORM_ESP8266)
      WiFi.setOutputPower(13.5);
      WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#elif defined(PLATFORM_ESP32)
      WiFi.setTxPower(WIFI_POWER_19_5dBm);
#endif
      WiFi.softAPConfig(ipAddress, ipAddress, netMsk);
      WiFi.softAP(wifi_ap_ssid, wifi_ap_password);
      startServices();
      break;
    case WIFI_STA:
      DBGLN("Connecting to network '%s'", station_ssid);
      wifiMode = WIFI_STA;
#if defined(PLATFORM_ESP32)
      WiFi.setHostname(wifi_hostname); // hostname must be set before the mode is set to STA
#endif
      WiFi.mode(wifiMode);
#if defined(PLATFORM_ESP8266)
      WiFi.setHostname(wifi_hostname); // hostname must be set after the mode is set to STA
#endif
      changeTime = now;
#if defined(PLATFORM_ESP8266)
      WiFi.setOutputPower(13.5);
      WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#elif defined(PLATFORM_ESP32)
      WiFi.setTxPower(WIFI_POWER_19_5dBm);
      WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
      WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
#endif
      WiFi.begin(station_ssid, station_password);
      startServices();
    default:
      break;
    }
    changeMode = WIFI_OFF;
  }
}
// Đặt thời gian chờ cho Wi-Fi
static int timeout()
{
  if (wifiStarted)
  {
    HandleWebUpdate();
<<<<<<< Updated upstream
    return 2;
  }
=======
    HandleMSP2WIFI();
#if defined(PLATFORM_ESP8266)
    // When in STA mode, a small delay reduces power use from 90mA to 30mA when idle
    // In AP mode, it doesn't seem to make a measurable difference, but does not hurt
    // Only done on 8266 as the ESP32 runs a throttled task
    if (!Update.isRunning())
      delay(1);
    return DURATION_IMMEDIATELY;
#else
    // All the web traffic is async apart from changing modes and MSP2WIFI
    // No need to run balls-to-the-wall; the wifi runs on this core too (0)
    return 2;
#endif
  }
#if defined(TARGET_TX)
  // if webupdate was requested before or .wifi_auto_on_interval has elapsed but uart is not detected
  // start webupdate, there might be wrong configuration flashed.
  if (firmwareOptions.wifi_auto_on_interval != -1 && webserverPreventAutoStart == false && connectionState < wifiUpdate && !wifiStarted)
  {
    DBGLN("No CRSF ever detected, starting WiFi");
    setWifiUpdateMode();
    return DURATION_IMMEDIATELY;
  }
#endif
>>>>>>> Stashed changes
  return DURATION_NEVER;
}

// Khai báo device_t cho Wi-Fi
device_t WIFI_device = {
    .initialize = initialize,
    .start = start,
    .event = event,
    .timeout = timeout};

#endif // Kết thúc kiểm tra PLATFORM_ESP8266 || PLATFORM_ESP32
