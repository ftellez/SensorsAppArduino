// LM35 Temperature sensor 

// PINOUT (as seen from flat face)
// LEFT   --> 4 - 20 V
// MIDDLE --> OUT --> Sensor1 (A0), Sensor2 (A1)
// RIGHT  --> GND 


void setup() {
  Serial.begin(9600);
}

void loop() {
  float temp_body = ( 5.0 * analogRead(A0) * 100.0) / 1024; // Sensor 1 
  float temp_hot_plate = ( 5.0 * analogRead(A1) * 100.0) / 1024; // Sensor 2
  Serial.print("Body temperature: ");
  Serial.println(temp_body);
  Serial.print("Hot plate temperature: ");
  Serial.println(temp_hot_plate);
  delay(700);
}
