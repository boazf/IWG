#include <Arduino.h>
#include <Common.h>
#include <EthernetUtil.h>
#include <Config.h>
#include <TimeUtil.h>
#ifdef ESP32
#include <ping.h>
#endif

bool IsZeroIPAddress(const IPAddress &ip)
{
  return ip == IPAddress(0, 0, 0, 0);
}

void InitEthernet()
{
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // start the Ethernet connection:
  if (!IsZeroIPAddress(Config::ip) && !IsZeroIPAddress(Config::gateway) && !IsZeroIPAddress(Config::mask))
  {
#ifdef ESP32
    WiFi.mode(WIFI_MODE_STA);
    WiFi.config(Config::ip, Config::gateway, Config::mask, Config::gateway);
    WiFi.setAutoReconnect(true);
#else
    Ethernet.begin(Config::mac, Config::ip, Config::gateway, Config::gateway, Config::mask);
#endif
  }
#ifdef ESP32
#ifdef DEBUG_ETHERNET
  Serial.print("Connecting to: ");
  Serial.print(Config::ssid);
  Serial.print(' ');
#endif
  WiFi.begin(Config::ssid, Config::password);
  while(WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(500);
#ifdef DEBUG_ETHERNET
    Serial.print('.');
#endif      
  }
#ifdef DEBUG_ETHERNET
    Serial.println(" Connected!");
#endif      
#else
  else
    Ethernet.begin(Config::mac);  
#endif

#ifdef DEBUG_ETHERNET
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
#endif
}

void MaintainEthernet()
{
#ifndef ESP32
#ifdef DEBUG_ETHERNET
  int res = 
#endif
  Ethernet.maintain();
#ifdef DEBUG_ETHERNET
  switch (res) {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;
  }
#endif // DEBUG_ETHERNET
#else
  static bool connected = true;
  static time_t tReconnect;
#ifdef DEBUG_ETHERNET
  static time_t tLastUpdate;
#endif
  wl_status_t status = (wl_status_t)WiFi.status();
  if (status != WL_CONNECTED)
  {
    if (connected)
    {
#ifdef DEBUG_ETHERNET
      Serial.println("Network disconnected, trying to reconnect");
      tLastUpdate = 0;
#endif
      tReconnect = t_now;
      connected = false;
    }
#ifdef DEBUG_ETHERNET
    if (tLastUpdate != t_now)
    {
      tLastUpdate = t_now;
      Serial.printf("WiFi status: %d\n", status);
    }
#endif
    if (t_now - tReconnect > 60)
    {
#ifdef DEBUG_ETHERNET
      Serial.println("Reconnecting");
#endif
      WiFi.reconnect();
      tReconnect = t_now;
      delay(2000);
    }
  }
  else
  {
    if (connected)
      return;
#ifdef DEBUG_ETHERNET
    Serial.printf("WiFi status: %d\n", status);
#endif
    delay(2000);
    if (!ping_start(Config::gateway, 4, 0, 0, 1000))
    {
#ifdef DEBUG_ETHERNET
      Serial.println("Failed to ping gateway after network reconnect!\nReconnecting");
#endif
      WiFi.reconnect();
      tReconnect = t_now;
      delay(2000);
    }
    else
    {
#ifdef DEBUG_ETHERNET
      Serial.println("Connected!");
#endif
      connected = true;
    }
  }
#endif // ESP32
}
