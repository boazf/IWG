/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

#include <Arduino.h>
#include <Common.h>
#include <EthernetUtil.h>
#include <Config.h>
#include <TimeUtil.h>
#include <AppConfig.h>
#ifdef USE_WIFI
#include <GWConnTest.h>
#include <ESPmDNS.h>
#ifdef DEBUG_ETHERNET
#include <map>
#endif
#else
#include <Dns.h>
#endif
#ifdef DEBUG_ETHERNET
#include <Trace.h>
#endif

#ifndef USE_WIFI

int EthernetClassEx::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
  Lock lock(csSpi);
  return EthernetClass::begin(mac, timeout, responseTimeout);
}

int EthernetClassEx::maintain()
{
  Lock lock(csSpi);
  return EthernetClass::maintain();
}

EthernetLinkStatus EthernetClassEx::linkStatus()
{
  Lock lock(csSpi);
  return EthernetClass::linkStatus();
}

EthernetHardwareStatus EthernetClassEx::hardwareStatus()
{
  Lock lock(csSpi);
  return EthernetClass::hardwareStatus();
}

// Manual configuration
void EthernetClassEx::begin(uint8_t *mac, IPAddress ip)
{
  Lock lock(csSpi);
  EthernetClass::begin(mac, ip);
}

void EthernetClassEx::begin(uint8_t *mac, IPAddress ip, IPAddress dns)
{
  Lock lock(csSpi);
  EthernetClass::begin(mac, ip, dns);
}

void EthernetClassEx::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway)
{
  Lock lock(csSpi);
  EthernetClass::begin(mac, ip, dns, gateway);
}

void EthernetClassEx::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
  Lock lock(csSpi);
  EthernetClass::begin(mac, ip, dns, gateway, subnet);
}

void EthernetClassEx::init(uint8_t sspin)
{
  Lock lock(csSpi);
  EthernetClass::init(sspin);
}

void EthernetClassEx::MACAddress(uint8_t *mac_address)
{
  Lock lock(csSpi);
  EthernetClass::MACAddress(mac_address);
}

IPAddress EthernetClassEx::localIP()
{
  Lock lock(csSpi);
  return EthernetClass::localIP();
}

IPAddress EthernetClassEx::subnetMask()
{
  Lock lock(csSpi);
  return EthernetClass::subnetMask();
}

IPAddress EthernetClassEx::gatewayIP()
{
  Lock lock(csSpi);
  return EthernetClass::gatewayIP();
}

IPAddress EthernetClassEx::dnsServerIP()
{
  Lock lock(csSpi);
  return EthernetClass::dnsServerIP();
}

void EthernetClassEx::setMACAddress(const uint8_t *mac_address)
{
  Lock lock(csSpi);
  ethernet.setMACAddress(mac_address);
}

void EthernetClassEx::setLocalIP(const IPAddress local_ip)
{
  Lock lock(csSpi);
  ethernet.setLocalIP(local_ip);
}

void EthernetClassEx::setSubnetMask(const IPAddress subnet)
{
  Lock lock(csSpi);
  ethernet.setSubnetMask(subnet);
}

void EthernetClassEx::setGatewayIP(const IPAddress gateway)
{
  Lock lock(csSpi);
  ethernet.setGatewayIP(gateway);
}

void EthernetClassEx::setDnsServerIP(const IPAddress dns_server)
{
  Lock lock(csSpi);
  ethernet.setDnsServerIP(dns_server);
}

void EthernetClassEx::setRetransmissionTimeout(uint16_t milliseconds)
{
  Lock lock(csSpi);
  ethernet.setRetransmissionTimeout(milliseconds);
}

void EthernetClassEx::setRetransmissionCount(uint8_t num)
{
  Lock lock(csSpi);
  ethernet.setRetransmissionCount(num);
}

EthernetClassEx EthernetEx(Ethernet);

size_t EthernetClientEx::write(uint8_t byte)
{
  Lock lock(csSpi);
  return EthernetClient::write(byte);
}

size_t EthernetClientEx::write(const uint8_t *buf, size_t size)
{
  Lock lock(csSpi);
  return EthernetClient::write(buf, size);
}

void EthernetClientEx::stop()
{
  Lock lock(csSpi);
  EthernetClient::stop();
}

