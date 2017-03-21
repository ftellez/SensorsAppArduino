#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SPI.h>
#include <SD.h>
/*
  The circuit: analog sensors on analog ins 0, 1,
  and 2 SD card attached to SPI bus as follows:
   MOSI - pin 11
   MISO - pin 12
   CLK - pin 13
   CS - pin 4
*/
TinyGPS gps;
SoftwareSerial ss(4, 3);
const int chipSelect = 8;
int success_LED = 7;
int failed_LED = 9;
int x = 0;

void setup() {
  Serial.begin(115200);
  ss.begin(9600);
  pinMode(success_LED, OUTPUT);
  pinMode(failed_LED, OUTPUT);
  digitalWrite(success_LED, HIGH);
}

void loop() {
  //-- Variables.
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, faisuccess_LED;
  unsigned long fix_age, time, date; //, speed, course;
  String stringGPS = "";
  String timeNow = "";

  //-- Initialize card.
  if (x < 1) {
    Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
      Serial.println("Card faisuccess_LED, or not present");
      return;
    }
    Serial.println("Card initialized.");
    x++;
  }

  //-- For one second we parse GPS data and report some key values. 
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (ss.available()) {
      char c = ss.read();
      // Serial.write(c); //-- Uncomment this line if you want to see the GPS data flowing. 
      if (gps.encode(c))  //-- New Data came in?
        newData = true;
    }
  }

  File GPS = SD.open("GPS.txt", FILE_WRITE);    //-- Start writing on .txt file. 

  //-- New valid information is coming. 
  if (newData) {
    float flat, flon;
    unsigned long age;
    
    int h;
    unsigned int m, s;

    //-- Getting position. 
    gps.f_get_position(&flat, &flon, &age);

    //-- Latitude. 
    //Serial.print("LAT=");
    flat = flat * 1000000;
    stringGPS += flat;
    stringGPS += "\t";
    //Serial.println(flat); 

    //-- Longitude. 
    //Serial.print(" LON=");
    flon = flon * 1000000;
    stringGPS += flon;
    stringGPS += "\t";
    //Serial.println(flon);

    //-- Optional: number of satellites and dilution of precision. 
    //Serial.print(" SAT=");
    //Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    //Serial.print(" PREC=");
    //Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());

    //-- Getting time. 
    gps.get_datetime(&date, &time, &fix_age);
    //Serial.println(flat);
    //Serial.print(" TIME=");
    //Serial.print(time);
    //Serial.print(" FIX AGE=");
    //Serial.println(fix_age);
    
    //-- Fixing time. 
    h = (time / 1000000) - 5;
    m = (time - (h + 5) * 1000000) / 10000;
    s = (time - (h + 5) * 1000000 - m * 10000);
   
    if (h < 0)  { h += 12; }
    if (h < 10) { timeNow += "0"; }
    timeNow += (String) h +  (String) ":";
    
    if (m < 10) { timeNow += "0"; }
    timeNow += (String) m + (String) ":";

    if ((s / 100) < 10) { timeNow += "0"; }
    timeNow += (String) (s / 100);
    
    stringGPS += timeNow;
 
    if (GPS) {
      GPS.println(stringGPS);
      GPS.close();
      Serial.println(stringGPS);
      int val = digitalRead(success_LED);
      if (val == 1) { digitalWrite(success_LED, LOW); }
      else { digitalWrite(success_LED, HIGH); }
    }
  //-- No valid information is comming. 
  } else {
    if (GPS) {
      gps.stats(&chars, &sentences, &faisuccess_LED);
      //Serial.print(" CHARS=");
      //Serial.print(chars);
      //Serial.print(" SENTENCES=");
      //Serial.print(sentences);
      //Serial.print(" CSUM ERR=");
      //Serial.println(faisuccess_LED);
      stringGPS  += chars;
      stringGPS  += "\t";
      stringGPS  += sentences;
      stringGPS  += "\t";
      stringGPS  += faisuccess_LED;

      GPS.println(stringGPS);
      GPS.close();
      Serial.println(stringGPS);
      int val = digitalRead(failed_LED);
      if (val == 1) { digitalWrite(failed_LED, LOW);}
      else { digitalWrite(failed_LED, HIGH); }
    }
  }

  if (chars == 0) { Serial.println("** No characters received from GPS: check wiring **"); }
}
