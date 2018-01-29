#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>

// -----Smoothin'---------
const int numReadings = 50;
int count= 0;                   // count to write just every second
int readIndex = 0;              // the index of the current reading 

     //Sensor 1
float readings1[numReadings];      // the readings from the analog input
float total1 = 0.0;                  // the running total
float average1 = 0.0;                // the average

    //Sensor 2
float readings2[numReadings];      // the readings from the analog input
float total2 = 0.0;                  // the running total
float average2 = 0.0;                // the average

    //Sensor 3
float readings3[numReadings];      // the readings from the analog input
float total3 = 0.0;                  // the running total
float average3 = 0.0;                // the average

    //Sensor 4
float readings4[numReadings];      // the readings from the analog input
float total4 = 0.0;                  // the running total
float average4 = 0.0;                // the average


//------Thermistors-------------------
String values = "";
double segundosEjec;
int ThermPin1 = 0,ThermPin2 = 1,ThermPin3 = 2,ThermPin4 = 3 ;
int Vo1,Vo2,Vo3,Vo4;
float R1 = 78700;
float logRA,logRB,logRC,logRD,logRE,logRF,RA,RB,RC,RD,RE,RF,R2A,R2B,R2C,R2D,R2E,R2F,TA,TB,TC,TD,TE,TF;
//float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
//float c1 = 3.354016e-03, c2 = 2.569850e-04, c3 = 2.620131e-07, c4 = 6.383091e-08;

//-----SD stuff
char nombreArch[] = "LOGGER00.CSV";
String file = "";
const int CS = 8; //Chip select
File datalogFile; //Datalog File

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  delay(3000);
  //Serial.println("Pulsa cualquier tecla para iniciar.");
  //while (!Serial.available()) {}
  while (!initSD()) { Serial.println("Starting SD again..."); }
  GenerateFilename(nombreArch);

  File openedFile = SD.open(nombreArch, FILE_WRITE); //prueba_0.txt
  if (openedFile) {
    Serial.println("Se abrio");
    openedFile.close();
  } else {
    Serial.println("Error abriendo");
 
  }
  Serial.println(nombreArch);
  
  //while (!writeDataToFile("Prueba,1", nombreArch)) { Serial.println("Sending data to SD again..."); }
  //Serial.println("Datos guardados.");
  file = nombreArch;
  Serial.print("Nombre del archivo: ");
  Serial.println(file);
  
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings1[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings2[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings3[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings4[thisReading] = 0;
  }
}

void loop() {
 
//
//  total1 = total1 - readings1[readIndex];
//readings2[readIndex] = analogRead(ThermPin1);
//  //Serial.println("Luego de resta");
//  Serial.println(total1);
//  total1 = total1 + readings1[readIndex];
// // Serial.println("Luego de suma");
//  Serial.println(total1);
//  total2 = total2 - readings2[readIndex];
//  readings2[readIndex] = analogRead(ThermPin2);
//  total2 = total2 + readings2[readIndex];
//
//  total3 = total3 - readings3[readIndex];
//  readings3[readIndex] = analogRead(ThermPin3);
//  total3 = total3 + readings3[readIndex];
//
//  total4 = total4 - readings4[readIndex];
//  readings4[readIndex] = analogRead(ThermPin4);
//  total4 = total4 + readings4[readIndex];
//  
//  readIndex = readIndex + 1;
//  
//  if (readIndex >= numReadings) {
//    readIndex = 0;
//  }
//
//  average1 = total1 / numReadings;
//  average2 = total2 / numReadings;
//  average3 = total3 / numReadings;
//  average4 = total4 / numReadings;
  
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(50);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(50);              // wait for a second

  count = count + 1;

 

  count = 0;  
  
  segundosEjec = hour() * 3600.0 + minute() * 60.0 + second();

  float alpha1 = 1.0 - analogRead(ThermPin1)/1023.0;
  float alpha2 = 1.0 - analogRead(ThermPin2)/1023.0;
  float alpha3 = 1.0 - analogRead(ThermPin3)/1023.0;
  float alpha4 = 1.0 - analogRead(ThermPin4)/1023.0;
  
  float TA = (0.5689-alpha1)/0.0104 + 24.0;
  float TB = (0.5689-alpha2)/0.0104 + 24.0;
  float TC = (0.5689-alpha3)/0.0104 + 24.0;
  float TD = (0.5689-alpha4)/0.0104 + 24.0;
  
  values = (String)segundosEjec + "," + (String)TA + "," + (String)TB + "," + (String)TC + "," + (String)TD + "," ;

Serial.print(segundosEjec);
Serial.print(" ");
Serial.print(TA);
Serial.print(" ");
Serial.print(TB);
Serial.print(" ");
Serial.print(TC);
Serial.print(" ");
Serial.println(TD);
//Serial.print(" ");
//Serial.println(TB);

//  Serial.print("Segundos: ");
//  Serial.println(segundosEjec);
//
//  Serial.print("Temperature_A: "); 
//  Serial.print(TA);
//  Serial.println(" C"); 
//
//  Serial.print("Temperature_C: "); 
//  Serial.print(TC);
//  Serial.println(" C"); 
//
//  Serial.print("Temperature_D: "); 
//  Serial.print(TD);
//  Serial.println(" C"); 
//
//  Serial.print("Temperature_B: "); 
//  Serial.print(TB);
//  Serial.println(" C"); 
//
//  Serial.println("");
//
//  Serial.print("Values string: ");
//  Serial.println(values);

  while (!writeDataToFile(values, file)) { Serial.println("Guardando datos de nuevo..."); }

  
}

bool initSD() {
  if (!SD.begin(CS)) {
    Serial.println("Error");  // Check if SD card is present
    return false;
  } else { Serial.println("Init successful"); }
  return true;
}

bool writeDataToFile(String datos, String filename) {
  datalogFile = SD.open(filename, FILE_WRITE); // Add header to file
  if (datalogFile) {
    datalogFile.println(datos);
    datalogFile.close();
  } else {
    Serial.println("Error opening .txt");
    return false;
  }

  return true;
}

void blinkLed() {
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
}

void GenerateFilename(char* fileName){
  // Construct the filename to be the next incrementally indexed filename in the set [00-99].
  for (byte i = 1; i <= 99; i++)
  {
    // check before modifying target filename.
    if (SD.exists(fileName))
    {
      // the filename exists so increment the 2 digit filename index.
      fileName[6] = i/10 + '0';
      fileName[7] = i%10 + '0';
    } else {
      break;  // the filename doesn't exist so break out of the for loop.
    }
  }
}

