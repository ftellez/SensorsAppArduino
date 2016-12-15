/*
  Example Bluetooth Serial Passthrough Sketch
 by: Jim Lindblom
 SparkFun Electronics
 date: February 26, 2013
 license: Public domain

 This example sketch converts an RN-42 bluetooth module to
 communicate at 9600 bps (from 115200), and passes any serial
 data between Serial Monitor and bluetooth module.
 */
#include <SoftwareSerial.h>  

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

int bluetoothTx = 2;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

void setup()
{
  Serial.begin(9600);  // Begin the serial monitor at 9600bps

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
  
//  bluetooth.write("Omius App"); //Start message
}

void loop()
{
  millisActual = millis();
  
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

  if(LastTime <= CurrentTime)  // If stuff was typed in the serial monitor
  {
    LastTime = CurrentTime;
    segundosEjec = CurrentTime / 1000.0;
    sendvalue = (String)segundosEjec + "," + (String)voltage + ","; 
    sendingSensLog(sendvalue);
  }
  // and loop forever and ever!
}

void sendingSensLog(String sensLog) {
  const char *Log = sensLog.c_str();
  bluetooth.write(Log);
  delay(50);
  Serial.print("Dato Enviado : ");
  Serial.println(sensLog);
}
