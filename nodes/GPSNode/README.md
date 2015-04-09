GPS Node
============
Receive GPS data and broadcast these on the CAN system.

GPS Module
--------------
Connect to external GPS module.
Parse GPS data and extract the following values:

* latitude
* longitude
* speed in km/h

When transmitting latitude and longitude must be represented as DD (Decimal Degrees).

____________________________________

ADC Readings
------------
Specification for ADC refresh rates is listed in Sensor requirement of the g5-documentation

Read cooler water output temperature on ADC 6 (Port F Pin 6)

Read cooler water input temperature on ADC 7 (Port F Pin 7)

Read the digital value of oil pressure sensor on ADC 5 (Port F Pin 5)

____________________________________

The GPS data must be broadcasted via the CAN system with id INSERT_ID_HERE

