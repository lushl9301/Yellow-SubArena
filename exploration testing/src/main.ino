#include "avr/io.h"    // hmm
#include "avr/interrupt.h" //

#include "SharpA21.h"  // Short-distance
#include "SharpA02.h"  // Long-distance
#include "URM37.h"     // Ultrasonic
#include "MotorShield.h"

#include "JsonGenerator/JsonGenerator.h" //adding Json generator https://github.com/bblanchon/ArduinoJson

using namespace ArduinoJson::Generator;

//digital 0, 1 are not alow to use ==> serial port is using
//digital 2, 3, 4, 7, 8, 9, 10, 13 are occuppied by motor shield & encoder
//A0, A1, 6, 12 need to be kicked out before use

//5, 6, 11, 12

//PWM for reading ==> orange
//TRIG for writing ==> yellow
#define urTRIG A3 //use one to write command

#define urPWM_F 12
#define urPWM_L A2
#define urPWM_R 5

#define motor_L 13  // encoder
#define motor_R 3     // encoder


#define shortIR_LF_in A4
#define shortIR_RF_in A5

#define longIR_L_in A1
#define shortIR_R_in A0

//#define longIR_F_in A4
/**********************/
#define RisingEdgePerGrid 273 // need testing
#define stepToStraighten 3 //every 5 step make a auto adjust
#define speedModeSpeed 300

int RisingEdgePerTurn_200 = 395; //for speed 200 382


volatile int direction;
volatile int delta;
volatile int rightMCtr, leftMCtr;
volatile int speedMode;  //1 for fast(400), 0 for slow(200)

volatile int counter_for_straighten;
volatile int empty_space_R;

int pwd = 1; // current direction
//1 north
//2 east
//3 south
//4 west

int currentX, currentY;
int goalX, goalY;

URM37 u_F, u_L, u_R;
SharpA21 shortIR_LF, shortIR_RF, shortIR_R;
SharpA02 longIR_L;

int u_F_dis, u_R_dis, u_L_dis;
int ir_rf_dis, ir_lf_dis, ir_l_dis, ir_r_dis;

MotorShield md;

void setPinsMode() {
    //analog pins no need
    //so IR sensor no need
    //digital pins are set in URM37.h
    //
    pinMode(motor_R, INPUT);
    pinMode(motor_L, INPUT);
}

void waitForCommand() {
    /**/
    delay(1000);
    return;
    /**/
    while (!Serial.available() || Serial.read() != 'S') {
        ;
    }
    delay(10);
}

void setup() {
    // while (1) {
    //     turn(1);
    //     delay(500);
    // }
    // while (1) {
    //      goAhead(1);
    //      delay(100);
    // }
    // 
    /*drifting
    md.init();
    while (1) {
        md.setSpeeds(330, -100);
        delay(2500);
        md.setSpeeds(200, 200);
        delay(2000);
        md.setSpeeds(-100, 330);
        delay(2500);
        md.setSpeeds(200,200);
        delay(2000);
    }
    */
    Serial.begin(9600);
    setPinsMode();

    //set up motor
    md.init();
    delay(10);
    
    //set up 3 Ultrasonic
    u_F.init(urPWM_F, urTRIG);
    u_L.init(urPWM_L, urTRIG);
    u_R.init(urPWM_R, urTRIG);
    delay(10);
    
    //set up IR sensor
    shortIR_RF.init(shortIR_RF_in);
    shortIR_LF.init(shortIR_LF_in);

    shortIR_R.init(shortIR_R_in);
    longIR_L.init(longIR_L_in);
    delay(10);
}

