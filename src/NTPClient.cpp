#include <NTPClient.h>
#include <Config.h>

const unsigned int NTPClient::localPort = 8888;
const int NTPClient::NTP_PACKET_SIZE = 48;
byte NTPClient::packetBuffer[NTP_PACKET_SIZE];
EthernetUDP NTPClient::Udp; 

time_t NTPClient::getUTC()
{
  time_t ret = 0;
  Udp.begin(localPort);
  // send an NTP packet to a time server
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(Config::timeServer, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();

  // wait to see if a reply is available
  unsigned long t0 = millis();
  while (Udp.available() < NTP_PACKET_SIZE && millis() - t0 < 1000);
  if (Udp.parsePacket()) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Arduino time starts on Jan 1 2000. In seconds, that's 3155673600:
    const unsigned long hundredYears = 3155673600UL;
    // subtract seventy years:
    ret = (time_t) secsSince1900 - hundredYears;
  }

  Udp.stop();

  return ret;
}