uint8_t EthernetClientEx::connected()
{
  Lock lock(csSpi);
  return EthernetClient::connected();
}

IPAddress EthernetClientEx::remoteIP()
{
  Lock lock(csSpi);
  return EthernetClient::remoteIP();
}

int EthernetClientEx::available()
{
  Lock lock(csSpi);
  return EthernetClient::available();
}

int EthernetClientEx::read()
{
  Lock lock(csSpi);
  return EthernetClient::read();
}

int EthernetClientEx::read(uint8_t *buf, size_t size)
{
  Lock lock(csSpi);
  return EthernetClient::read(buf, size);
}

void EthernetClientEx::flush()
{
  Lock lock(csSpi);
  EthernetClient::flush();
}

uint16_t EthernetClientEx::remotePort()
{
  Lock lock(csSpi);
  return EthernetClient::remotePort();
}

int EthernetClientEx::connect(IPAddress ip, uint16_t port)
{
  Lock lock(csSpi);
  return EthernetClient::connect(ip, port);
}

int EthernetClientEx::connect(const char *host, uint16_t port)
{
  Lock lock(csSpi);
  return EthernetClient::connect(host, port);
}

int EthernetClientEx::peek()
{
  Lock lock(csSpi);
  return EthernetClient::peek();
}

int EthernetClientEx::availableForWrite(void)
{
  Lock lock(csSpi);
  return EthernetClient::availableForWrite();
}

uint16_t EthernetClientEx::localPort()
{
  Lock lock(csSpi);
  return EthernetClient::localPort();
}

EthernetClientEx EthernetServerEx::available()
{
  Lock lock(csSpi);
  return EthernetServer::available();
}

EthernetClientEx EthernetServerEx::accept()
{
  Lock lock(csSpi);
  return EthernetServer::accept();
}

void EthernetServerEx::begin()
{
  Lock lock(csSpi);
  EthernetServer::begin();
}

size_t EthernetServerEx::write(uint8_t byte)
{
  Lock lock(csSpi);
  return EthernetServer::write(byte);
}

size_t EthernetServerEx::write(const uint8_t *buf, size_t size)
{
  Lock lock(csSpi);
  return EthernetServer::write(buf, size);
}

EthernetServerEx::operator bool()
{
  Lock lock(csSpi);
  return EthernetServer::operator bool();
}

uint8_t EthernetUDPEx::begin(uint16_t port)
{
  Lock lock(csSpi);
  return EthernetUDP::begin(port);
}

void EthernetUDPEx::stop()
{
  Lock lock(csSpi);
  EthernetUDP::stop();
}

int EthernetUDPEx::beginPacket(IPAddress ip, uint16_t port)
{
  Lock lock(csSpi);
  return EthernetUDP::beginPacket(ip, port);
}

int EthernetUDPEx::beginPacket(const char *host, uint16_t port)
{
  Lock lock(csSpi);
  return EthernetUDP::beginPacket(host, port);
}

int EthernetUDPEx::endPacket()
{
  Lock lock(csSpi);
  return EthernetUDP::endPacket();
}

size_t EthernetUDPEx::write(const uint8_t *buffer, size_t size)
{
  Lock lock(csSpi);
  return EthernetUDP::write(buffer, size);
}

int EthernetUDPEx::available()
{
  Lock lock(csSpi);
  return EthernetUDP::available();
}

int EthernetUDPEx::read(unsigned char* buffer, size_t len)
{
  Lock lock(csSpi);
  return EthernetUDP::read(buffer, len);
}

int EthernetUDPEx::parsePacket()
{
  Lock lock(csSpi);
  return EthernetUDP::parsePacket();
}
size_t EthernetUDPEx::write(uint8_t byte)
{
  Lock lock(csSpi);
  return EthernetUDP::write(byte);
}

void EthernetUDPEx::flush()
{
  Lock lock(csSpi);
  EthernetUDP::flush();
}


int EthernetUDPEx::read()
{
  Lock lock(csSpi);
  return EthernetUDP::read();
}


int EthernetUDPEx::peek()
{
  Lock lock(csSpi);
  return EthernetUDP::peek();
}

uint8_t EthernetUDPEx::beginMulticast(IPAddress addr, uint16_t word)
{
  Lock lock(csSpi);
  return EthernetUDP::beginMulticast(addr, word);
}

