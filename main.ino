#include "SharpA21.h"  // Short-distance
#include "SharpA02.h"  // Long-distance
#include "URM37.h"     // Ultrasonic
//#include "HMC5883L.h"  // Digital Compass
#include "avr/io.h"    // hmm
#include "avr/interrupt.h" //

//PWM for reading ==> orange
//TRIG for writing ==> yellow  
#define urPWM_F 15
#define urTRIG_F 14

#define urPWM_L 1
#define urTRIG_L 0

#define urPWM_R A2
#define urTRIG_R A3

#define motor_L 13  // encoder
#define motor_R 3   // encoder

/**********************/
#define shortIR_LF_in A4
#define shortIR_RF_in A5
//#define longIR_F_in A4
/**********************/
#define RisingEdgePerGrid 400 // need testing
#define RisingEdgePerTurn_200 367 //for speed 200
#define stepToStraighten 5 //every 5 step make a auto adjust


volatile int currentDirection;
volatile int delta;
volatile int rightMCtr, leftMCtr;
volatile long timer;


URM37 u_F, u_L, u_R;
SharpA02 longIR_F;
SharpA21 shortIR_LF, shortIR_RF;

MotorShield md;

void setPinsMode() {
    //analog pins no need
    //so IR sensor no need
    //digital pins are set in URM37.h
    //
    //TODO ===> set moto sensor in
    pinMode(motor_R, INPUT);
    pinMode(motor_L, INPUT);
}

void waitForCommand() {
    while (!Serial.available() || Serial.read() != 'S') {
    }
    delay(10);
}

void setup() {
    
    Serial.begin(9600);
    setPinsMode(); // not sure yet

    //set up motor
    md.init();
    sp.init(md);
    cn.init(md);
    delay(10);
    
    //set up 3 Ultrasonic
    u_F.init(urPWM_F, urTRIG_F);
    u_L.init(urPWM_L, urTRIG_L);
    u_R.init(urPWM_R, urTRIG_R);
    delay(10);
    
    //set up IR sensor
    shortIR_RF.init(shortIR_RF_in);
    shortIR_LF.init(shortIR_LF_in);
    //longIR_F.init(longIR_F_in);
    delay(10);
}

void loop() {
    waitForCommand();
    goalX = 1;
    goalY = 1;
    currentX = 10;
    currentY = 7;
    pwd = 1;
    int counter_for_straighten = stepToStraighten; //every 3 or 5 step do a straighten
    exploration();
    waitForCommand();
    getFRInstructions();
    currentX = 1;
    currentY = 1;
    bridesheadRevisited();
}
void exploration() {

    findWall();
    /*
    find my way to start point
    then make a travel
     */
    goalX = 1;
    goalY = 1;
    while ()

    empty_space_R = 0;
    while (abs(goalX - currentX) >= 2 && abs(goalY - currentY) >= 2) {
        //check right
        
        //get all sensor data here.

        u_F_dis = u_F.getDis();
        u_L_dis = u_L.getDis();
        u_R_dis = u_R.getDis();
        thinkForAWhile();

        if (u_R_dis > 12) { //right got space
            ++empty_space_R;
            if (empty_space_R >= 2) {
                turn(1);
                continue;
            }
        } else {
            empty_space_R = 0;

            if (--counter_for_straighten == 0) {    //auto fix
                turn(1);    //turn right
                straighten();
                turn(-1);   //turn left
                counter_for_straighten = stepToStraighten;
            }
        }

        if (u_F_dis <= 6) {
            straighten();
            turn(-1);   //turn left
            continue;
        }

        //default go ahead
        goAhead(1);
    }
}

void findWall() {
    //find the closest obstacle
    //go and auto fix
    //go back
    //find farthest obstacle according to the distance
    //go that way
    //
    
    /*
    HOWTO find closest obstacle
    360 turning. use sensor to see the distance
     */
    
    /*
    HOWTO find fasest obstacle
    go back
    find the farthest distance
    take this as the wall?
     */
}

void bridesheadRevisited() {
    //follow instruction
    //
    //turn(1);
    //turn(-1);
    //goAhead(l);
    getFRInstructions();
}

void thinkForAWhile() {
    //TODO
    //see and think
    //send and delay
}

boolean turn(int direction) {
    //TODO
    //Check if can rotate
    //rotate
    //update current direction
    
}

void getFRInstructions() {
    //TODO
    //get shortest path from RPi
    //or Fast Run?
}

void straighten() {
    adjustDistance();
    adjustDirection();
}

void adjustDirection() {
    //Ultrasonic go until 5cm
    for (int i = 0; i < 1000; i++) {
        if (shortIR_RF.getDis() > shortIR_LF.getDis()) {
            md.setSpeeds(-60, 60);
        } else if (shortIR_RF.getDis() < shortIR_LF.getDis()) {
            md.setSpeeds(60, -60);
        }
    }
    md.setBrakes(400, 400);
}

void adjustDistance() {
    int frontDis = max(shortIR_LF.getDis(), shortIR_RF.getDis());
    for (int i = 0; i < 1000; i++) {
        if (frontDis < 450) {
            md.setSpeeds(100, 100);
        } else if (frontDis > 500) {
            md.setSpeeds(-100, -100);
        } else {
            break;
        }
    }
    md.setBrakes(400, 400);
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
  TIMSK1 = 0; // disable
  sei();
}

ISR(TIMER1_COMPA_vect) {
  if (speedMode == 0)
    md.setM1Speed((200 + leftCompensate) * neg);
  else
    md.setM1Speed((350 + leftCompensate) * neg);
}