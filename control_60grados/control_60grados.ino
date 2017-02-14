
int gate_MOSFET = 9; // Gate pin on MOSFET

void setup() {
  Serial.begin(9600);
  pinMode(gate_MOSFET, OUTPUT);
  digitalWrite(gate_MOSFET, HIGH);
}

void loop() {

  // Primera lectura
  int reading_1 = analogRead(A0);
  float voltage_1 = reading_1 * 5.0 / 1023;
  float temperatureC_1 = (voltage_1 - 0.5) * 100;
 
  bool change = temperatureC_1 <= 65;

  while (change) {
    digitalWrite(gate_MOSFET, HIGH);
   
    // Sensor temperatura
    reading_1 = analogRead(A0);
    voltage_1 = reading_1 * 5.0 / 1023;
    temperatureC_1 = (voltage_1 - 0.5) * 100;
    change = temperatureC_1 < 62;  // voltaje al que se llega a 60*
    
    Serial.print(voltage_1);
    Serial.print("V   ");
    Serial.print(temperatureC_1);
    Serial.print("*C \t");
    Serial.println(change);
    delay(1000); 
  }

  while (!(change)) {
    digitalWrite(gate_MOSFET, LOW);
    
    // Sensor temperatura
    reading_1 = analogRead(A0);
    voltage_1 = reading_1 * 5.0 / 1023;
    temperatureC_1 = (voltage_1 - 0.5) * 100;
    change = temperatureC_1 < 58;  // voltaje al que se llega a 60*C
    
    Serial.print(voltage_1);
    Serial.print("V   ");
    Serial.print(temperatureC_1);
    Serial.print("*C \t");
    Serial.println(change);
    delay(1000);
  }
}