#define RESET_P	17				// Tie the W5500 reset pin to ESP32 GPIO17 pin.
#define CS_P 16

static void WizReset() {
#ifdef DEBUG_ETHERNET
    LOCK_TRACE;
    Trace("Resetting Wiz W5500 Ethernet Board...  ");
#endif
    pinMode(RESET_P, OUTPUT);
    digitalWrite(RESET_P, HIGH);
    delay(250);
    digitalWrite(RESET_P, LOW);
    delay(50);
    digitalWrite(RESET_P, HIGH);
    delay(350);
#ifdef DEBUG_ETHERNET
    Traceln("Done.");
#endif
}
#endif // !USE_WIFI

bool IsZeroIPAddress(const IPAddress &ip)
{
  return ip == IPAddress(0, 0, 0, 0);
}

bool InitEthernet()
{
  // start the Ethernet connection:
#ifndef USE_WIFI
  Eth.init(CS_P);
  WizReset();
#else
  WiFi.mode(WIFI_MODE_STA);
#endif

  if (!IsZeroIPAddress(Config::ip) && !IsZeroIPAddress(Config::gateway) && !IsZeroIPAddress(Config::mask))
  {
#ifdef USE_WIFI
    WiFi.config(Config::ip, Config::gateway, Config::mask, Config::gateway);
#else
    Eth.begin(Config::mac, Config::ip, Config::gateway, Config::gateway, Config::mask);
#endif
  }
#ifndef USE_WIFI
  else
  {
    Eth.begin(Config::mac);
  }
#else // USE_WIFI
  WiFi.setAutoReconnect(true);
  uint8_t *mac = IsZeroIPAddress(Config::mac) ? NULL : Config::mac;
  WiFi.setHostname(Config::hostName);
  WiFi.begin(Config::ssid, Config::password, 0, mac);
  unsigned long t0 = millis();
  while(true)
  {
    if (WiFi.waitForConnectResult() != WL_CONNECTED && millis() - t0 < 60000)
    {
      delay(500);
#ifdef DEBUG_ETHERNET
      Trace('.');
#endif      
    }
    else
    {
      delay(2000);
#ifdef DEBUG_ETHERNET
      Tracef("\nPinging gateway: %s\n", Eth.gatewayIP().toString().c_str());
#endif
      if (!GWConnTest::ping(20, 100))
      {
#ifdef DEBUG_ETHERNET
        Traceln("Failed to ping gateway!\nReconnecting");
#endif
      }
      else
        break;
      WiFi.reconnect();
      delay(2000);
      t0 = millis();
    }
  }

#ifdef DEBUG_ETHERNET
  Traceln("Connected!");
  Tracef("RSSI: %ddb, BSSID:%s\n", WiFi.RSSI(), WiFi.BSSIDstr().c_str());
#endif      
#endif // USE_WIFI

#ifdef DEBUG_ETHERNET
  TRACE_BLOCK
	{
    Trace("My IP address: ");
    Traceln(Eth.localIP());
  }
#endif

#ifdef USE_WIFI
#ifdef DEBUG_ETHERNET
  bool mdnsSuccess = 
#endif
  MDNS.begin(Config::hostName);
#ifdef DEBUG_ETHERNET
  if (mdnsSuccess)
    Traceln("mDNS started");
  else
    Traceln("mDNS failed to start");
#endif
#endif // USE_WIFI
  return true;
}

bool WaitForDNS()
{
  // Wait for successful DNS queries.
  #define EXPECT_SEQ_SUCCESS 5
#ifdef DEBUG_ETHERNET
  Traceln("Waiting for DNS availability...");
#endif
  bool success = false;
  IPAddress addrSrv;
  unsigned long t0 = millis();
  unsigned long tWait = Config::dnsAvailTimeSec * 1000;
  do
  {
    while(!TryGetHostAddress(addrSrv, AppConfig::getServer1()) &&
          !TryGetHostAddress(addrSrv, AppConfig::getServer2()) &&
          millis() - t0 < tWait)
      delay(1000);
    if (millis() - t0 > tWait)
      break;
    // Expect several sequential successful queries
    int i = 0;
    do
    {
      if (!TryGetHostAddress(addrSrv, AppConfig::getServer1()) &&
          !TryGetHostAddress(addrSrv, AppConfig::getServer2()))
        break;
      delay(500);
      success = ++i == EXPECT_SEQ_SUCCESS;
    } while (!success);
  } while (!success);
#ifdef DEBUG_ETHERNET
  if (!success)
    Traceln("No DNS!");
  else
    Traceln("DNS is available.");
#endif
  return success;
}

