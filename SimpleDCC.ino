const int POT_PIN = A0; 
const int DCC_SIGNAL_PIN_MAIN = 5;

int potValue = 0;  // variable to store the value coming from the sensor

void setup() {
  pinMode(DCC_SIGNAL_PIN_MAIN, OUTPUT);
}

void loop() {
  potValue = analogRead(POT_PIN);
  digitalWrite(DCC_SIGNAL_PIN_MAIN, HIGH);
  delay(potValue);
  digitalWrite(DCC_SIGNAL_PIN_MAIN, LOW);
  delay(potValue);
}
