const int POT_PIN = A0; 
const int SIGNAL_PIN = 10;

int potValue = 0;  // variable to store the value coming from the sensor

const int DIRECTION_MOTOR_CHANNEL_PIN_A = 12;
const int SPEED_MOTOR_CHANNEL_PIN_A = 3;
const int BRAKE_MOTOR_CHANNEL_PIN_A = 9;

void setup() {
  pinMode(SIGNAL_PIN, OUTPUT);

  disableDirectionOutput();

  pinMode(SPEED_MOTOR_CHANNEL_PIN_A, OUTPUT);  
  pinMode(BRAKE_MOTOR_CHANNEL_PIN_A, OUTPUT);

  powerOn();
}

void loop() {
  potValue = analogRead(POT_PIN);
  digitalWrite(SIGNAL_PIN, HIGH);
  delay(potValue);
  digitalWrite(SIGNAL_PIN, LOW);
  delay(potValue);
}

void powerOn() {
  digitalWrite(SPEED_MOTOR_CHANNEL_PIN_A, HIGH);
  digitalWrite(BRAKE_MOTOR_CHANNEL_PIN_A, LOW); 
}

void disableDirectionOutput() {
  pinMode(DIRECTION_MOTOR_CHANNEL_PIN_A, INPUT);
  digitalWrite(DIRECTION_MOTOR_CHANNEL_PIN_A, LOW);
}
