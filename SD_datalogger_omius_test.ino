/*
 * SD datalogger
 * 
 * Pinout:
 *        CLK 13 (Clock source)
 *        DO  12 (MISO)
 *        DI  11 (MOSI)
 *        CS  10 (Chip Select)
 */
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <SD.h>

const int CS = 10; // Chip select 
DHT_Unified dht(2, DHT11);  //-- Sensor configuration.
unsigned int delayms;       //-- Same as uint32_t.

void setup() {
  Serial.begin(9600);
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
  if (!SD.begin(CS)) {
    Serial.println("Error, card not pressent");
    return;
  }
  Serial.println("Init successful");
}

void loop() {
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

  //Add data to file 
  File datalogFile = SD.open("zcrypt.txt", FILE_WRITE);
  if (datalogFile) {
    datalogFile.println(datalog);
    datalogFile.close();
    Serial.println(datalog);
  } else { Serial.println("Error opening datalog.txt"); }
  
  delay(500);
}

