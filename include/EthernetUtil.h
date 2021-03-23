#ifndef EthernetUtil_h
#define EthernetUtil_h

#ifdef USE_WIFI
#include <WiFi.h>
#else
#include <Ethernet.h>
#ifdef ESP32
#include <EthernetUdp.h>
#endif
#endif

#ifdef USE_WIFI
#define Eth WiFi
#define EthClient WiFiClient
#define EthServer WiFiServer
#else
#ifdef ESP32
class EthernetClassEx
{
public:
    EthernetClassEx(EthernetClass _ethernet) { ethernet = _ethernet; }
	static int begin(uint8_t *mac, unsigned long timeout = 60000, unsigned long responseTimeout = 4000);
	static int maintain();
	static EthernetLinkStatus linkStatus();
	static EthernetHardwareStatus hardwareStatus();

	// Manaul configuration
	static void begin(uint8_t *mac, IPAddress ip);
	static void begin(uint8_t *mac, IPAddress ip, IPAddress dns);
	static void begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway);
	static void begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);
	static void init(uint8_t sspin = 10);

	static void MACAddress(uint8_t *mac_address);
	static IPAddress localIP();
	static IPAddress subnetMask();
	static IPAddress gatewayIP();
	static IPAddress dnsServerIP();

	void setMACAddress(const uint8_t *mac_address);
	void setLocalIP(const IPAddress local_ip);
	void setSubnetMask(const IPAddress subnet);
	void setGatewayIP(const IPAddress gateway);
	void setDnsServerIP(const IPAddress dns_server);
	void setRetransmissionTimeout(uint16_t milliseconds);
	void setRetransmissionCount(uint8_t num);

private:
    EthernetClass ethernet;
};

extern EthernetClassEx EthernetEx;

class EthernetClientEx
{
public:
    EthernetClientEx() {}
    EthernetClientEx(const EthernetClient &_client) { client = _client; }
    EthernetClientEx operator =(const EthernetClient &_client) { client = _client; return *this; }
    EthernetClientEx operator =(const EthernetClientEx &other) { client = other.client; return *this; }
    uint16_t remotePort();
	size_t write(uint8_t);
	size_t write(const uint8_t *buf, size_t size);
	size_t write(const char *buf, size_t size) { return write((uint8_t*)buf, size); }
	int available();
	int read();
	int read(uint8_t *buf, size_t size);
	operator bool();
    uint8_t connected();
    IPAddress remoteIP();
    void stop();
    uint8_t getSocketNumber() { return client.getSocketNumber(); }
    size_t print(const String &str);
    size_t println(const String &str);
    size_t print(int n);
    size_t println(int n);
    size_t println();
	virtual void flush();

private:
    EthernetClient client;
};

class EthernetServerEx
{
public:
    EthernetServerEx(uint16_t port) { server = new EthernetServer(port); }
	EthernetClientEx available();
	EthernetClientEx accept();
	void begin();

private:
    EthernetServer *server;
};

class EthernetUDPEx
{
public:
	EthernetUDPEx() {}
	uint8_t begin(uint16_t);
	void stop();
	int beginPacket(IPAddress ip, uint16_t port);
	int beginPacket(const char *host, uint16_t port);
	int endPacket();
	size_t write(const uint8_t *buffer, size_t size);
	int available();
	int read(unsigned char* buffer, size_t len);
	int parsePacket();

private:
    EthernetUDP udp;
};

#define Eth EthernetEx
#define EthClient EthernetClientEx
#define EthServer EthernetServerEx
#define EthUDP EthernetUDPEx
#else
#define Eth Etherne
#define EthClient EthernetClient
#define EthServer EthernetServer
#define EthUDP EthernetUDP
#endif // ESP32
#endif // USE_WIFI

void InitEthernet();
void MaintainEthernet();
bool IsZeroIPAddress(const IPAddress &address);

#endif // EthernetUtil_h