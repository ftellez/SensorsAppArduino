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
// CLK    --> 13
// D0     --> 12  (MISO)
// D1     --> 11  (MOSI)
// CS     --> 10  (Chip Select)
//----------------------------------------------------
//-----MOSFET PINOUT
//  GATE  --> 9
//----------------------------------------------------
//-----SWITCH
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

//-----GPS stuff
#define GPSSerial Serial1     // Name of hardware serial port Serial .--> arduino uno, Serial1 --> Arduino micro
#define GPSECHO false         // False --> turn off echoing, true --> turn on echoing and debug
Adafruit_GPS GPS(&GPSSerial);

//-----Time sutff
uint32_t timer_two_sec = millis();  // Timer to check if it's time to print information. 
uint32_t timer_five_sec = millis(); // Timer to check if 5 seconds have passed 
long CurrentTime = 0;
int millisAnterior = 0, millisOverflow = 0, LastTime = 0;

//-----SD stuff
const int CS = 8; //Chip select
File datalogFile; //Datalog File 

//-----Temperature sensors stuff
const float max_temp = 30; // Adjust this setpoint
const float min_temp = 26; // Adjust this setpoint
const float min_speed = 0; // Adjust this setpoint
float temp_hot_plate = 0, temp_body = 0, speed_KMpH = 0;
int casetemp = 0;

//-----Bluetooth stuff
SoftwareSerial bluetooth(2, 3); // TX-O(Pin D2), RX-I (Pin D3)

//-----Peltier stuff
const int pin_peltier = 11;      // Mosfet gate; current control      
const int switch_on = 4;
int duty_cycle = 0;

void setup() {
  
  //-----Begin
  Serial.begin(9600); // Set Serial Monitor baud rate to 115200 too
  //while (!Serial);    // Do not start until serial monitor is open 

  //-----Time
  setTime(0,0,0,0,0,0);
  
  //-----SD Card
  Serial.print("Init SD card...");
  if (!SD.begin(CS)) { Serial.println("Error"); } // Check if SD card is present 
  else{ Serial.println("Init successful"); }
  
  datalogFile = SD.open("mdata.txt", FILE_WRITE); // Add header to file
  if (datalogFile) {
    datalogFile.println("Test Fin");
    datalogFile.println("Time / Speed  / Body Temp / Peltier Temp");
    datalogFile.close();
    //Serial.println("Test");
    Serial.println("Time / Speed  / Body Temp / Peltier Temp");
  } else { Serial.println("Error opening .txt"); }
  
  //-----GPS
  GPS.begin(9600);                               // Set 9600 NMEA is the default baud rate 
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  // Turn on RMC (recommended minimum) and GGA (fix data)
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); // Turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);     // Set update rate to 1 kHz. Recommended for the parsing code to work correctly and sort through data
  //GPS.sendCommand(PGCMD_ANTENNA);                // Request updates on antenna status, comment out to keep quiet
//  GPSSerial.println(PMTK_Q_RELEASE);             // Ask for firmware version
  delay(1000);
  
  //----Bluetooth
//  bluetooth.print("$");                        // Print three times individually
//  bluetooth.print("$");
//  bluetooth.print("$");                        // Enter command mode
//  delay(100);                                  // Short delay, wait for the Mate to send back CMD
//  bluetooth.println("U,9600,N");               // Temporarily Change the baudrate to 9600, no parity
  bluetooth.begin(9600);                       // Start bluetooth serial at 9600, 115200 baud rate can be too fast to relay data reliability. 
  //Serial.print("Hit any key to start");
  //while (Serial.available() && Serial.read()); // Empty buffer //MOD "mySerial"
  //while (!Serial.available());                 // Wait for data //MOD
  //while (Serial.available() && Serial.read()); // Empty buffer again //MOD
  delay(1000);
  
  //-----Other stuff  
  pinMode(switch_on, INPUT); 
  pinMode(pin_peltier, OUTPUT);
  analogWrite(pin_peltier, 0);
}

void loop() {
  
  if (digitalRead(switch_on) == HIGH) {
  
    // if millis() or timers wrap around, we'll just reset them. 
    if (timer_two_sec > millis())  { timer_two_sec = millis(); }  
    
    // parte adapatada para YC combinator
    duty_cycle = map(analogRead(A2), 0, 990, 0, 250); // Read pot to adjust duty cycle (Max voltage is 4.84V)
    analogWrite(pin_peltier, duty_cycle); 
    logData();
  }
}

void logData() {

  String datalog = "45, "; 
  
  if (millis() - timer_two_sec > 700) { // approximately every 2 seconds print out the current stats
    timer_two_sec = millis();            // reset the timer_two_sec  
   
    //--Temperature sensors
    temp_body = 0; 
    temp_hot_plate = 0; 
    temp_body = ( 5.0 * analogRead(A1) * 100.0) / 1024;      // Sensor 1 
    temp_hot_plate = ( 5.0 * analogRead(A0) * 100.0) / 1024; // Sensor 2
    
    datalog += (String) temp_body + "," + (String) temp_hot_plate + ","; 
    Serial.print("Body temperature: "); Serial.println(temp_body);
    Serial.print("Hot plate temperature: "); Serial.println(temp_hot_plate);
    Serial.println();
    
    double segundosEjec = hour()*3600.0 + minute()*60.0 + second();
    datalog = (String)segundosEjec + "," + datalog; 
    sendingSensLog(datalog);
        
    //--Add data to file 
    datalogFile = SD.open("mdata.txt", FILE_WRITE);
    if (datalogFile) {
      datalogFile.println(datalog);
      datalogFile.close();
      Serial.print("SD Datos Guardados : ");
      Serial.println(datalog); 
    } else { Serial.println("Error opening mdata.txt"); }
  }
}

void sendingSensLog(String sensLog) {
  const char *Log = sensLog.c_str();
  bluetooth.write(Log);
  delay(50);
  Serial.print("Bluetooth Datos Enviados : ");
  Serial.println(sensLog);
}
