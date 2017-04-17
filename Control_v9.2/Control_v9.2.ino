//----------------------------------------------------
//-----Ultimate GPS PINOUT
// GPS VIN -->  5V
// GPS GND -->  GND
// GPS TX  -->  D3
// GPS RX  -->  D2
//----------------------------------------------------
//-----LM35 Temperature Sensors PINOUT
// LEFT   --> 4 - 20 V
// MIDDLE --> OUT --> Sensor1 (A0), Sensor2 (A1)
// RIGHT  --> GND
//----------------------------------------------------
//-----SD PINOUT
// CLK    --> SCLK
// D0     --> (MISO)
// D1     --> (MOSI)
// CS     --> 8 (Chip Select)
//----------------------------------------------------
//-----MOSFET PINOUT
//  PIN PELTIER  --> 11
//----------------------------------------------------
//-----SWITCH
//  SWITCH ON --> 4
//----------------------------------------------------
//-----POT
//----------------------------------------------------
//-----Bluetooth
//  RX    --> 3
//  TX    --> 2
//  GND   --> GND
//  VCC   --> 5v
//----------------------------------------------------
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <TimeLib.h>
#include <math.h>

//-----GPS stuff
#define GPSSerial Serial1     // Name of hardware serial port Serial .--> arduino uno, Serial1 --> Arduino micro
#define GPSECHO false         // False --> turn off echoing, true --> turn on echoing and debug
Adafruit_GPS GPS(&GPSSerial);

//-----Time sutff
uint32_t timer_two_sec = millis();  // Timer to check if it's time to print information.
//uint32_t timer_five_sec = millis(); // Timer to check if 5 seconds have passed
//long CurrentTime = 0;
//int millisAnterior = 0, millisOverflow = 0, LastTime = 0;

//-----SD stuff
const int CS = 8; //Chip select
File datalogFile; //Datalog File

//-----Temperature sensors stuff
const float max_temp = 40; // Adjust this setpoint
const float min_temp = 26; // Adjust this setpoint
const float min_speed = 0; // Adjust this setpoint
float temp_hot_plate = 0, temp_body = 0, speed_KMpH = 0;
int casetemp = 0;
double resistencia = 0, voltTemp = 0;
bool temp_good, speed_good, delayIni = true, TempIni = false, TempIniCold = false, startTemp = true;

//-----Bluetooth stuff
SoftwareSerial bluetooth(2, 3); // TX-O(Pin D2), RX-I (Pin D3)

//-----Peltier stuff
const int pin_peltier = 11;      // Mosfet gate; current control
//const int switch_on = 4;
//int duty_cycle = 0;

//-----Average
const int numReadings = 8;

double readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
double total = 0;                  // the running total
double average = 0;                // the average
double tempCold = 0;
double tempHot = 0;

double readingsCold[numReadings];      // the readings from the analog input
int readIndexCold = 0;              // the index of the current reading
double totalCold = 0;                  // the running total
double averageCold = 0;                // the average

void setup() {

  //-----Begin
  Serial.begin(9600); // Set Serial Monitor baud rate to 115200 too
  //while (!Serial);    // Do not start until serial monitor is open
  delay(1000);

  //-----GPS
  //Serial.begin(115200);
  GPS.begin(9600);                               // Set 9600 NMEA is the default baud rate
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  // Turn on RMC (recommended minimum) and GGA (fix data)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);     // Set update rate to 1 kHz. Recommended for the parsing code to work correctly and sort through data
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  GPSSerial.println(PMTK_Q_RELEASE);             // Ask for firmware version

  //-----Time
  setTime(0, 0, 0, 0, 0, 0);

  //-----SD Card
  //Serial.print("Init SD card...");
  if (!SD.begin(CS)) { } // Check if SD card is present Serial.println("Error");
  else {
    Serial.println("Init successful");
  }

  datalogFile = SD.open("D2.txt", FILE_WRITE); // Add header to file
  if (datalogFile) {
    //datalogFile.println("Test");
    datalogFile.println("Time/Speed/BodyTemp/HotTemp");
    datalogFile.close();
  } else {
    Serial.println("Error opening");
  }

  //----Bluetooth
  bluetooth.begin(9600);                       // Start bluetooth serial at 9600, 115200 baud rate can be too fast to relay data reliability.
  delay(1000);

  //-----Average Array
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  //-----Other stuff
  pinMode(pin_peltier, OUTPUT);
  digitalWrite(pin_peltier, LOW);
}

