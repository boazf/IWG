Internet Recovery Box
=====================

This project is intended to run on an Arduino MEGA2560 board with Ethernet Shield 2. 
What it does is continuously ping a server on the internet and optionally a local address on the LAN. 
If ping fails, it switches the router power to off and then on after several seconds. Then it waits for 
internet connectivity to resume. If internet connectivity is not resumed after some time, it switches
the modem power to off and then on after several seconds. Then it waits for internet connectivity to return.
If internet connctivity does not return, it goes again through the recovery attemps cycles until internet
connectivity returns, or maximum recovery cycles exceeded.
There are quite a few configurable paramters to this software, like name of server on the internet to ping,
an optional alternative internet server to ping if the ping to the first server failed. Also the time for 
power disconnect and waiting time for connection resume is configurable and more.
For turning the power off and on, it is neccessary to have a two relay board wired to two GPIO pins. The pin
numbers are also configurable.
There is also an implementation of a web (HTML) site that is used for controlloing and configuring the software.
