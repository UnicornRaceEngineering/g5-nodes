GPS Node
============
Receive GPS data and broadcast these on the CAN system.

Requirements
--------------
Connect to external GPS module.
Parse GPS data and extract the following values:

* latitude
* longitude
* speed in km/h

latitude and longitude must contain:

* North/South or East/West indicator
* degrees
* minutes
* seconds

The GPS data must be broadcasted via the CAN system with id INSERT_ID_HERE

