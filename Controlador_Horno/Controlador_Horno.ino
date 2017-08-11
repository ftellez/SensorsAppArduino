#include <SPI.h>
#include "Adafruit_MAX31855.h"

// Default connection is using software SPI, but comment and uncomment one of
// the two examples below to switch between software SPI and hardware SPI:

// Example creating a thermocouple instance with software SPI on any three
// digital IO pins.
#define MAXDO   5
#define MAXCS   4
#define MAXCLK  3
double c, s, e;
double millisActual = 0, millisAnterior = 0;
bool onTemp = true;
bool desactVarTemp = false;

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// Example creating a thermocouple instance with hardware SPI
// on a given CS pin.
//#define MAXCS   10
//Adafruit_MAX31855 thermocouple(MAXCS);

void setup() {
  while (!Serial); // wait for Serial on Leonardo/Zero, etc
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {
  millisActual = millis();
  //if setpoint is given.
  if (s != 0) {
    sensor();
    error();
    desactVarTemp = control(desactVarTemp);
    delay(1000);
  } else {
    desactVarTemp = false;
  }
  setpoint();
}

void setpoint() {
  //demand first input.
  if (s == 0) {
    Serial.println("Please enter setpoint");
    delay(1000);
  }
  //keep checking input.
  if (Serial.available() > 0) {
    s  = ((Serial.readString().toFloat()));
  }
  //Display input.
  Serial.print("Setpoint = ");
  Serial.println(s);
}

void sensor() {
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

void error() {
  //Difference between setpoint and sensed value.
  e = s - c;
  Serial.print("Error = ");
  Serial.println(e);
}

bool control(bool chkStep) {
  //basic comparison in error.
  if (e < -2) {
    digitalWrite(2, LOW);
    return true;
  }
  if (e > 2) {
//    Serial.println("Error mayor a 2 grados.");
    if (!chkStep) {
//      Serial.println("Temperatura por steps.");
//      Serial.print("millisActual : ");
//      Serial.println(millisActual);
//      Serial.print("millisAnterior : ");
//      Serial.println(millisAnterior);
      if ((millisActual - millisAnterior) >= 10000 && onTemp) {
        Serial.println("--------------");
        Serial.println("Relay Apagado.");
        Serial.println("--------------");
        onTemp = false;
        millisAnterior = millisActual;
        digitalWrite(2, LOW);
        Serial.println(">>>>>>>>>Apagado hecho.<<<<<<<<");
      }
      if ((millisActual - millisAnterior) >= 5000 && !onTemp) {
        Serial.println("--------------");
        Serial.println("Relay Encendido.");
        Serial.println("--------------");
        onTemp = true;
        millisAnterior = millisActual;
        digitalWrite(2, HIGH);
        Serial.println(">>>>>>>>>Encendido hecho.<<<<<<<<");
      }
    } else {
      digitalWrite(2, HIGH);
    }
    return chkStep;
  }
}
