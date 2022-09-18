#include <Arduino.h>

#include "RefreshBuffer.h"

const int POT_PIN_FREQ = A0; 
const int POT_PIN_DUTY = A1; 
const int SIGNAL_PIN = 10; // timer 1, OC1B

const int DIRECTION_MOTOR_CHANNEL_PIN_A = 12; // not used, but disable to prevent short cut
const int SPEED_MOTOR_CHANNEL_PIN_A = 3;
const int BRAKE_MOTOR_CHANNEL_PIN_A = 9;

int freqPot = 0;  // variable to store the value coming from the sensor
int dutyPot = 0;  // variable to store the value coming from the sensor

// The refresh buffer
RefreshBuffer buffer;

const int SLOT = 0;
const int LOCO = 77;

byte speed = 0;
void setup() {
  Serial.begin(115200);
  disableDirectionOutput();

  pinMode(SIGNAL_PIN, OUTPUT);
  pinMode(SPEED_MOTOR_CHANNEL_PIN_A, OUTPUT);  
  pinMode(BRAKE_MOTOR_CHANNEL_PIN_A, OUTPUT);

  buffer.slot(0).update().withIdleCmd();
  
  configureTimer1();
  powerOn();
}

void loop() {
  freqPot = analogRead(POT_PIN_FREQ);
  dutyPot = analogRead(POT_PIN_DUTY);
  long s = map(freqPot, 15, 1023, 0, 126);
  
  if (s != speed) {
    speed = s;
    Serial.print("speed: "); Serial.println(speed);
    buffer.slot(0).update().withThrottleCmd(LOCO, speed, true, 0);
  }
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


  OCR1A = 20000;
  OCR1B = 10000;

  // enable interrupt OC1B
  bitSet(TIMSK1, OCIE1B);
}

ISR(TIMER1_COMPB_vect) {

  if (buffer.nextBit()) {
    Serial.println("sign: 1");

    // now we need to sent "1"
    // set OCR0A for next cycle to full cycle of DCC ONE bit
    // set OCR0B for next cycle to half cycle of DCC ONE bit
    
    // OCR0A = DCC_ONE_BIT_TOTAL_DURATION_TIMER0;
    // OCR0B = DCC_ONE_BIT_PULSE_DURATION_TIMER0;
    OCR1A = 2000;
    OCR1B = 1000;
    
  } else {
    Serial.println("sign: 0");
    // now we need to sent "0"
    // set OCR0A for next cycle to full cycle of DCC ZERO bit
    // set OCR0B for next cycle to half cycle of DCC ZERO bit
    // OCR0A = DCC_ZERO_BIT_TOTAL_DURATION_TIMER0;
    // OCR0B = DCC_ZERO_BIT_PULSE_DURATION_TIMER0;

    OCR1A = 20000;
    OCR1B = 10000;  
  }

}