void loop() {
  // if millis() or timers wrap around, we'll just reset them.
  if (timer_two_sec > millis())  {
    timer_two_sec = millis();
  }

  // Read plate's temperature for the first time and measure speed
  if (startTemp) {
    startTemp = false;
    temp_hot_plate = ( 5 * analogRead(A0) * 100.0) / 1024; // Sensor 2
  }
  if (GPS.fix) { speed_KMpH = GPS.speed * 1.852; } // GPS working ?
  temp_good = temp_hot_plate <= max_temp;     // temperature less than setpoint to stop?
  speed_good = speed_KMpH >= min_speed;       // moving?

  // Sentence received? cheksum and parse it. Failed to parse it? wait for another
  //if (GPS.newNMEAreceived()) { if (!GPS.parse(GPS.lastNMEA())) {  } }//return;

  // How to proceed ?
  // CASE 1(F, F):  T > 60 °c   S < 10 km/h   -- MOSFET LOW  ... this case should not happen
  // CASE 2(T, F):  T < 60 °c   S < 10 km/h   -- MOSFET LOW  ... test is about to start
  // CASE 3(T, T):  T < 60 °c   S > 10 km/h   -- MOSFET HIGH ... turn on the system, wait for case 4 to happen
  // CASE 4(F, T):  T > 60 °c   S > 10 km/h   -- MOSFET LOW  ... turn off the system

  // CASE 2 (turn on control system until conditions reach CASE 4
  while (temp_good && speed_good) {           // You are under 60° C, and moving at least at 10km/h
    casetemp = 2;
    digitalWrite(pin_peltier, HIGH);

    char c = GPS.read();
    if (GPS.newNMEAreceived()) { GPS.parse(GPS.lastNMEA()); }
    if (GPS.fix) { speed_KMpH = GPS.speed * 1.852; }  // Speed is given in knots, so we have to convert it

    logData();
  } delayIni = true;

  // CASE 3 (temperature reached its highest setpoint value, must turn off until it reaches lowest setpoint value and go back to CASE 3
  while (!temp_good && speed_good) {
    casetemp = 3;
    digitalWrite(pin_peltier, LOW);
   
    if (delayIni) {
      delayIni = false;
      delay(5000);
    }
    char c = GPS.read();
    if (GPS.newNMEAreceived()) { GPS.parse(GPS.lastNMEA()); }
    if (GPS.fix) { speed_KMpH = GPS.speed * 1.852; }  // Speed is given in knots, so we have to convert it

    logData();
  }
}

void logData() {
  String datalog = ""; // To write on the SD card and send data through BT
  //voltTemp = (5.0 * analogRead(A0))/ 1023;
  //resistencia = ((voltTemp*10000)/(5.0-voltTemp));
  //tempHot = log(resistencia/27155)/(-0.0403);

  tempCold = ( 5.0 * analogRead(A0) * 100.0) / 1024;
  tempHot = ( 5.0 * analogRead(A0) * 100.0) / 1024;
  temp_body = TempAverageCold(tempCold);      // Sensor 1
  temp_hot_plate = TempAverage(tempHot);
//  temp_hot_plate = tempHot;
  temp_good = temp_hot_plate <= min_temp; // Reached minimal high temperature yet?
  speed_good = speed_KMpH >= min_speed;    // Still moving ?
  
  if (millis() - timer_two_sec > 2000) { // approximately every 1 second print out the current stats
    timer_two_sec = millis();            // reset the timer_two_sec

    // Debug case
    //    Serial.print("Case: ");
    //    Serial.println(casetemp);
    //    Serial.print("Status temp_good: ");
    //    Serial.println(temp_good);
    //    Serial.print("Status speed_good: ");
    //    Serial.println(speed_good);
    //    Serial.print("Min Temp: ");
    //    Serial.println(min_temp);

    //--GPS Stuff
    Serial.print("Fix:"); Serial.println((int)GPS.fix);
    if (GPS.fix) {
      //speed_KMpH = GPS.speed * 1.852; // Speed is given in knots, so we have to convert it
      datalog += (String) speed_KMpH + ",";
    }
    else {
      datalog +=  "45,";
    }

    datalog += (String) temp_hot_plate + "," + (String) temp_body + ",";
    Serial.print("Body temp: "); Serial.println(temp_body);
    Serial.print("Hot temp: "); Serial.println(temp_hot_plate);
    //Serial.println();

    double segundosEjec = hour() * 3600.0 + minute() * 60.0 + second();
    datalog = (String)segundosEjec + "," + datalog;
    sendingSensLog(datalog);

    //--Add data to file
    datalogFile = SD.open("D2.txt", FILE_WRITE);
    if (datalogFile) {
      datalogFile.println(datalog);
      datalogFile.close();
      Serial.print("SD:");
      Serial.println(datalog);
    } else {
      Serial.println("Err SD");
    }
  }
}

void sendingSensLog(String sensLog) {
  const char *Log = sensLog.c_str();
  bluetooth.write(Log);
  delay(50);
    Serial.print("BT:");
    Serial.println(sensLog);
}

double TempAverage(double avg) {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = avg;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    TempIni = true;
    readIndex = 0;
  }

  // calculate the average:
  if (!TempIni) {
    average = total / readIndex;
  } else {
    average = total / numReadings;
  }

  //  Serial.println("-----------");
  //  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
  //    Serial.println(readings[thisReading]);
  //  }
  //Serial.println("-----------");
  // send it to the computer as ASCII digits

  return average;
}

double TempAverageCold(double avgCold) {
  // subtract the last reading:
  totalCold = totalCold - readingsCold[readIndexCold];
  // read from the sensor:
  readingsCold[readIndexCold] = avgCold;
  // add the reading to the total:
  totalCold = totalCold + readingsCold[readIndexCold];
  // advance to the next position in the array:
  readIndexCold = readIndexCold + 1;

  // if we're at the end of the array...
  if (readIndexCold >= numReadings) {
    // ...wrap around to the beginning:
    TempIniCold = true;
    readIndexCold = 0;
  }

  // calculate the average:
  if (!TempIniCold) {
    averageCold = totalCold / readIndexCold;
  } else {
    averageCold = totalCold / numReadings;
  }

  //  Serial.println("-----------");
  //  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
  //    Serial.println(readingsCold[thisReading]);
  //  }
  //Serial.println("-----------");
  // send it to the computer as ASCII digits

  return averageCold;
}

