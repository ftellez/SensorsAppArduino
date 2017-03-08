// This code just echoes whatever is coming from the GPS unit to the serial monitor.
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

// GPS VIN ---> 5V
// GPS GND ---> GND
// GPS TX  ---> D3
// GPS RX  ---> D2

/* GPS NMEA that will be needed are 
   $GPGGA - Global Positioning System Fix Data
   $GPRMC - Recommended minimum specific GPS/Transit data
   $GPZDA - Date & Time
 */

SoftwareSerial mySerial(3, 2);
Adafruit_GPS GPS(&mySerial);

// GPSECHO ---> 'false' to turn off echoing the GPS data to the Serial console
// GPSECHO ---> 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true

// this keeps track of whether we're using the interrupt off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

void setup()  {    
  // Set baud rate in the Serial Monitor to 115200 
  Serial.begin(115200);
  // Set NMEA baud rate to 9600 (default) 
  GPS.begin(9600);
  
  // Turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Turn on only the "minimum recommended" data for high update rates
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Turn on all the available data - for 9600 baud you'll want 1 Hz rate
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
  
  // Set the update rate: change both output rate (how often poition is written to the serial line)
  // and the position fix rate.
  
  // 1 Hz update rate
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  //GPS.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
  
  // 5 Hz update rate- for 9600 baud you'll have to set the output to RMC or RMCGGA only (see above)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);
  
  // 10 Hz update rate - for 9600 baud you'll have to set the output to RMC only (see above)
  // Note the position can only be updated at most 5 times a second so it will lag behind serial output.
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  //GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);

  // Request updates on antenna status
  GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. 
  useInterrupt(true);
  
  delay(1000);
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

void loop(){
   //all reading and printing is done in the interrupt
}
