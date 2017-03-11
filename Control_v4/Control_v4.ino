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

//-----GPS stuff
#define GPSSerial Serial1         // name of hardware serial port Serial .--> arduino uno, Serial1 --> Arduino micro
#define GPSECHO false             // false --> turn off echoing, true --> turn on echoing and debug
Adafruit_GPS GPS(&GPSSerial);     // connect to the GPS on the hardware port

//-----Time stff
uint32_t timer = millis();        // timer check if its time to print information 
int LastTime = 0;
long CurrentTime = 0;
int millisAnterior = 0;
int millisOverflow = 0;

//-----SD stuff
const int CS = 4;                 //Chip select
File datalogFile;                 //Datalog File 

//-----Temperature sensors stuff
const float max_temp = 60;        // adjust this setpoint
const float min_temp = 40;        // adjust this setpoint
const float min_speed = 10;       // adjust this setpoint
float temp_hot_plate = 0;
float temp_body = 0, speed_KMpH = 0;

//-----Bluetooth stuff
SoftwareSerial bluetooth(2, 3);  //TX-O(Pin D2), RX-I (Pin D3)

//-----Other component stuff 
const int gate_MOSFET = 9;       // Mostfate gate; current control

void setup() {
    Serial.begin(9600);  // Set Serial Monitor baud rate to 115200 too
  while (!Serial);       // Do not start until serial monitor is open 
  //----------------------------------------------------
  //-----SD Card
  Serial.print("Init SD card...");
  if (!SD.begin(CS)) { Serial.println("Error, card not pressent"); }  //Check if SD card is present 
  else{ Serial.println("Init successful"); }
  datalogFile = SD.open("mdata.txt", FILE_WRITE);
  if (datalogFile) {
    datalogFile.println("Test");
    datalogFile.println("Time /Speed / T-body / T-hotplate ");
    datalogFile.close();
    Serial.println("Test");
    Serial.println("Time /Speed / T-body / T-hotplate ");
  } else { Serial.println("Error opening .txt"); }
  
  //----------------------------------------------------
  //-----GPS
  GPS.begin(9600);                                  // Set 9600 NMEA is the default baud rate 
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);     //--Turn on RMC (recommended minimum) and GGA (fix data)
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);  //--Turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);        //--Set update rate to 1 kHz. Recommended for the parsing code to work correctly and sort through data
  GPS.sendCommand(PGCMD_ANTENNA);                   //--Request updates on antenna status, comment out to keep quiet
  //--TIMER0 interrupt goes off every 1 millisecond, and reads data from the GPS
  // useInterrupt(false);
  GPSSerial.println(PMTK_Q_RELEASE);                // Ask for firmware version
  
  //----------------------------------------------------
  //----Bluetooth
  bluetooth.print("$");           // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");           // Enter command mode
  delay(100);                     // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);          // Start bluetooth serial at 9600
  Serial.print("Teclea cualquier tecla para iniciar.");
  while (Serial.available() && Serial.read());  // empty buffer //MOD "mySerial"
  while (!Serial.available());                  // wait for data //MOD
  while (Serial.available() && Serial.read());  // empty buffer again //MOD
  delay(1000);
  
  //----------------------------------------------------
  //-----Other component stuff 
  pinMode(gate_MOSFET, OUTPUT);
  digitalWrite(gate_MOSFET, HIGH);
}

