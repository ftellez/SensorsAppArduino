/*
  DigitalReadSerial
 Reads a digital input on pin 2, prints the result to the serial monitor

 This example code is in the public domain.
 */

// digital pin 2 has a pushbutton attached to it. Give it a name:
int pushButton = 9;
int activeMOSFET = 11;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the pushbutton's pin an input:
  pinMode(pushButton, INPUT);
  pinMode(activeMOSFET,OUTPUT);
  digitalWrite(activeMOSFET, LOW);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  int buttonState = digitalRead(pushButton);
  // print out the state of the button:
//  Serial.println(buttonState);
  if(buttonState){
    digitalWrite(activeMOSFET, HIGH);
    Serial.println("Gate Activo");
  } else {
    digitalWrite(activeMOSFET, LOW);
    Serial.println("Gate Inactivo");
  }
  delay(1);        // delay in between reads for stability
}



