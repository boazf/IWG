Arduino Internet Watchdog Device
================================

How many times have you lost internet connectivity, then you had to go to your router and/or modem, disconnect it/them from power, wait a while,  reconnect and wait for internet connectivity to resume? Or worth, how many times were you not at home/office and you wanted to remotely connect to your computer at home/office, but there is no internet connectivity at the remote end and there is no one there to recover it? If the answer is too many times, this project is for you! This project will do it all by it self on the spot whenever internet connectivity is lost and practically you'll have continuous internet connectivity whether you're at home or not.
<br/>
<h2>Main Features</h2>

  - Automatically recovers internet connectivity.
  - Supports recovering of router and modem as two separate devices or a single combined device.
  - Supports recovering of an access point.
  - Highly configurable.
  - Supports HTML interface for controlling, configuring and monitoring the device.
  - Supports either WiFi or Wired (w5500) Ethernet connection.
  - Supports manual control using LED indicators and pushbuttons.
  - Supports OTA firmware upgrades (no SSL support, yet).
  - Includes EasyEDA electrical diagram, PCB design and packaging design.

This project is intended to run on ESP32 with SD card reader and optionally a wired ethernet adapter. The code is developed using platformio in visual studio code.

What it does is basically periodically ping a server on the internet and optionally a local address on the LAN. If ping fails, it switches the router power to off and then on after several seconds. Then it waits for internet connectivity to resume. If internet connectivity is not resumed after some time, it switches the modem power to off and then on after several seconds. Then it waits for internet connectivity to resume. If internet connectivity is not resume, it goes again through the recovery attempt cycles until internet connectivity resumes, or maximum recovery cycles exceeded.

It is also possible to configure the device to periodically restart the router and/or the modem at a certain time, once in every 24 hours. This should usually be done during the night when there is low or no Internet usage. This usually prevent the need to recover the devices during the day which might interrupt connectivity for some time.

There are quite a few configurable parameters to this software, like name of server on the internet to ping, an optional alternative internet server to ping if the ping to the first server failed. Also the time for power disconnect and waiting time for connection to resume are configurable and more.

There is also an implementation of a web (HTML) site that is used as a human interface for monitoring, controlling and configuring the software. The web site also has a page for reporting recoveries history. For example, read about the <a href="https://github.com/boazf/IWG/wiki/Control-Page">Control Page</a> in the wiki.

The files in the SD directory should be (tree) copied to an SD card and be placed in the SD card reader.

Edit the content of file config.txt on the SD card. The content is pretty much self-explanatory. The TimeServer parameter is used for setting the server name used for getting the current GMT time using NTP protocol. The TimeZone parameter is used to set the local time. The value is in minutes and can also be negative. DST is number of minutes to add to the time during daylight saving time. Then there are parameters for setting Ethernet. The code uses static IP address so the site address is always the same. Then you can also set the pin numbers for switching the router and modem power using relays. For more elaborated information see <a href="https://github.com/boazf/IWG/wiki/CONFIG.TXT">CONFIG.TXT</a> wiki page.

The project is designed so that it is possible to connect to the LAN using the ESP32's WiFi, or using a wired Ethernet adapter. In case WiFi is used, then the configuration file should also contain the SSID and password to connect to the LAN.

There are also four pushbuttons with a LED in each of them. the pushbuttons are used to initiate connectivity checks and recovery cycles manually. The LEDs are used to report the status of the device, such as normal status when internet is available, or whether a recovery cycle is on-going. For more information see <a href="https://github.com/boazf/IWG/wiki/Manual-Control">Controlling the Device Manually</a> wiki page.

The HTML pages are using bootsrap 3 libraries in order to produce nice looking responsive pages. The bootstrap libraries were renamed to names that conform to 8.3 file names (e.g., bstrap.js). This was done because the project started on a mega2560 board and the only SD library that I had there supported only 8.3 file names. After transferring to ESP32, I could use longer file names, but I left it as it was. Also jquery libraries are used by the JavaScript code. All required jquery libraries were merged to one single library (jqcombo.js) in order to reduce page load time and sockets consumption.</br></br>
For much more information read the <a href="https://github.com/boazf/IWG/wiki">Wiki</a>.

<h2>Important Note for Building the Project for the Wired Version</h2>

The Ethernet library for the W5500 contains a compilation error in the `Ethernet.h` file. The issue arises because the `EthernetServer` class inherits from the `Server` class, which defines a pure virtual `begin` method that accepts an optional `port` parameter. However, `EthernetServer` does not implement this method, resulting in a compilation failure.

This likely compiles without issue in the Arduino IDE due to differences in how the IDE handles pure virtual methods or builds libraries. To resolve the error, add the following line to the public section of the `EthernetServer` class declaration in `Ethernet.h`, located in the directory `<Your repository>\.pio\libdeps\wired\Ethernet\src`:
```C++
virtual void begin(uint16_t port) { if (port) _port = port; begin(); }
```
This method is not used by the project's code, so its implementation is not particularly important. For the sake of completeness, use the suggested implementation.
