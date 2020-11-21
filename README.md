Arduino Internet Recovery Box
=============================

How many times did you loose internet connecticity, then you had to go to your router and/or modem, disconnect it/them from power wait a while and reconnect and wait for internet connectivity to resume? If the answer is too many time, this project i for you!<br/><br/>
This project is intended to run on an Arduino MEGA2560 board with Ethernet Shield 2. What it does is continuously ping a server on the internet and optionally a local address on the LAN. If ping fails, it switches the router power to off and then on after several seconds. Then it waits for internet connectivity to resume. If internet connectivity is not resumed after some time, it switches the modem power to off and then on after several seconds. Then it waits for internet connectivity to return. If internet connectivity does not return, it goes again through the recovery attempts cycles until internet connectivity returns, or maximum recovery cycles exceeded.
<br/><br/>
There are quite a few configurable parameters to this software, like name of server on the internet to ping, an optional alternative internet server to ping if the ping to the first server failed. Also the time for power disconnect and waiting time for connection to resume is configurable and more.
<br/><br/>
For turning the power off and on, it is necessary to have a two relay board wired to two GPIO pins. The pin numbers are also configurable.
<br/><br/>
There is also an implementation of a web (HTML) site that is used for controlling and configuring the software. The web site also has a page for reporting recoveries history.
<br/><br/>
The files in the SD directory should be (tree) copied to an SD card and be placed in the Ethernet Shield 2.
<br/><br/>
Edit the content of file config.txt on the SD card. The content is pretty much self-explanatory. The TimeServer parameter is used for setting the server name used for getting the current GMT time using NTP protocol. The TimeZone parameter is used to set the local time. The value is in minutes and can also be negative. Then there are parameters for setting Ethernet. The code uses static IP address so the site address is always the same. Then you can also set the pin numbers for switching the router and modem power using relays.