void loop() {
    // while (1) {
    //     turn(1);
    //     delay(400);
    // }
    
    //waitForCommand();
    // while(1) {
    //     straighten();
    //     turn(-1);
    //     straighten();
    //     turn(-1);
    //     goAhead(6);
    //     turn(1);
    //     turn(1);
    //     goAhead(6);
    //     delay(500);
    // }
    

    // RisingEdgePerTurn_200 /= 2;
    // RisingEdgePerTurn_200 -= 10;
    // int i = 8;
    // while (i--) {
    //     turn(1);
    //     delay(200);
    // }
    // RisingEdgePerTurn_200 = 395 * 4 + 35;
    // i = 3;
    // while (i--) {
    //     turn(1);
    //     delay(400);
    // }

    RisingEdgePerTurn_200 = 395;
    currentX = 10;
    currentY = 7;
    pwd = 1; //north
    findWall();

    pwd = 1;
    counter_for_straighten = stepToStraighten; //every 3 or 5 step do a straighten
    goalX = 1;
    goalY = 1;
    exploration();
    Serial.println("I reach here");
    delay(3000);
    arriving(0);

    turn(1);
    goalX = 20;
    goalY = 15;
    exploration();
    delay(3000);
    arriving(1);

    turn(1);
    goalX = 1;
    goalY = 1;
    exploration();
    delay(3000);
    arriving(0);

    waitForCommand();
    //getFRInstructions();
    currentX = 1;
    currentY = 1;
    bridesheadRevisited();
}

void sensorReading() {
    while ((u_F_dis = u_F.getDis()) == 0) {
        delay(10);
    }
    while ((u_L_dis = u_L.getDis()) == 0) {
        delay(10);
    }
    while ((u_R_dis = u_R.getDis()) == 0) {
        delay(10);
    }

    ir_rf_dis = shortIR_RF.getDis();
    ir_lf_dis = shortIR_LF.getDis() * 0.95;

    ir_r_dis = shortIR_R.getDis();
    ir_l_dis = longIR_L.getDis();

    thinkForAWhile();
}

void thinkForAWhile() {
    //think
    //send and delay
    //cancel delay
    
    JsonObject<10> talk_Json;
    talk_Json["X"] = currentX;
    talk_Json["Y"] = currentY;
    talk_Json["direction"] = pwd;

    talk_Json["U_F"] = u_F_dis;
    talk_Json["U_R"] = u_R_dis;
    talk_Json["U_L"] = u_L_dis;

    talk_Json["short_LF"] = ir_lf_dis;
    talk_Json["short_RF"] = ir_rf_dis;

    talk_Json["short_FR"] = ir_r_dis;

    talk_Json["long_BL"] = ir_l_dis;
    Serial.print(talk_Json);
    Serial.println();

    //root.prettyPrintTo(Serial);

    // Serial.println("==========================");
    // Serial.println("X" + String(currentX));
    // Serial.println("Y" + String(currentY));
    // Serial.println("UF " + String(u_F_dis));
    // Serial.println("IRLF " + String(ir_lf_dis));
    // Serial.println("IRRF " + String(ir_rf_dis));

    // Serial.println("UL " + String(u_L_dis));
    // Serial.println("IRL " + String(ir_l_dis));

    // Serial.println("UR " + String(u_R_dis));
    // Serial.println("IRR " + String(ir_r_dis));
    // Serial.println("___________________________");

}

void exploration() {
    empty_space_R = 0;
    while (abs(goalX - currentX) >= 2 || abs(goalY - currentY) >= 2) {
        //check right
        
        //get all sensor data here.
        sensorReading();

        if (u_R_dis > 12) { //right got space
            ++empty_space_R;
            Serial.println("RRRRRRR");
            if (empty_space_R >= 2) {
                turn(1);
                empty_space_R = -1;
                counter_for_straighten = stepToStraighten;
                continue;
            }
        } else {
            empty_space_R = 0;
            Serial.println("right no space");
            if (--counter_for_straighten == 0) {    //auto fix
                turn(1);    //turn right
                straighten();
                turn(-1);   //turn left
                counter_for_straighten = stepToStraighten;
                sensorReading();
            }
        }

        if (u_F_dis <= 10 || ir_lf_dis > 400 || ir_rf_dis > 400) { //TODO at one edge of arena. strange things happened
            Serial.println("shit in front");
            straighten();
            turn(-1);   //turn left
            counter_for_straighten = stepToStraighten;
            empty_space_R = 0;
            continue;
        }

        //TODO testing
        //default go ahead
        //if (u_F_dis > 12 && ir_rf_dis < 400 && ir_lf_dis < 400) {
            //can go
        goAhead(1);
        //}
    }
}

