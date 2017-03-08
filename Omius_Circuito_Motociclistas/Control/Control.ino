//--Ultimate GPS PINOUT
// GPS VIN -->  5V
// GPS GND -->  GND
// GPS TX  -->  D3
// GPS RX  -->  D2

//--LM35 Temperature Sensors PINOUT
// LEFT   --> 4 - 20 V
// MIDDLE --> OUT --> Sensor1 (A0), Sensor2 (A1)
// RIGHT  --> GND 

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 2);
Adafruit_GPS GPS(&mySerial);

const int gate_MOSFET = 9;
const float temp_toStop = 60;     // adjust this setpoint
const float temp_toStart = 20;    // adjust this setpoint
const float speed_toStart = 10;   // adjust this setpoint

//--GPSECHO --> 'false' to turn off echoing the GPS data to the Serial console
//--GPSECHO --> 'true'  to debug and listen to the raw GPS sentences
#define GPSECHO  false

//--Keep track of whether we're using the interrupt
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

void setup()  {
  Serial.begin(115200);  // Set Serial Monitor baud rate to 115200 too 
  GPS.begin(9600);       // Set 9600 NMEA is the default baud rate

  ///--Uncomment statement to be used: 
  //--Turn on RMC (recommended minimum) and GGA (fix data)
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); 
  //--Turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  
  //--Set update rate to 1 kHz. Recommended for the parsing code to work correctly
  // and have time to sort through the data. 
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  //--Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  //--TIMER0 interrupt goes off every 1 millisecond, and reads data from the GPS.
  useInterrupt(true);

  delay(1000);

  pinMode(gate_MOSFET, OUTPUT);
  digitalWrite(gate_MOSFET, HIGH);
  
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  #ifdef UDR0
    if (GPSECHO)
      if (c) UDR0 = c;  // faster than Serial.print(), but only reads a character at a time
  #endif
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

uint32_t timer = millis();
void loop() {                 
  
  //----in case you are not using the interrupt above, you'll
  if (!usingInterrupt) {  // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // debug here
    if (GPSECHO)
      if (c) Serial.print(c);
  }

  // Read plate's temperature for the first time and measure speed 
  float temp_hot_plate = ( 5.0 * analogRead(A1) * 100.0) / 1024; // Sensor 2
  float temp_body = 0;
  float speed_KMpH = 0;
  
  if (GPS.fix) { speed_KMpH = GPS.speed * 1.852; }
  bool temp_setpoint = temp_hot_plate <= temp_toStop;    
  bool speed_setpoint = speed_KMpH > speed_toStart;       
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis()) {  timer = millis(); }
  
  while (temp_setpoint && speed_setpoint) {

    digitalWrite(gate_MOSFET, HIGH);
    
    if (millis() - timer > 2000) { // approximately every 2 seconds print out the current stats
      timer = millis(); // reset the timer
      Serial.print("Fix: "); Serial.print((int)GPS.fix);
      if (GPS.fix) {
        Serial.print("Speed (Km/H): ");
        float speed_KMpH = GPS.speed * 1.852; // Speed is given in knots, so we have to convert it
        Serial.println(speed_KMpH);
      }
      temp_body = ( 5.0 * analogRead(A0) * 100.0) / 1024; // Sensor 1 
      temp_hot_plate = ( 5.0 * analogRead(A1) * 100.0) / 1024; // Sensor 2
      Serial.print("Body temperature: ");
      Serial.println(temp_body);
      Serial.print("Hot plate temperature: ");
      Serial.println(temp_hot_plate);
      Serial.println();
    }
    // Check conditions 
    temp_setpoint = temp_hot_plate <= temp_toStop;    
    speed_setpoint = speed_KMpH > speed_toStart; 
  }

  while (!(temp_setpoint && speed_setpoint)) {
    
    digitalWrite(gate_MOSFET, LOW);
   
    if (millis() - timer > 2000) { // approximately every 2 seconds print out the current stats
      timer = millis(); // reset the timer
      Serial.print("Fix: "); Serial.print((int)GPS.fix);
      if (GPS.fix) {
        Serial.print("Speed (Km/H): ");
        float speed_KMpH = GPS.speed * 1.852; // Speed is given in knots, so we have to convert it
        Serial.println(speed_KMpH);
      }
      temp_body = ( 5.0 * analogRead(A0) * 100.0) / 1024; // Sensor 1 
      temp_hot_plate = ( 5.0 * analogRead(A1) * 100.0) / 1024; // Sensor 2
      Serial.print("Body temperature: ");
      Serial.println(temp_body);
      Serial.print("Hot plate temperature: ");
      Serial.println(temp_hot_plate);
      Serial.println();
    }
    // Check conditions 
    temp_setpoint = temp_hot_plate >= temp_toStart;    
    speed_setpoint = speed_KMpH > speed_toStart; 
  }
}

