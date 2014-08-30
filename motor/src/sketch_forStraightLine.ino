#include "MotorShield.h"
#include "Speed.h"

#define motor_L 3  // encoder
#define motor_R 5  // encoder

#define RisingEdgePerGrid 400

MotorShield md;
Speed sp;

volatile int direction;
volatile int delta;

void setup() {
    pinMode(motor_R, INPUT);
    pinMode(motor_L, INPUT);
    Serial.begin(115200);
    md.init();
    sp.init(md);
}

void loop() {
    grid = 3;
    rightMCtr = leftMCtr = 400 * grid;

    setTimerInterrupt();
    attachInterrupt(1, countRight, RISING);

    sp.setSpeedLvls(3, 3);

    while (--rightMCtr) {
        while (digitalRead(motor_R));
        while (!digitalRead(motor_R));
        //wait for one cycle
    }

    detachTimerInterrupt();
    detachInterrupt(1);

    md.brakeWithABS();
}

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
  OCR1A = 1562;   // scale = 1024, OCR1A = (xxx / 64 / 1024)

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
  TIMSK1 &= 0; // disable
  sei();
}
ISR(TIMER1_COMPA_vect) {
    md.setM2Speed((350 + delta) * direction);
}