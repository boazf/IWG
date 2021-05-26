#include <Arduino.h>
#include <Common.h>
#include <EthernetUtil.h>
#include <Config.h>
#include <TimeUtil.h>
#ifdef USE_WIFI
#include <ping.h>
#else
#include <Dns.h>
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

// Manaul configuration
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
  return client.write(byte);
}

size_t EthernetClientEx::write(const uint8_t *buf, size_t size)
{
  Lock lock(csSpi);
  return client.write(buf, size);
}

EthernetClientEx::operator bool()
{
  Lock lock(csSpi);
  return !!client;
}

void EthernetClientEx::stop()
{
  Lock lock(csSpi);
  client.stop();
}

uint8_t EthernetClientEx::connected()
{
  Lock lock(csSpi);
  return client.connected();
}
IPAddress EthernetClientEx::remoteIP()
{
  Lock lock(csSpi);
  return client.remoteIP();
}

int EthernetClientEx::available()
{
  Lock lock(csSpi);
  return client.available();
}

int EthernetClientEx::read()
{
  Lock lock(csSpi);
  return client.read();
}

int EthernetClientEx::read(uint8_t *buf, size_t size)
{
  Lock lock(csSpi);
  return client.read(buf, size);
}

size_t EthernetClientEx::print(const String &str)
{
  Lock lock(csSpi);
  return client.print(str);
}

size_t EthernetClientEx::println(const String &str)
{
  Lock lock(csSpi);
  return client.println(str);
}

size_t EthernetClientEx::print(int n)
{
  return print(String(n));
}

size_t EthernetClientEx::println(int n)
{
  return println(String(n));
}

size_t EthernetClientEx::println()
{
  Lock lock(csSpi);
  return client.println();
}

void EthernetClientEx::flush()
{
  Lock lock(csSpi);
  client.flush();
}

uint16_t EthernetClientEx::remotePort()
{
  Lock lock(csSpi);
  return client.remotePort();
}

EthernetClientEx EthernetServerEx::available()
{
  Lock lock(csSpi);
  return server->available();
}

EthernetClientEx EthernetServerEx::accept()
{
  Lock lock(csSpi);
  return server->accept();
}

void EthernetServerEx:: begin()
{
  Lock lock(csSpi);
  server->begin();
}

uint8_t EthernetUDPEx::begin(uint16_t port)
{
  Lock lock(csSpi);
  return udp.begin(port);
}

void EthernetUDPEx::stop()
{
  Lock lock(csSpi);
  udp.stop();
}

int EthernetUDPEx::beginPacket(IPAddress ip, uint16_t port)
{
  Lock lock(csSpi);
  return udp.beginPacket(ip, port);
}

int EthernetUDPEx::beginPacket(const char *host, uint16_t port)
{
  Lock lock(csSpi);
  return udp.beginPacket(host, port);
}

int EthernetUDPEx::endPacket()
{
  Lock lock(csSpi);
  return udp.endPacket();
}

size_t EthernetUDPEx::write(const uint8_t *buffer, size_t size)
{
  Lock lock(csSpi);
  return udp.write(buffer, size);
}

int EthernetUDPEx::available()
{
  Lock lock(csSpi);
  return udp.available();
}

int EthernetUDPEx::read(unsigned char* buffer, size_t len)
{
  Lock lock(csSpi);
  return udp.read(buffer, len);
}

int EthernetUDPEx::parsePacket()
{
  Lock lock(csSpi);
  return udp.parsePacket();
}

#define RESET_P	17				// Tie the W5500 reset pin to ESP32 GPIO17 pin.
#define CS_P 16

static void WizReset() {
#ifdef DEBUG_ETHERNET
    LOCK_TRACE();
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
#endif

bool IsZeroIPAddress(const IPAddress &ip)
{
  return ip == IPAddress(0, 0, 0, 0);
}

void InitEthernet()
{
  // start the Ethernet connection:
#ifndef USE_WIFI
    Eth.init(CS_P);           // GPIO5 on the ESP32.
    WizReset();
#endif

  if (!IsZeroIPAddress(Config::ip) && !IsZeroIPAddress(Config::gateway) && !IsZeroIPAddress(Config::mask))
  {
#ifdef USE_WIFI
    WiFi.mode(WIFI_MODE_STA);
    WiFi.config(Config::ip, Config::gateway, Config::mask, Config::gateway);
    WiFi.setAutoReconnect(true);
#else
    Eth.begin(Config::mac, Config::ip, Config::gateway, Config::gateway, Config::mask);
#endif
  }
#ifdef USE_WIFI
  WiFi.begin(Config::ssid, Config::password);
  while(WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(500);
#ifdef DEBUG_ETHERNET
    Trace('.');
#endif      
  }
#ifdef DEBUG_ETHERNET
  Traceln(" Connected!");
#endif      
#else
  else
    Eth.begin(Config::mac);  
#endif

#ifdef DEBUG_ETHERNET
  {
    LOCK_TRACE();
    Trace("My IP address: ");
    Traceln(Eth.localIP());
  }
#endif
}

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
      {
        LOCK_TRACE();
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
      {
        LOCK_TRACE();
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
      Traceln("Network disconnected, trying to reconnect");
      tLastUpdate = 0;
#endif
      tReconnect = t_now;
      connected = false;
    }
#ifdef DEBUG_ETHERNET
    if (tLastUpdate != t_now)
    {
      tLastUpdate = t_now;
      Tracef("WiFi status: %d\n", status);
    }
#endif
    if (t_now - tReconnect > 60)
    {
#ifdef DEBUG_ETHERNET
      Traceln("Reconnecting");
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
    Tracef("WiFi status: %d\n", status);
#endif
    delay(2000);
    if (!ping_start(Config::gateway, 4, 0, 0, 1000))
    {
#ifdef DEBUG_ETHERNET
      Traceln("Failed to ping gateway after network reconnect!\nReconnecting");
#endif
      WiFi.reconnect();
      tReconnect = t_now;
      delay(2000);
    }
    else
    {
#ifdef DEBUG_ETHERNET
      Traceln("Connected!");
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
#ifdef USE_WIFI
	if (WiFi.hostByName(server.c_str(), address) != 1)
#else
	DNSClient dns;
  {
    Lock lock(csSpi);
    
	  dns.begin(Config::gateway);

	  if (dns.getHostByName(server.c_str(), address) != 1)
#endif
  	{
#ifdef DEBUG_ETHERNET
      LOCK_TRACE();
      Trace("Failed to get host address for ");
      Traceln(server.c_str());
#endif
      return false;
    }
#ifndef USE_WIFI
  }
#endif

	return true;
}
