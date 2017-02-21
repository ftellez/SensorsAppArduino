#include <SPI.h>
#include "Adafruit_MAX31855.h"

// Default connection is using software SPI, but comment and uncomment one of
// the two examples below to switch between software SPI and hardware SPI:

// Example creating a thermocouple instance with software SPI on any three
// digital IO pins.
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5
double c,s,e;
int OnTime, OffTime, cont = 0;

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// Example creating a thermocouple instance with hardware SPI
// on a given CS pin.
//#define MAXCS   10
//Adafruit_MAX31855 thermocouple(MAXCS);

void setup() {
  while (!Serial); // wait for Serial on Leonardo/Zero, etc
  pinMode(6, OUTPUT);
  Serial.begin(9600);
  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
  Serial.println("Introduce los segundos de apagado: ");
  OffTime = getNumber();
  Serial.println("Introduce los segundos de encendido: ");
  OnTime = getNumber();
}

void loop() {
  setpoint();
  //if setpoint is given.
  if(s != 0) {
  sensor();
  error();
  control();
  delay(1000);
  }
}

void setpoint(){  
  //demand first input.
  if (s == 0){
    Serial.println("Please enter setpoint");
    delay(1000);
  }
  //keep checking input.
  if (Serial.available()>0){
    s  = ((Serial.readString().toDouble()));
  }
  //Display input.
  Serial.print("Setpoint = ");
  Serial.println(s);
}

void control(){
  //basic comparison in error.
  if (e > -2) {
    digitalWrite(6, LOW);
    }
  if (e > 2) {
      while (cont < OffTime){
        cont++;
        digitalWrite(6, LOW);  
      }
      while (cont < OnTime){
        cont++;
        digitalWrite(6, HIGH);
      }
    }
  }
  
void sensor(){
  // basic readout test, just print the current temp
   Serial.print("Internal Temp = ");
   Serial.println(thermocouple.readInternal());

   c = thermocouple.readCelsius();
   if (isnan(c)) {
     Serial.println("Something wrong with thermocouple!");
   } else {
     Serial.print("C = "); 
     Serial.println(c);
   }
   //Serial.print("F = ");
   //Serial.println(thermocouple.readFarenheit());
  }

void error(){
  //Difference between setpoint and sensed value.
  e = s-c; 
  Serial.print("Error = ");
  Serial.println(e);
  }

int getNumber(){
  int secs = Serial.read();
  secs = (secs * 1000) / 12;
  return secs;
}

