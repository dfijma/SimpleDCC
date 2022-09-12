#include <Arduino.h>

const int POT_PIN = A0; 
const int TIMER_PIN = 10; // timer 1, OC1B

const int DIRECTION_MOTOR_CHANNEL_PIN_A = 12; // not used, but disable to prevent short cut
const int SPEED_MOTOR_CHANNEL_PIN_A = 3;
const int BRAKE_MOTOR_CHANNEL_PIN_A = 9;

int potValue = 0;  // variable to store the value coming from the sensor

void setup() {
  Serial.begin(115200);
  
  disableDirectionOutput();

  pinMode(TIMER_PIN, OUTPUT);
  pinMode(SPEED_MOTOR_CHANNEL_PIN_A, OUTPUT);  
  pinMode(BRAKE_MOTOR_CHANNEL_PIN_A, OUTPUT);

  configureTimer1();
  powerOn();
}

void loop() {
  potValue = analogRead(POT_PIN);
}

void powerOn() {
  digitalWrite(SPEED_MOTOR_CHANNEL_PIN_A, HIGH);
  digitalWrite(BRAKE_MOTOR_CHANNEL_PIN_A, LOW); 
}

void disableDirectionOutput() {
  pinMode(DIRECTION_MOTOR_CHANNEL_PIN_A, INPUT);
  digitalWrite(DIRECTION_MOTOR_CHANNEL_PIN_A, LOW);
}


void configureTimer1() {
  
  // configure timer 1, fast PWM from BOTTOM to TOP == OCR1A
  // fast PWM
  bitSet(TCCR1A, WGM10);
  bitSet(TCCR1A, WGM11);
  bitSet(TCCR1B, WGM12);
  bitSet(TCCR1B,WGM13);

  // set OC1B interrupt pin (pin 19) on Compare Match == OCR1B, clear at BOTTOM (inverting mode)
  bitSet(TCCR1A, COM1B1);
  bitSet(TCCR1A, COM1B0);

  // prescale=1
  //bitClear(TCCR1B,CS12);    
  //bitClear(TCCR1B,CS11);
  //bitSet(TCCR1B,CS10);
  
  // prescale=1024
  bitSet(TCCR1B,CS12);    
  bitClear(TCCR1B,CS11);
  bitSet(TCCR1B,CS10);

 
  setCycle();

  // enable interrupt OC1B
  bitSet(TIMSK1, OCIE1B);
}

ISR(TIMER1_COMPB_vect) {
  setCycle();
}

void setCycle() {
    long full = map(potValue, 0, 1023, 0, 65535); 
    OCR1A = full;
    OCR1B = full / 2;
}
