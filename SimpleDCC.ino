const int POT_PIN = A0; 
const int SIGNAL_PIN = 10;

int potValue = 0;

void setup() {
  pinMode(SIGNAL_PIN, OUTPUT);
}

void loop() {
  potValue = analogRead(POT_PIN);
  digitalWrite(SIGNAL_PIN, HIGH);
  delay(potValue);
  digitalWrite(SIGNAL_PIN, LOW);
  delay(potValue);
}