#ifdef USE_WIFI
#ifdef DEBUG_ETHERNET
static std::map<wl_status_t, std::string> statusNames = 
{
  { WL_NO_SHIELD, "WL_NO_SHIELD" },
  { WL_IDLE_STATUS, "WL_IDLE_STATUS" },
  { WL_NO_SSID_AVAIL, "WL_NO_SSID_AVAIL" },
  { WL_SCAN_COMPLETED, "WL_SCAN_COMPLETED" },
  { WL_CONNECTED, "WL_CONNECTED" },
  { WL_CONNECT_FAILED, "WL_CONNECT_FAILED" },
  { WL_CONNECTION_LOST, "WL_CONNECTION_LOST" },
  { WL_DISCONNECTED, "WL_DISCONNECTED" }
};
#endif
#endif

void MaintainEthernet()
{
#ifndef USE_WIFI
#ifdef DEBUG_ETHERNET
  int res = 
#endif
  Eth.maintain();
#ifdef DEBUG_ETHERNET
  switch (res) {
    case 1:
      //renewed fail
#ifdef DEBUG_ETHERNET
      Traceln("Error: renewed fail");
#endif
      break;

    case 2:
#ifdef DEBUG_ETHERNET
      TRACE_BLOCK
      {    
        //renewed success
        Traceln("Renewed success");
        //print your local IP address:
        Trace("My IP address: ");
        Traceln(Eth.localIP());
      }
#endif
      break;

    case 3:
      //rebind fail
#ifdef DEBUG_ETHERNET
      Traceln("Error: rebind fail");
#endif
      break;

    case 4:
#ifdef DEBUG_ETHERNET
      TRACE_BLOCK
      {
        //rebind success
        Traceln("Rebind success");
        //print your local IP address:
        Trace("My IP address: ");
        Traceln(Eth.localIP());
      }
#endif
      break;

    default:
      //nothing happened
      break;
  }
#endif // DEBUG_ETHERNET
#else // USE_WIFI
#define WIFI_RECONNECT() \
  WiFi.reconnect(); \
  tReconnect = t_now; \
  delay(2000);

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
      Traceln("Network disconnected, trying to reconnect");
      tLastUpdate = 0;
#endif
      WIFI_RECONNECT();
      connected = false;
    }
#ifdef DEBUG_ETHERNET
    if (tLastUpdate != t_now)
    {
      tLastUpdate = t_now;
      Tracef("WiFi status: %s\n", statusNames[status].c_str());
    }
#endif
    if (t_now - tReconnect > 60)
    {
#ifdef DEBUG_ETHERNET
      Traceln("Reconnecting");
#endif
      WIFI_RECONNECT();
    }
  }
  else
  {
    if (connected)
      return;
#ifdef DEBUG_ETHERNET
    Tracef("WiFi status: %s\n", statusNames[status].c_str());
#endif
    delay(2000);
    if (!GWConnTest::ping(5, 1000))
    {
#ifdef DEBUG_ETHERNET
      Traceln("Failed to ping gateway after network reconnect!\nReconnecting");
#endif
      WIFI_RECONNECT();
    }
    else
    {
#ifdef DEBUG_ETHERNET
      Traceln("Connected!");
      Tracef("RSSI: %ddb, BSSID:%s\n", WiFi.RSSI(), WiFi.BSSIDstr().c_str());
#endif
      connected = true;
    }
  }
#endif // USE_WIFI
}

bool TryGetHostAddress(IPAddress &address, String server)
{
	if (server.equals(""))
		return false;
  int error;
#ifdef USE_WIFI
	error = WiFi.hostByName(server.c_str(), address);
#else
	DNSClient dns;
  {
    Lock lock(csSpi);
    
    dns.begin(Eth.gatewayIP());

    error = dns.getHostByName(server.c_str(), address);
  }
#endif
  if (error != 1)
  {
#ifdef DEBUG_ETHERNET
    Tracef("Failed to get host address for %s, error: %d\n", server.c_str(), error);
#endif
    return false;
  }

	return true;
}