bool isGoodObstacle() {
    if (ir_lf_dis < 300 || ir_rf_dis < 300) {
        return false;
    }
    return able2Straighten();
}

bool able2Straighten() {
    if (ir_rf_dis > 500 && ir_lf_dis > 500) {
        return true;
    }
    return (abs(ir_rf_dis - ir_lf_dis) < 100);
}

int shortSensorToCM(int ir_rf_dis) {
    //TODO
    //add library here
}

int longSensorToCM(int ir_l_dis) {
    //TODO
    //add library here
}

void findWall() {
    //find the closest obstacle
    //go and auto fix
    //go back
    //find farthest obstacle according to the distance
    //go that way
  
    /*
    HOWTO find closest obstacle
    360 turning. use sensor to see the distance
     */
    Serial.println("finding wall");
    sensorReading();
    int f_dis = min(ir_rf_dis, ir_lf_dis);
    
    int tempDis = f_dis;
    int tempMin = 1;
    
    for (int i = 2; i <= 4; ++i) {
        turn(1);
        sensorReading();
        // if (!isGoodObstacle()) {
        //     continue;
        // }
        f_dis = min(ir_rf_dis, ir_lf_dis);
        if (tempDis > f_dis) {
            //ignore small difference
            tempMin = i;
            tempDis = f_dis;
        }
    }
    turn(1);
    for (int i = 1; i < tempMin; ++i) {
        turn(1);
    }

    sensorReading();
    int farthestX = currentX;
    int farthestY = currentY;
    int farthestDis = 3;
    Serial.println("Found neasest one");

    while (1) {
        if (u_L_dis > farthestDis) {
            farthestDis = u_L_dis;
            farthestX = currentX;
            farthestY = currentY;
        }
        if (u_R_dis > farthestDis) {
            farthestDis = u_R_dis;
            farthestX = -currentX;
            farthestY = -currentY;
        }
        if (u_F_dis <= 10) {
            break;
        }
        goAhead(1);
        sensorReading();
    }

    straighten();
    //auto fix

    /*
    HOWTO find fasest obstacle
    1. go back
    2. find the farthest distance
    take this as the wall?
    3. go to the wall
     */
    turn(1);
    turn(1);

    int grids2goback = abs(abs(farthestX) - currentX) + abs(abs(farthestY) - currentY)  ;
    Serial.print("Go back =======>");
    Serial.println(grids2goback);
    while (grids2goback > 0) {
        goAhead(1);
        grids2goback--;
    }

    if (farthestX < 0) {
        turn(-1); //was on the right. go back. turn left
    } else {        
        turn(1);
    }
    Serial.println("I found the wall");
    //found where is the wall
    
    //go to the wall
    while (1) {
        sensorReading();
        if (u_F_dis <= 10) {
            break;
        }
        goAhead(1);
    }

    straighten();

    turn(-1);
    Serial.println("im with the wall now========================");
    //turn left
    //start stick2TheWall & turn right
    //job done
}

void bridesheadRevisited() {
    //follow instruction
    //
    //turn(1); //right
    //turn(-1); //left
    //goAhead(l);
    getFRInstructions();
}

char getChar() {
    while (!Serial.available());
    return Serial.read();
}

void arriving(int endPoint) {
    //TODO
    //when near the end point
    //do nice stop
    if (endPoint) {
        if (pwd == 3) {
            turn(-1); //face down/3
        }
        straighten();
    
        //now face 3
        turn(1); // turn right
        straighten();
        turn(1);
    } else {
        if (pwd == 1) {
            turn(-1); //face up/1
        }
        straighten();

        //now face up 1
        turn(1); //turn right
        straighten();
        turn(1);
    }
    //face to
    //==========
    //|| o -> 2
    //|| (1, 1)
    //|| 

    //
    //OR
    //
    //
    //         ||
    // (20, 15)||
    //  4 <- o ||
    //===========

}

