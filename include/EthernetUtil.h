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

#ifndef EthernetUtil_h
#define EthernetUtil_h

#ifdef USE_WIFI
#include <WiFi.h>
#else
#include <Ethernet.h>
#include <EthernetUdp.h>
#endif

#ifdef USE_WIFI
#define Eth WiFi
#define EthClient WiFiClient
#define EthServer WiFiServer
#else
class EthernetClassEx
{
public:
    EthernetClassEx(EthernetClass _ethernet) { ethernet = _ethernet; }
	static int begin(uint8_t *mac, unsigned long timeout = 60000, unsigned long responseTimeout = 4000);
	static int maintain();
	static EthernetLinkStatus linkStatus();
	static EthernetHardwareStatus hardwareStatus();

	// Manual configuration
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

class EthernetClientEx : public EthernetClient
{
public:
	EthernetClientEx() : EthernetClient() {}
	EthernetClientEx(const EthernetClient &client) : EthernetClient(client) {}
    uint16_t remotePort() override;
	size_t write(uint8_t) override;
	size_t write(const uint8_t *buf, size_t size) override;
	size_t write(const char *buf, size_t size) { return write(reinterpret_cast<const uint8_t *>(buf), size); }
	int available() override;
	int read() override;
	int read(uint8_t *buf, size_t size) override;
    uint8_t connected() override;
    IPAddress remoteIP() override;
    void stop() override;
	void flush() override;
    int connect(IPAddress ip, uint16_t port) override;
    int connect(const char *host, uint16_t port) override;
    int peek() override;
	int availableForWrite(void) override;
	uint16_t localPort() override;
};

class EthernetServerEx : private EthernetServer
{
public:
    EthernetServerEx(uint16_t port) : EthernetServer(port) {}
	EthernetClientEx available();
	EthernetClientEx accept();
	void begin() override;
	virtual size_t write(uint8_t) override;
	virtual size_t write(const uint8_t *buf, size_t size) override;
	virtual operator bool() override;
};

class EthernetUDPEx : public EthernetUDP
{
public:
	EthernetUDPEx() {}
	uint8_t begin(uint16_t) override;
	uint8_t beginMulticast(IPAddress, uint16_t) override;
	void stop() override;
	int beginPacket(IPAddress ip, uint16_t port) override;
	int beginPacket(const char *host, uint16_t port) override;
	int endPacket() override;
	size_t write(uint8_t) override;
	size_t write(const uint8_t *buffer, size_t size) override;
	int parsePacket() override;
	int available() override;
	int read() override;
	int read(unsigned char* buffer, size_t len) override;
	int peek() override;
	void flush() override;
};

#define Eth EthernetEx
#define EthClient EthernetClientEx
#define EthServer EthernetServerEx
#define EthUDP EthernetUDPEx
#endif // USE_WIFI

class AutoStopClient : public EthClient
{
public:
	~AutoStopClient()
	{
		stop();
	}
};

bool WaitForDNS();

bool InitEthernet();
void MaintainEthernet();
bool IsZeroIPAddress(const IPAddress &address);
bool TryGetHostAddress(IPAddress &address, String server);

#endif // EthernetUtil_h