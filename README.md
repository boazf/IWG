Arduino Internet Recovery Box
=============================

How many times have you lost internet connectivity, then you had to go to your router and/or modem, disconnect it/them from power, wait a while,  reconnect and wait for internet connectivity to resume? Or worth, how many times were you not at home/office and you wanted to remotely connect to your computer at home/office, but there is no internet connectivity at the remote end and there is no one there to recover it? If the answer is too many times, this project is for you! This project will do it all by it self on the spot whenever internet connectivity is lost and practically you'll have continuous internet connectivity whether you're at home or not.
<br/><br/>
This project is intended to run on ESP32 with SD card reader and optionally a wired ethernet adapter. The code is developed using platformio in visual studio code.
<br/><br/>
What it does is basically periodcall ping a server on the internet and optionally a local address on the LAN. If ping fails, it switches the router power to off and then on after several seconds. Then it waits for internet connectivity to resume. If internet connectivity is not resumed after some time, it switches the modem power to off and then on after several seconds. Then it waits for internet connectivity to resume. If internet connectivity is not resume, it goes again through the recovery attempt cycles until internet connectivity resumes, or maximum recovery cycles exceeded.
<br/><br/>
There are quite a few configurable parameters to this software, like name of server on the internet to ping, an optional alternative internet server to ping if the ping to the first server failed. Also the time for power disconnect and waiting time for connection to resume are configurable and more.
<br/><br/>
There is also an implementation of a web (HTML) site that is used as a human interface for monitoring, controlling and configuring the software. The web site also has a page for reporting recoveries history.
<br/><br/>
The files in the SD directory should be (tree) copied to an SD card and be placed in the SD card reader.
<br/><br/>
Edit the content of file config.txt on the SD card. The content is pretty much self-explanatory. The TimeServer parameter is used for setting the server name used for getting the current GMT time using NTP protocol. The TimeZone parameter is used to set the local time. The value is in minutes and can also be negative. DST is number of minutes to add to the time during daylight saving time. Then there are parameters for setting Ethernet. The code uses static IP address so the site address is always the same. Then you can also set the pin numbers for switching the router and modem power using relays.
<br/><br/>
The project is designed so that it is possible to connect to the LAN using the ESP32's WiFi, or using a wired Ethernet adapter. In case WiFi is used, then the configuration file should also contain the SSID and password to connect to the LAN.
<br/><br/>
There are also four pushbuttons with a LED in each of them. the pushbuttons are used to initiate connectivity checks and recovery cycles manually. The LEDs are used to report the status of the box, such as normal status when imternet is available, or whether a recovery cycle is on-going.
<br/><br/>
For much more information read the Wiki
