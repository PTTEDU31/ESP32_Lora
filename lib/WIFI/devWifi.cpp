#include "device.h"

#if defined(PLATFORM_ESP8266) || defined(PLATFORM_ESP32)

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
#include <set>
#include <StreamString.h>

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

static AsyncWebServer server(80);
static bool servicesStarted = false;
static constexpr uint32_t STALE_WIFI_SCAN = 20000;
static uint32_t lastScanTimeMS = 0;

static bool force_update = false;

static struct
{
  const char *url;
  const char *contentType;
  const uint8_t *content;
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
  connectionState = wifiUpdate;
}
static void startWiFi(unsigned long now)
{
  if (wifiStarted)
  {
    return;
  }

  if (connectionState < FAILURE_STATES)
  {
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
static void WebUpdateSendNetworks(AsyncWebServerRequest *request)
{
  int numNetworks = WiFi.scanComplete();
  if (numNetworks >= 0 && millis() - lastScanTimeMS < STALE_WIFI_SCAN)
  {
    DBGLN("Found %d networks", numNetworks);
    std::set<String> vs;
    String s = "[";
    for (int i = 0; i < numNetworks; i++)
    {
      String w = WiFi.SSID(i);
      DBGLN("found %s", w.c_str());
      if (vs.find(w) == vs.end() && w.length() > 0)
      {
        if (!vs.empty())
          s += ",";
        s += "\"" + w + "\"";
        vs.insert(w);
      }
    }
    s += "]";
    request->send(200, "application/json", s);
  }
  else
  {
    if (WiFi.scanComplete() != WIFI_SCAN_RUNNING)
    {
#if defined(PLATFORM_ESP8266)
      scanComplete = false;
      WiFi.scanNetworksAsync([](int)
                             { scanComplete = true; });
#else
      WiFi.scanNetworks(true);
#endif
      lastScanTimeMS = millis();
    }
    request->send(204, "application/json", "[]");
  }
}
static void sendResponse(AsyncWebServerRequest *request, const String &msg, WiFiMode_t mode)
{
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", msg);
  response->addHeader("Connection", "close");
  request->send(response);
  request->client()->close();
  changeTime = millis();
  changeMode = mode;
}
static void WebUpdateAccessPoint(AsyncWebServerRequest *request)
{
  DBGLN("Starting Access Point");
  String msg = String("Access Point starting, please connect to access point '") + wifi_ap_ssid + "' with password '" + wifi_ap_password + "'";
  sendResponse(request, msg, WIFI_AP);
}

static void WebUpdateConnect(AsyncWebServerRequest *request)
{
  DBGLN("Connecting to network");
  String msg = String("Connecting to network '") + station_ssid + "', connect to http://" +
               wifi_hostname + ".local from a browser on that network";
  sendResponse(request, msg, WIFI_STA);
}
static void WebUpdateSetHome(AsyncWebServerRequest *request)
{
  String ssid = request->arg("network");
  String password = request->arg("password");

  DBGLN("Setting network %s", ssid.c_str());
  strcpy(station_ssid, ssid.c_str());
  strcpy(station_password, password.c_str());
  if (request->hasArg("save")) {
    strlcpy(firmwareOptions.home_wifi_ssid, ssid.c_str(), sizeof(firmwareOptions.home_wifi_ssid));
    strlcpy(firmwareOptions.home_wifi_password, password.c_str(), sizeof(firmwareOptions.home_wifi_password));
    saveOptions();
  }
  WebUpdateConnect(request);
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
  server.on("/networks.json", WebUpdateSendNetworks);
  server.on("/sethome", WebUpdateSetHome);
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
    return 2;
  }
  return DURATION_NEVER;
}

// Khai báo device_t cho Wi-Fi
device_t WIFI_device = {
    .initialize = initialize,
    .start = start,
    .event = event,
    .timeout = timeout,
    .id=deviceId::WiFi,
    };

#endif // Kết thúc kiểm tra PLATFORM_ESP8266 || PLATFORM_ESP32