void loop() {
  int millisActual = millis();                   //
  
  // This is to be able to send information to the app without timer wrapping around
  if (millisActual > millisAnterior){
    millisAnterior = millisActual;
    CurrentTime = ((millisOverflow * 65535.0) + (millisActual + 32768.0)) - 32768.0;
  } else {
    millisOverflow++;
    millisAnterior = millisActual;
    CurrentTime = ((millisOverflow * 65535.0) + (millisActual + 32768.0)) - 32768;
  }

  char c = GPS.read();                           // Read characacters from GPS
  if (GPSECHO) { if (c) { Serial.print(c); } }   // Echo info from GPS 
  if (GPS.newNMEAreceived()){
    Serial.println(GPS.lastNMEA()); 
    if (!GPS.parse(GPS.lastNMEA())) return;   // if we cannot parse a sentence, we wait for another
  }

  // Read plate's temperature for the first time and measure speed 
  temp_hot_plate = ( 5.0 * analogRead(A1) * 100.0) / 1024; // Sensor 2
  temp_body = 0, speed_KMpH;
  if (GPS.fix) { speed_KMpH = GPS.speed * 1.852; }       // GPS working ?
  bool temp_good = temp_hot_plate <= max_temp;    // temperature less than setpoint to stop?
  bool speed_good = speed_KMpH > min_speed;      // moving?
  
  // Sentence received? cheksum and parse it. Failed to parse it? wait for another
  if (GPS.newNMEAreceived()) { if (!GPS.parse(GPS.lastNMEA())) { return; } }
  // if millis() or timer wraps around, we'll just reset it. 
  if (timer > millis()) {  timer = millis(); }  

  // How to proceed ?  
  // CASE 1(F, F):  T > 60 °c   S < 10 km/h   -- MOSFET LOW  ... this case should not happen 
  // CASE 2(T, F):  T < 60 °c   S < 10 km/h   -- MOSFET LOW  ... test is about to start
  // CASE 3(T, T):  T < 60 °c   S > 10 km/h   -- MOSFET HIGH ... turn on the system, wait for case 4 to happen
  // CASE 4(F, T):  T > 60 °c   S > 10 km/h   -- MOSFET LOW  ... turn off the system 
  while ((!temp_good && !speed_good) || (temp_good && !speed_good)) { 
    digitalWrite(gate_MOSFET, LOW);
    logData();
    // check conditions 
    temp_good = temp_hot_plate <= max_temp;    // Reached highest temperature yet? 
    speed_good = speed_KMpH > min_speed;      // Still moving ? 
    // Do nothing I guess...? 
  } 
  
  while (temp_good && speed_good) {          // You are under 60° C, and moving at least at 10km/h
    digitalWrite(gate_MOSFET, HIGH);         // Allow current to flow through the peltiers 
    logData();
    // check conditions 
    temp_good = temp_hot_plate <= max_temp;    // Reached highest temperature yet? 
    speed_good = speed_KMpH > min_speed;      // Still moving ? 
  } 
  
  while (!temp_good && speed_good) { 
    digitalWrite(gate_MOSFET, LOW);
    logData();
    // check conditions
    temp_good = temp_hot_plate <= min_temp;    // Reached minimal high temperature yet? 
    speed_good = speed_KMpH > min_speed;      // Still moving ? 
  }
}

void logData() {
  String datalog = "";                           // To write on the SD card and send data through BT
  if (millis() - timer > 2000) { // approximately every 2 seconds print out the current stats
    timer = millis(); // reset the timer  
    
    //--GPS Stuff
    Serial.print("Fix: "); Serial.println((int)GPS.fix);
    if (GPS.fix) {
      float speed_KMpH = GPS.speed * 1.852; // Speed is given in knots, so we have to convert it
      Serial.print("Speed (Km/H): "); Serial.println(speed_KMpH);
      datalog += (String) speed_KMpH + ",";
    } else { datalog +=  "0,"; }

    //--Temperature sensors
    temp_body = ( 5.0 * analogRead(A0) * 100.0) / 1024; // Sensor 1 
    temp_hot_plate = ( 5.0 * analogRead(A1) * 100.0) / 1024; // Sensor 2
    datalog += (String) temp_body + "," + (String) temp_hot_plate + ","; 
    Serial.print("Body temperature: "); Serial.println(temp_body);
    Serial.print("Hot plate temperature: "); Serial.println(temp_hot_plate);
    Serial.println();
    
    if(LastTime <= CurrentTime) { // If stuff was typed in the serial monitor
      LastTime = CurrentTime;
      double segundosEjec = CurrentTime / 1000.0;
      datalog += (String)segundosEjec + ","; 
      sendingSensLog(datalog);
    }
        
    //Add data to file 
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

