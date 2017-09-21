#include "Wire.h"
#include "Si7020.h"

//Encendido configuracion 1 : Input 1 ON, Input 2 OFF
//Encendido configuracion 2 : Input 1 OFF, Input 2 ON (Output 1 [primero trasero] negativo, Output 2 [segundo delantero] positivo)


//---Peltier---
double setpoint = 23.50;
double error = 0;
double potencia = 0;
unsigned long lastTime;
double errSum, lastErr;
double kp = 1000;
double ki = 0;
double kd = 0;
int peltier = 6;
int input1 = 8;
int input2 = 7;

//----Thermistor-------------
int ThermistorPin = A0;
int Vo;

//----Mosfet Skin-------
int skin = 10;
double error2 = 0;
double potencia2 = 0;
unsigned long lastTime2;
double errSum2, lastErr2;
double kp2 = 1000;
double ki2 = 0;
double kd2 = 0;

//----Current Measurement-----
int ledPin = 13;       // outside leads to ground and +5V

//----Si7020------------------
Si7020 sensor;

//------ Skin stabilizer
int stable = 0;  // Put "1" to stabilize the skin temperature in 33.5 ºC
float skinsetpoint = 33.5;


void setup() {


  Serial.begin(57600);
  pinMode(peltier, OUTPUT);
  pinMode(input2, OUTPUT);
  pinMode(input1, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(skin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  digitalWrite(input1, LOW);
  digitalWrite(input2, HIGH);
  sensor.begin();

}

void loop() {
  // Measure RH
  double rh = sensor.getRH();
  double t = sensor.readTemp();
  
  /*How long since we last calculated*/
  unsigned long now = millis();
  double timeChange = (double)(now - lastTime);

  /*Compute all the working error variables*/
  error = setpoint - t;
  errSum += (error * timeChange);
  double dErr = (error - lastErr) / timeChange;

  /*Remember some variables for next time*/
  lastErr = error;
  lastTime = now;

  /*Compute PID Output*/
  potencia = -kp * error - ki * errSum - kd * dErr;

  if (potencia < 0) {
    potencia = 0;
  }
  analogWrite(peltier, potencia);

  //Skin control
  double skinTemp = chkTemp();
  if (stable == 1) {
    /*Compute all the working error variables*/
    error2 = skinsetpoint - skinTemp;
    errSum2 += (error2 * timeChange);
    double dErr2 = (error2 - lastErr2) / timeChange;

    /*Remember some variables for next time*/
    lastErr2 = error2;
    
    /*Compute PID Output*/
    potencia2 = kp2 * error2 + ki2 * errSum2 + kd2 * dErr2;
    
    if (potencia2 < 0) {
      potencia2 = 0;
    }

    analogWrite(skin, potencia2);
  }
  else {
    analogWrite(skin, 146); //146 Fan OFF / 207 Fan ON
  }
  
  //Serial prints
  Serial.println("-----------------------");
  Serial.print("Box Temperature: ");
  Serial.println(t);
  Serial.print("Skin Temperature: ");
  Serial.println(chkTemp());
  Serial.print("Cooling power [%]: ");
  Serial.println(potencia / 255 * 100);
  //  Serial.print(" ");
  //  Serial.print(errSum);
  //  Serial.print(" ");
  //  Serial.println(potencia/255*100);
  //  Serial.print("Error :");
  //  Serial.println(error);
  //  Serial.print("Error derivado :");
  //  Serial.println(dErr);
  //  Serial.print("Error integral :");
  //  Serial.println(errSum);
  //  Serial.print("Potencia :");
  //  Serial.println(potencia);
  //  Serial.print("Thermistor :");
  //  Serial.println(chkTemp());
  delay(500);              // wait for half a second
}

//Thermistor function
float chkTemp() {
  Vo = analogRead(ThermistorPin);
  float alpha = Vo / 1023.0;
  return (50.00 / 503.00 * (sqrt(10.00) * sqrt(3133827.00 - 1006000.00 * alpha) - 4950.00) + 24.00);
}

