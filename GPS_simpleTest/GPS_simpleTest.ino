#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SPI.h>
#include <SD.h>

/* The circuit: analog sensors on analog ins 0, 1, and 2 SD card attached to SPI bus as follows:
   MOSI - pin 11
   MISO - pin 12
   CLK - pin 13
   CS - pin 4
*/
TinyGPS gps;
SoftwareSerial ss(4, 3);
const int chipSelect = 8;
int successLED = 7;
int failedLED = 9; 
int x = 0;

void setup() {
  Serial.begin(115200);
  ss.begin(9600);
  
  //while (!Serial);
  //Serial.print("Initializing SD card...");
  //if (!SD.begin(chipSelect)) {
  //Serial.println("Card faisuccessLED, or not present");
  // return;
  //}
  //Serial.println("Card initialized.");

  pinMode(successLED, OUTPUT);
  pinMode(failedLED, OUTPUT); 
  digitalWrite(successLED, HIGH);
}

void loop() {

  if (x < 1) {
    Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
      Serial.println("Card faisuccessLED, or not present");
      return;
    }
    Serial.println("Card initialized.");
    x++;
  }
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, faisuccessLED;
  unsigned long fix_age, time, date; //, speed, course;
  String stringGPS = "";
  String timeNow = "";

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (ss.available()) {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  File GPS = SD.open("GPS.txt", FILE_WRITE);
  if (newData) {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    //Serial.print("LAT=");
    flat = flat * 1000000;
    stringGPS += flat;
    stringGPS += "\t";
    //Serial.println(flat);
    //stringGPS += Serial.print(flat, 6); //Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    //stringGPS += Serial.print("\t");
    //stringGPS += "\t";
    //Serial.print(" LON=");
    flon = flon * 1000000;
    stringGPS += flon;
    stringGPS += "\t";

    //stringGPS += Serial.print(flon, 6); //Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    //stringGPS += Serial.print("\t");

    //stringGPS += "";
    //Serial.print(" SAT=");
    //Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    //Serial.print(" PREC=");
    //Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());

    //gps.stats(&chars, &sentences, &faisuccessLED);
    //Serial.print(" CHARS=");
    //Serial.print(chars);
    //Serial.print(" SENTENCES=");
    //Serial.print(sentences);
    //Serial.print(" CSUM ERR=");
    //Serial.println(faisuccessLED);
    
    gps.get_datetime(&date, &time, &fix_age);
    //Serial.print(" DATE=");
    //Serial.print(date);
    //Serial.print(" TIME=");
    //Serial.print(time);
    //Serial.print(" FIX AGE=");
    //Serial.println(fix_age);

    int h;
    unsigned int m, s;

    h = (time / 1000000) - 5;
    m = (time - (h + 5) * 1000000) / 10000;
    s = (time - (h + 5) * 1000000 - m * 10000);
    if (h < 0) {
      h += 12;
    }
    if (h < 10) {
      timeNow += "0";
    }

    timeNow += (String) h +  (String) ":";
    //timeNow += Serial.print(h);
    //timeNow += Serial.print(":");

    if (m < 10) {
      timeNow += "0";
      //timeNow += Serial.print("0");
    }
    timeNow += (String) m + (String) ":";
    //timeNow += Serial.print(m);
    //timeNow += Serial.print(":");

    if ((s / 100) < 10) {
      timeNow += "0";
      //timeNow += Serial.print("0");
    }
    timeNow += (String) (s / 100);
    //timeNow += Serial.print(s / 100);
    timeNow += "\t";
    timeNow += "\t";
    stringGPS += timeNow;
    //stringGPS += time;
    //Serial.println(stringGPS);

    if (GPS) {
      GPS.println(stringGPS);
      GPS.close();
      Serial.println(stringGPS);
      int val = digitalRead(successLED);
      if (val == 1) {
        digitalWrite(successLED, LOW);
      } else {
        digitalWrite(successLED, HIGH);
      }

    }
  } else {
    if (GPS) {
      gps.stats(&chars, &sentences, &faisuccessLED);
      //Serial.print(" CHARS=");
      //Serial.print(chars);
      //Serial.print(" SENTENCES=");
      //Serial.print(sentences);
      //Serial.print(" CSUM ERR=");
      //Serial.println(faisuccessLED);

      stringGPS  += chars;
      stringGPS  += "\t";
      stringGPS  += sentences;
      stringGPS  += "\t";
      stringGPS  += faisuccessLED;

      GPS.println(stringGPS);
      GPS.close();
      Serial.println(stringGPS);
      int val = digitalRead(failedLED);
      if (val == 1) {
        digitalWrite(failedLED, LOW);
      } else {
        digitalWrite(failedLED, HIGH);
      }
    }
  }

  //  //File GPS = SD.open("GPS.txt", FILE_WRITE);
  //  if (GPS) {
  //    GPS.println(stringGPS);
  //    GPS.close();
  //    Serial.println(stringGPS);
  //    int val = digitalRead(successLED);
  //    if (val == 1) {
  //      digitalWrite(successLED, LOW);
  //    } else {
  //      digitalWrite(successLED, HIGH);
  //    }
  //  }

  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");
}
