#include <Arduino.h>
#include <Ethernet.h>
#include <Common.h>
#include <EthernetUtil.h>
#include <Config.h>

bool IsZeroIPAddress(byte *ip)
{
  return ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0;
}

void InitEthernet()
{
  //IPAddress ip = { 192,168,0,75 };

  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // start the Ethernet connection:
  if (!IsZeroIPAddress(Config::ip) && !IsZeroIPAddress(Config::gateway) && !IsZeroIPAddress(Config::mask))
    Ethernet.begin(Config::mac, Config::ip, Config::gateway, Config::gateway, Config::mask);
  else
    Ethernet.begin(Config::mac);  

#ifdef DEBUG_ETHERNET
  Serial.print("My IP address set setup: ");
  Serial.println(Ethernet.localIP());
#endif
}

void MaintainEthernet()
{
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
#endif
}