void getFRInstructions() {
    //get shortest path from RPi
    //then move

    char instrChar;
    int grids;
    while (1) {
        
        while (isDigit(instrChar = getChar())) {
            grids = grids * 10 + instrChar - '0';
        }
        if (grids != 0) {
            goAhead(grids);
            grids = 0;
        }
        if (instrChar == 'R') {
            turn(1);
        } else if (instrChar == 'L') {
            turn(-1);
        } else if (instrChar == 'G') {
            arriving(1);
        }
    }
}


/**************************************************************/
/***************************movement***************************/
/**************************************************************/



void goAhead(int grids) {
    speedMode = 1;
    direction = 1;
    delta = 0;
    rightMCtr = leftMCtr = RisingEdgePerGrid * grids;

    setTimerInterrupt();
    attachInterrupt(1, countRight, RISING);

    md.init();
    md.setSpeeds(speedModeSpeed, speedModeSpeed);
    while (--leftMCtr) {
        while (digitalRead(motor_L));
        while (!digitalRead(motor_L));
        //wait for one cycle
    }

    detachTimerInterrupt();
    detachInterrupt(1);

    md.brakeWithABS();
    
    //update direciton
    switch (pwd) {
        case 1: currentY -=grids;
                break;
        case 2: currentX +=grids;
                break;
        case 3: currentY +=grids;
                break;
        case 4: currentX -=grids;
                break;
        default: break;
    }
    delay(500);
}

void turn(int turnRight) {
    speedMode = 0;
    direction = turnRight;
    delta = 0;
    rightMCtr = leftMCtr = RisingEdgePerTurn_200;

    setTimerInterrupt();
    attachInterrupt(1, countRight, RISING);

    //sp.setSpeedLvls(3, 3);
    md.init();
    md.setSpeeds(-200 * direction, 200 * direction);
    while (--leftMCtr) {
        while (digitalRead(motor_L));
        while (!digitalRead(motor_L));
        //wait for one cycle
    }

    detachTimerInterrupt();
    detachInterrupt(1);

    md.brakeWithABS();
    
    //update direction
    pwd += turnRight;
    if (pwd == 5) {
        pwd = 1;
    } else if (pwd == 0) {
        pwd = 4;
    }
    delay(500);
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
    if (speedMode) {
        md.setM2Speed((speedModeSpeed - delta*5) * direction);
    } else {
        md.setM2Speed((200 - delta*5) * direction);
    }
}



/**************************************************************/
/***************************auto fix***************************/
/**************************************************************/


void straighten() {
    sensorReading();
    if (!isGoodObstacle()) {
        return;
    }
    adjustDistance();
    delay(50);
    adjustDirection();
    delay(50);
}

void adjustDirection() {
    //Ultrasonic go until 5cm
    int speed = 60;
    int l, r;
    for (int i = 0; i < 150; i++) {
        l = shortIR_LF.getDis() * 0.95;
        r = shortIR_RF.getDis();
        delay(3);
        if (r > l) { //TODO WARNING
            md.setSpeeds(-60, 60);
        } else if (r < l) {
            md.setSpeeds(60, -60);
        }
    }
    md.setBrakes(400, 400);
}

void adjustDistance() {
    int speed = 100;
    int l, r, frontDis;
    for (int i = 0; i < 1000; i++) {
        l = shortIR_LF.getDis() * 0.95;
        r = shortIR_RF.getDis();
        delay(7);
        frontDis = max(l, r);
        
        if (frontDis < 480) {
            md.setSpeeds(speed, speed);
        } else if (frontDis > 530) {
            md.setSpeeds(-speed, -speed);
        } else {
            break;
        }
    }
    md.setBrakes(400, 400);
}