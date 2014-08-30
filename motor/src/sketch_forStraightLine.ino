#include "MotorShield.h"
#include "Speed.h"
#include "avr/io.h"
#include "avr/interrupt.h"

#define motor_L 13  // encoder
#define motor_R 3  // encoder

#define RisingEdgePerGrid 400
#define RisingEdgePerTurn 400

MotorShield md;

volatile int direction;
volatile int delta;
volatile int rightMCtr, leftMCtr;


void setup() {
    pinMode(motor_R, INPUT);
    pinMode(motor_L, INPUT);
    Serial.begin(9600);
}

void loop() {
  //direction = 1;
  //turn90Degree(3, 1);
  //delay(3000);

  //turn90Degree(3, -1);
  //delay(3000);
  //direction = 1;
  //goStraight(3);
  //delay(3000);
    int grid = 2;
    rightMCtr = leftMCtr = RisingEdgePerGrid * grid;
    delta = 0;
    
    setTimerInterrupt();
    attachInterrupt(1, countRight, RISING);
    md.init();
    md.setSpeeds(300, 300);    
    //sp.setSpeedLvls(3, 3);
    //md.setM1Speed(100);
    while (leftMCtr--) {
        while (digitalRead(motor_L)) {
          ;
        }
        while (!digitalRead(motor_L)) {
          ;
        }
        //wait for one cycle
    }
    detachInterrupt(1);
    detachTimerInterrupt();

    md.setBrakes(400, 400);
    //Serial.println(delta);
    delay(3000);
}
/*
void turn90Degree(int speedLvl, int direction) {
    rightMCtr = leftMCtr = RisingEdgePerTurn;
    delta = 0;
    
    setTimerInterrupt();
    attachInterrupt(1, countRight, RISING);
    md.init();
    md.setSpeeds(300, -300);

    while (leftMCtr--) {
        while (digitalRead(motor_L)) {
          ;
        }
        while (!digitalRead(motor_L)) {
          ;
        }
        //wait for one cycle
    }
    detachInterrupt(1);
    detachTimerInterrupt();

    md.setBrakes(400, 400);
    Serial.println(delta);
    delay(1000);
}
*/

void countRight() {
    --rightMCtr;
    delta = rightMCtr - leftMCtr;
}

void setTimerInterrupt() {
  cli();          // disable global interrupts
  // Timer/Counter Control Registers
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B

  // set compare match register to desired timer count:
  OCR1A = 1062;   // scale = 1024, OCR1A = (xxx / 64 / 1024)

  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler:
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);

  //  enable timer compare interrupt:
  //  Timer/Counter Interrupt Mask Register
  //  Compare A&B interrupts 
  TIMSK1 |= (1 << OCIE1A);
  sei();          // enable global interrupts
}
void detachTimerInterrupt() {
  cli();
  TIMSK1 = 0; // disable
  sei();
}
ISR(TIMER1_COMPA_vect) {
    md.setM2Speed((300 - delta*4) * direction);
}
