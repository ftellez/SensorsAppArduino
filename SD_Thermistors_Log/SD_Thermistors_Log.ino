#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include <math.h>

//------Thermistors-------------------
String values = "";
double segundosEjec;
int ThermPin1 = 0,ThermPin2 = 1,ThermPin3 = 2,ThermPin4 = 3,ThermPin5 = 4, ThermPin6 = 5;
int Vo1,Vo2,Vo3,Vo4,Vo5,Vo6;
float R1 = 10000;
float logRA,logRB,logRC,logRD,logRE,logRF,RA,RB,RC,RD,RE,RF,R2A,R2B,R2C,R2D,R2E,R2F,TA,TB,TC,TD,TE,TF;
//float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float c1 = 3.354016e-03, c2 = 2.569850e-04, c3 = 2.620131e-07, c4 = 6.383091e-08;

//-----SD stuff
char nombreArch[] = "LOGGER00.CSV";
const String file = "";
const int CS = 8; //Chip select
File datalogFile; //Datalog File

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  delay(2000);
  //Serial.println("Pulsa cualquier tecla para iniciar.");
  //while (!Serial.available()) {}
  while (!initSD()) { Serial.println("Starting SD again..."); }
  GenerateFilename(nombreArch);
  File openedFile = SD.open(nombreArch, FILE_WRITE); //prueba_0.txt
  openedFile.close();
  Serial.println(nombreArch);
  //while (!writeDataToFile("Prueba,1", nombreArch)) { Serial.println("Sending data to SD again..."); }
  //Serial.println("Datos guardados.");
  file = nombreArch;
  Serial.print("Nombre del archivo: ");
  Serial.println(file);
}

void loop() {
  segundosEjec = hour() * 3600.0 + minute() * 60.0 + second();
  Vo1 = analogRead(ThermPin1);
  Vo2 = analogRead(ThermPin2);
  Vo3 = analogRead(ThermPin3);
  Vo4 = analogRead(ThermPin4);
  Vo5 = analogRead(ThermPin5);
  Vo6 = analogRead(ThermPin6);
  R2A = R1 * (1023.0 / (float)Vo1 - 1.0);
  R2B = R1 * (1023.0 / (float)Vo2 - 1.0);
  R2C = R1 * (1023.0 / (float)Vo3 - 1.0);
  R2D = R1 * (1023.0 / (float)Vo4 - 1.0);
  R2E = R1 * (1023.0 / (float)Vo5 - 1.0);
  R2F = R1 * (1023.0 / (float)Vo6 - 1.0);
  RA = R2A / 10000.0;
  RB = R2B / 10000.0;
  RC = R2C / 10000.0;
  RD = R2D / 10000.0;
  RE = R2E / 10000.0;
  RF = R2F / 10000.0;
  logRA = log(RA);
  logRB = log(RB);
  logRC = log(RC);
  logRD = log(RD);
  logRE = log(RE);
  logRF = log(RF);
  TA = (1.0 / (c1 + c2*logRA + c3*logRA*logRA + c4*logRA*logRA*logRA));
  TB = (1.0 / (c1 + c2*logRB + c3*logRB*logRB + c4*logRB*logRB*logRB));
  TC = (1.0 / (c1 + c2*logRC + c3*logRC*logRC + c4*logRC*logRC*logRC));
  TD = (1.0 / (c1 + c2*logRD + c3*logRD*logRD + c4*logRD*logRD*logRD));
  TE = (1.0 / (c1 + c2*logRE + c3*logRE*logRE + c4*logRE*logRE*logRE));
  TF = (1.0 / (c1 + c2*logRF + c3*logRF*logRF + c4*logRF*logRF*logRF));
  TA = TA - 273.15;
  TB = TB - 273.15;
  TC = TC - 273.15;
  TD = TD - 273.15;
  TE = TE - 273.15;
  TF = TF - 273.15;
  //T = (T * 9.0)/ 5.0 + 32.0; 

  values = (String)segundosEjec + "," + (String)TA + "," + (String)TB + "," + (String)TC + "," + (String)TD + "," + (String)TE + "," + (String)TF + ",";

  Serial.print("Segundos: ");
  Serial.println(segundosEjec);

  Serial.print("Temperature_A: "); 
  Serial.print(TA);
  Serial.println(" C"); 

  Serial.print("Temperature_B: "); 
  Serial.print(TB);
  Serial.println(" C"); 

  Serial.print("Temperature_C: "); 
  Serial.print(TC);
  Serial.println(" C"); 

  Serial.print("Temperature_D: "); 
  Serial.print(TD);
  Serial.println(" C"); 

  Serial.print("Temperature_E: "); 
  Serial.print(TE);
  Serial.println(" C"); 

  Serial.print("Temperature_F: "); 
  Serial.print(TF);
  Serial.println(" C"); 

  Serial.println("");

  Serial.print("Values string: ");
  Serial.println(values);

  while (!writeDataToFile(values, file)) { Serial.println("Guardando datos de nuevo..."); }

  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(500);              // wait for a second
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
  delay(1000);
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

