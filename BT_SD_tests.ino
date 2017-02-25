
/*
 * SD datalogger
 * 
 * Pinout:
 *        CLK 13 (Clock source)  52 (mega)
 *        DO  12 (MISO)          50 (mega)
 *        DI  11 (MOSI)          51 (mega)
 *        CS  10 (Chip Select)   53 (mega)
 */
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>  

//Dynamic voltage
int pinVoltaje = A0;  // Pin from which battery voltage is read
int offset = 0;      // Offset of the arduino source voltage (50 mV)
int voltagePrev = 0;  // For checking the change in voltage
String strVoltage;
int valueTest = 0;
int LastTime = 0;
long CurrentTime = 0;
double segundosEjec = 0;
int minutosEjec = 0;
int millisActual = 0;
int millisAnterior = 0;
int millisOverflow = 0;
String sendvalue;

//Bluetooth
int bluetoothTx = 2;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, Arduino D3
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

//Temperature
const int CS = 53; // Chip select 
DHT_Unified dht(4, DHT11);  //-- Sensor configuration.
unsigned int delayms;       //-- Same as uint32_t.

void setup() {
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  while (!Serial) ;

  // Init DHT11
  Serial.println("Init DHT11 sensor");
  dht.begin();                           //-- Initializating DHT sensor.
  sensor_t dht11;                        //-- Create object. 
  dht.temperature().getSensor(&dht11);   //-- Getting temperature details.
  dht.humidity().getSensor(&dht11);      //-- Getting humidity details.
  delayms = dht11.min_delay / 1000;      //-- Setting up the delay of sensor readings.
  // Init SD 
  Serial.print("Init SD card...");
  //Check if SD card is present 
  if (!SD.begin(CS)) { Serial.println("Error, card not pressent"); } 
  else{ Serial.println("Init successful"); }
  
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600

  Serial.print("Teclea cualquier tecla para iniciar.");
  while (Serial.available() && Serial.read()); // empty buffer //MOD "mySerial"
  while (!Serial.available());                   // wait for data //MOD
  while (Serial.available() && Serial.read()); // empty buffer again //MOD
  
  File datalogFile = SD.open("zcrypt.txt", FILE_WRITE);
  if (datalogFile) {
    datalogFile.println("Test");
    datalogFile.println("Tiempo / Voltaje / Temperatura / Humedad");
    datalogFile.close();
    Serial.println("Test");
    Serial.println("Tiempo / Voltaje / Temperatura / Humedad");
  } else { Serial.println("Error opening .txt"); }

  
}

void loop() {

  millisActual = millis();
  
  String datalog = "";  //String to assemble data to be sent
  sensors_event_t dht11;

  //Temperature 
  dht.temperature().getEvent(&dht11);
  if (isnan(dht11.temperature)) { datalog += "X,"; }
  else { datalog += (String) dht11.temperature + ","; }

  //Humidity 
  dht.humidity().getEvent(&dht11);
  if (isnan(dht11.relative_humidity)){ datalog += "X,"; }
  else { datalog += (String) dht11.relative_humidity + ","; }
  
  if (millisActual > millisAnterior){
    millisAnterior = millisActual;
    CurrentTime = ((millisOverflow * 65535.0) + (millisActual + 32768.0)) - 32768.0;
  } else {
    millisOverflow++;
    millisAnterior = millisActual;
    CurrentTime = ((millisOverflow * 65535.0) + (millisActual + 32768.0)) - 32768;
  }

  int medidaVolt = analogRead(pinVoltaje); // Reads voltage of the battery
  double voltage = ((medidaVolt * 5.0) / 1023.0) + (offset / 100.0);

  if(LastTime <= CurrentTime) { // If stuff was typed in the serial monitor
    LastTime = CurrentTime;
    segundosEjec = CurrentTime / 1000.0;
    datalog = (String)segundosEjec + "," + (String)voltage + "," + datalog; 
    sendingSensLog(datalog);
  }

  //Add data to file 
  File datalogFile = SD.open("zcrypt.txt", FILE_WRITE);
  if (datalogFile) {
    datalogFile.println(datalog);
    datalogFile.close();
    Serial.println(datalog);
  } else { Serial.println("Error opening .txt"); }
  
  delay(700);
  
}

void sendingSensLog(String sensLog) {
  const char *Log = sensLog.c_str();
  bluetooth.write(Log);
  delay(50);
  Serial.print("Dato Enviado : ");
  Serial.println(sensLog);
}
