# GPS-Tracker
A GPS tracker that logs data received from GNSS satellites using the NeoGPS library.

------------------------------------------------------------------------------------

The data logged is saved on a SD card, with an interactive GUI comprised of 1 LCD screen 8x2 and 4 button presses. The rebouncing is controlled via the Bounce2 library and the SD control is done with the generic Arduino SD library. Data are saved in a .csv format for ease of analysis. 

The format is as follow: DATE, TIME, LATITUDE, LONGTITUDE, NB OF SATELLITES, [ID ELEVATION/AZIMUTH].

Serial Baud: 4800, GPS Baud: 9600, Frames Received: .RMC .GGA .GSV , Last sentence received is .RMC, RX_PIN is 3 and TX_PIN is 2. The serial monitor used is the default SoftwareSerial on the Arduino IDE for ease of use and to reduce dependency.
