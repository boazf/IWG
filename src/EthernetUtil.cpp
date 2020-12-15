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
  wl_status_t status = (wl_status_t)WiFi.status();
  if (status != WL_CONNECTED)
  {
    connected = false;
#ifdef DEBUG_ETHERNET
    Serial.println("Network disconnected, trying to reconnect ");
#endif
    time_t t0 = t_now;
    time_t tReconnect = t0;
    bool stop = false;
    while(!stop)
    {
      delay(500);
      status = (wl_status_t)WiFi.status();
#ifdef DEBUG_ETHERNET
      Serial.print(status);
#endif
      switch (status)
      {
        case WL_CONNECTED:
#ifdef DEBUG_ETHERNET
          Serial.println();
#endif
          stop = true;
          break;
        default:
          if (t_now - tReconnect > 60)
          {
#ifdef DEBUG_ETHERNET
            Serial.println(" Reconnecting");
#endif
            WiFi.reconnect();
            tReconnect = t_now;
          }
          break;
      }
      if (t_now - t0 >=120)
      {
#ifdef DEBUG_ETHERNET
      Serial.print(" Failed!");
#endif
        break;
      }
    }
    if (status == WL_CONNECTED)
    {
      delay(2000);
      if (!ping_start(Config::gateway, 4, 0, 0, 1000))
      {
#ifdef DEBUG_ETHERNET
        Serial.println("Failed to ping gateway after network reconnect!");
#endif
        status = WL_DISCONNECTED;
      }
    }
  }
  if (status == WL_CONNECTED)
  {
    if(!connected)
    {
#ifdef DEBUG_ETHERNET
      Serial.println("Connected!");
#endif
      connected = true;
    }
  }
  else
  {
#ifdef DEBUG_ETHERNET
    Serial.println("Reconnecting");
#endif
    WiFi.reconnect();
    delay(2000);
  }
#endif // ESP32
}
