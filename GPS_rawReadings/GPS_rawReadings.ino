#include <SoftwareSerial.h>

SoftwareSerial GPS(4, 3);  //-- 4 is TX and 3 is RX. 

char data = ' ';          //-- Store data received by the arduino. 

/*
 * We will read data from the GPS following the NMEA (National Marine Electronics Asociation) protocol; 
 * Example: 
    $GPRMC,044235.000,A,4322.0289,N,00824.5210,W,0.39,65.46,020615,,,A*44
     -  044235.000      Global hour (04:42:35)
     -   A”             Info is fixed and correct.  
         V”             Info is invalid. 
     -  4322.0289       Longitude (43º 22.0289´).
     -  N               Represents the North.
     - 00824.5210       Latitude (8º 24.5210´).
     -  W               Represents the West.
     - 0.39             Velocity in nodes.
     - 65.46            Orientation in degrees.
     - 020615           Represents the date (2-June-2015.
 */

void setup() {
  Serial.begin(115200); 
  GPS.begin(9600); 
}

void loop() {
  if (GPS.available()){
    data = GPS.read(); 
    Serial.print(data); 
  }
}

