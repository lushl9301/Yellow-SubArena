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
#define urPWM_L 6
#define urPWM_R 5

#define motor_L 13  // encoder
#define motor_R 3     // encoder


#define shortIR_LF_in A4
#define shortIR_RF_in A5

#define longIR_L_in A1
#define shortIR_R_in A0
#define shortIR_L_in A2

//#define longIR_F_in A4
/**********************/

#define RisingEdgePerTurnRight_200 389 //for speed 200 382
#define RisingEdgePerTurnLeft_200 391
#define RisingEdgePerGrid_300 272 // need testing
#define RisingEdgePerGrid_400 290
#define stepToStraighten 3 //every 3 step make a auto adjust 3 OCT
#define speedModeSpeed 300
#define autoAlignmentSpeed 55


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
SharpA21 shortIR_LF, shortIR_RF, shortIR_R, shortIR_L;
SharpA02 longIR_L;

int u_F_dis, u_R_dis, u_L_dis;
int ir_rf_dis, ir_lf_dis, ir_l_dis, ir_r_dis, ir_l2_dis;

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
    /*
    delay(1000);
    return;
    */
    while (!Serial.available() || Serial.read() != 'S') {
        ;
    }
    delay(100);
}

void setup() {

    Serial.begin(9600);
    setPinsMode();

    //set up motor
    md.init();

    //set up 3 Ultrasonic
    u_F.init(urPWM_F, urTRIG);
    u_L.init(urPWM_L, urTRIG);
    u_R.init(urPWM_R, urTRIG);

    //set up IR sensor
    shortIR_RF.init(shortIR_RF_in);
    shortIR_LF.init(shortIR_LF_in);

    shortIR_R.init(shortIR_R_in);
    shortIR_L.init(shortIR_L_in);
    longIR_L.init(longIR_L_in);
}

void dailyTuning() {
    int i;
    
    i = 4;
    delay(1000);
    while (--i) {
         straighten();
         delay(1000);    
    }

    delay(1000);
    i = 12;
    while (--i) {
        turn(1);
        delay(200);
    }

    delay(1000);
    i = 12;
    while (--i) {
        turn(-1);
        delay(200);
    }

}

void sptuning() {

    // turn(-1);
    // straighten();
    // turn(-1);
    // straighten();
    // turn(1);
    // turn(1);
    int grids = 18;
    while (grids-- != 0) {
        goAhead(1);
        delay(1000);
    }
}

void loop() {
    sptuning();
    //dailyTuning();
    //delay(1000);
    
    //explorationFLow();
    while (1) {
        sensorReading();
        delay(150);
    }

    waitForCommand();

    switch (getChar()) {
        case 'E': {
            explorationFLow();
            break;
        }
        case 'P': {
            // turn(1);
            // goalX = 20;
            // goalY = 15;
            // exploration();
            // JsonObject<2> toRPi_t;
            // toRPi_t["type"] = "status";
            // toRPi_t["data"] = "END_PATH";
            // Serial.println(toRPi_t);
            bridesheadRevisited();
            break; 
        }
        case 'R': {
            remote();
            break;
        }
        default:
            // error
            break;
    }
}

void demo() {
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
    // int i = 4;
    // while(i) {
    //     --i;
    //     goAhead(10);
    //     turn(1);
    //     turn(1);
    //     goAhead(10);
    //     delay(500);
    // }
    // delay(5000);
    // Serial.println("Start");
    // while (1) {
    //     sensorReading();
    //     delay(500);
    // }
    // delay(5000);

    // RisingEdgePerTurn_200 /= 2;
    // RisingEdgePerTurn_200 -= 10;
    // i = 8;
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
}

void explorationFLow() {

    // currentX = 2;
    // currentY = 2;
    // sensorReading();
    // JsonObject<2> toRPi1;
    // toRPi1["type"] = "status";
    // toRPi1["data"] = "END_EXP";
    // Serial.println(toRPi1);
    // return;

    currentX = 10;
    currentY = 8;
    pwd = 1; //north
    while (!findWall());

    counter_for_straighten = stepToStraighten; //every 3 or 5 step do a straighten
    goalX = 1;
    goalY = 1;
    exploration();
    Serial.println("I reach START");
    arriving(0);
    currentX = 2;
    currentY = 2;

    turn(1);
    goalX = 20;
    goalY = 15;
    exploration();
    Serial.println("I reach GOAL");
    arriving(1);
    currentX = 19;
    currentY = 14;

    turn(1);
    goalX = 1;
    goalY = 1;
    exploration();
    arriving(0);

    JsonObject<2> toRPi;
    toRPi["type"] = "status";
    toRPi["data"] = "END_EXP";
    Serial.println(toRPi);
}

void sensorReading() {
    //TODO only for testing
    delay(500);
    
    int i = 10;
    while (--i > 0 && ((u_F_dis = u_F.getDis()) == 0 || u_F_dis > 200)) {
        delay(2);
    }

    i = 10;
    while (--i > 0 && ((u_L_dis = u_L.getDis()) == 0 || u_L_dis > 200)) {
        delay(2);
    }
    
    i = 10;
    while (--i > 0 && ((u_R_dis = u_R.getDis()) == 0 || u_R_dis > 200)) {
        delay(2);
    }

    ir_rf_dis = shortIR_RF.getDis();
    ir_lf_dis = shortIR_LF.getDis();

    ir_r_dis = shortIR_R.getDis();
    ir_l_dis = longIR_L.getDis();
    ir_l2_dis = shortIR_L.getDis();

    thinkForAWhile();
}

void thinkForAWhile() {
    //think
    //send and delay
    //cancel delay
    
    JsonObject<11> talk_Json;
    talk_Json["X"] = currentX;
    talk_Json["Y"] = currentY;
    talk_Json["direction"] = pwd;

    talk_Json["U_F"] = u_F_dis;
    talk_Json["U_R"] = u_R_dis;
    talk_Json["U_L"] = u_L_dis;

    // 5 OCT SBS testing
    // talk_Json["short_LF"] = shortSensorToCM2(ir_lf_dis);
    // talk_Json["short_RF"] = shortSensorToCM2(ir_rf_dis);
    // talk_Json["short_FR"] = shortSensorToCM2(ir_r_dis);
    // talk_Json["long_BL"] = longSensorToCM(ir_l_dis);
    
    talk_Json["short_LF"] = shortSensorToCM(ir_lf_dis);
    talk_Json["short_RF"] = shortSensorToCM(ir_rf_dis);

    talk_Json["short_FR"] = shortSensorToCM(ir_r_dis);
    talk_Json["short_FL"] = shortSensorToCM(ir_l2_dis);
    talk_Json["long_BL"] = longSensorToCM(ir_l_dis);
    
    JsonObject<2> toRPi;
    toRPi["type"] = "reading";
    toRPi["data"] = talk_Json;

    Serial.println(toRPi);

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
    int flag_turn_right_just_now = 0;
    
    while (abs(goalX - currentX) >= 3 || abs(goalY - currentY) >= 3) { 
        //get all sensor data here.
        sensorReading();
        if (flag_turn_right_just_now >= 0 && u_R_dis > 12) { //right got space
            if (++empty_space_R >= 2) {
                turn(1);
                flag_turn_right_just_now = 1;
                empty_space_R = -1;
                counter_for_straighten = stepToStraighten;
                continue;
            }
        } else {
            empty_space_R = 0;
            if (--counter_for_straighten == 0) {    //auto fix
                turn(1);    //turn right
                straighten();
                turn(-1);   //turn left
                counter_for_straighten = stepToStraighten;
                sensorReading();
            }
        }

        if (u_F_dis <= 12 || ir_lf_dis > 400 || ir_rf_dis > 400) {
            //Serial.println("something in front");
            straighten();
            turn(-1);   //turn left
            empty_space_R = 0;
            if (flag_turn_right_just_now == 1) {
                empty_space_R = 1;
                flag_turn_right_just_now = -1;
            } else {
                flag_turn_right_just_now = 0;
            }
            counter_for_straighten = stepToStraighten;
            continue;
        }

        //default go ahead
        //if (u_F_dis > 12 && ir_rf_dis < 400 && ir_lf_dis < 400)
        //    then go
        goAhead(1);
        flag_turn_right_just_now = 0;
    }
}

bool isGoodObstacle() {
    return ((shortSensorToCM(ir_rf_dis) < 26) && abs(shortSensorToCM(ir_rf_dis) - shortSensorToCM(ir_lf_dis)) <= 6);

    // if (ir_lf_dis < 280 || ir_rf_dis < 280) {
    //     return false;
    // }
    // return able2Straighten();
}

// bool able2Straighten() {
//     if (ir_rf_dis > 400 && ir_lf_dis > 400) {
//         return true;
//     }
// }

int shortSensorToCM(int ir_dis) {
    int result = 6787 / (ir_dis - 3) - 4;
    return result;
}

// int shortSensorToCM2(int _raw) {
//     float voltFromRaw=map(_raw, 0, 1023, 0, 5000);
//     return (int)(27.728*pow(voltFromRaw/1000, -1.2045));
// }

int longSensorToCM(int ir_dis) {
    int result = 16667 / (ir_dis + 15) - 10;
    return result;
}

// int longSensorToCM2(int _raw) {
//     float voltFromRaw=map(_raw, 0, 1023, 0, 5000);
//     return (int)(61.573*pow(voltFromRaw/1000, -1.1068));
// }

bool isWithWall() {
    if (abs(currentX) <= 2 || currentX >= 19 || abs(currentY) <= 2 || currentY >= 14) {
        return true;
    }
}

bool findWall() {
    //find the furthest obstacle
    //go and auto fix
    //go back
    //find farthest obstacle according to the distance
    //go that way
  
    /*
    HOWTO find furthest obstacle
    360 turning. use sensor to see the distance
     */
    // Serial.println("finding wall");
    sensorReading();
    int f_dis = min(shortSensorToCM(ir_rf_dis), shortSensorToCM(ir_lf_dis));
    if (u_F_dis > f_dis) {
        f_dis = u_F_dis;
    }
    
    int tempDis = f_dis;
    int tempMin = 1;
    
    for (int i = 2; i <= 4; ++i) {
        turn(1);
        sensorReading();
        f_dis = min(shortSensorToCM(ir_rf_dis), shortSensorToCM(ir_lf_dis));
        if (u_F_dis > f_dis) {
            f_dis = u_F_dis;
        }
        if (tempDis < f_dis) {
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
    // Serial.println("Found furthest one");

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
        if (u_F_dis <= 10 || ir_lf_dis > 400 || ir_rf_dis > 400) {
            break;
        }
        goAhead(1);
        sensorReading();
    }

    straighten();
    if (isWithWall()) {
        turn(-1);
        return true;
    }
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
    //Serial.println("Go back =======>");
    Serial.println(grids2goback);
    while (grids2goback > 0) {
        sensorReading();
        goAhead(1);
        grids2goback--;
    }

    if (farthestX < 0) {
        turn(-1); //was on the right. go back. turn left
    } else {
        turn(1);
    }
    //Serial.println("I found the wall");
    //found where is the wall
    
    //go to the wall
    while (1) {
        sensorReading();
        if (u_F_dis <= 10 || ir_lf_dis > 400 || ir_rf_dis > 400) {
            break;
        }
        goAhead(1);
    }

    straighten();

    turn(-1);
    //turn left
    //start stick2TheWall & turn right
    //job done
    if (isWithWall()) {
        return true;
    }
    return false;
}

void bridesheadRevisited() {
    //follow instruction
    //
    //turn(1); //right
    //turn(-1); //left
    //goAhead(l);
    currentX = 1;
    currentY = 1;
    getFRInstructions();
    JsonObject<2> toRPi;
    toRPi["type"] = "status";
    toRPi["data"] = "END_PATH";
    Serial.println(toRPi);
}

void remote() {
    char instrChar;
    int grids;

    while (1) {
        if (isDigit(instrChar = getChar())) {
            grids = instrChar - '0';
            goAhead(grids);
        }
        if (instrChar == 'R') {
            turn(1);
        } else if (instrChar == 'L') {
            turn(-1);
        } else if (instrChar == 'G') {
            break;
        }
    }

    JsonObject<2> toRPi;
    toRPi["type"] = "status";
    toRPi["data"] = "END_RMT";
    Serial.print(toRPi);
}

void arriving(int endPoint) {
    //when near the end point
    //do nice stop
    if (endPoint) {
        if (pwd == 3) {
            straighten();
            turn(-1); //face down/3
        }
        straighten();
    
        //now face 3
        turn(1); // turn right
        straighten();
        turn(1);
    } else {
        if (pwd == 1) {
            straighten();
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

char getChar() {
    while (!Serial.available());
    return Serial.read();
}

void getFRInstructions() {
    //get shortest path from RPi
    //then move

    turn(-1);
    straighten();
    turn(1);

    char instrChar;
    int grids = 0;
    int auto_alignment_counter = 5;
    while (1) {
        
        while (isDigit(instrChar = getChar())) {
            grids = grids * 10 + instrChar - '0';
            Serial.println(grids);
        }
        while (grids-- != 0) {
            if (--auto_alignment_counter == 0) {
                turn(-1);
                straighten();
                turn(1);
                auto_alignment_counter = 4;
            }
            goAhead(1);
        }
        grids = 0;
        if (instrChar == 'R') {
            straighten();
            turn(1);
        } else if (instrChar == 'L') {
            straighten();
            turn(-1);
        } else if (instrChar == 'G') {
            arriving(1);
            return;
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
    rightMCtr = leftMCtr = RisingEdgePerGrid_300 * grids;

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

    brake();
    
    //update direciton
    switch (pwd) {
        case 1: currentY -=grids;

                if (currentY < 2) {
                    currentY = 2;
                }
                break;
        case 2: currentX +=grids;
                if (currentX > 19) {
                    currentX = 19;
                }
                break;
        case 3: currentY +=grids;
                if (currentY > 14) {
                    currentY = 14;
                }
                break;
        case 4: currentX -=grids;
                if (currentX < 2) {
                    currentX = 2;
                }
                break;
        default: break;
    }
    delay(100);
}

void turn(int turnRight) {
    speedMode = 0;
    direction = turnRight;
    delta = 0;
    if (turnRight == 1) {
        rightMCtr = leftMCtr = RisingEdgePerTurnRight_200;
    } else {
        rightMCtr = leftMCtr = RisingEdgePerTurnLeft_200;
    }

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

    brake();
    
    //update direction
    pwd += turnRight;
    if (pwd == 5) {
        pwd = 1;
    } else if (pwd == 0) {
        pwd = 4;
    }
    delay(100);
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
        md.setM2Speed((speedModeSpeed - delta) * direction);
    } else {
        md.setM2Speed((200 - delta) * direction);
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
    delay(100);
}

void adjustDirection() {
    //Ultrasonic go until 5cm
    int speed = 60;
    int l, r;
    for (int i = 0; i < 300; i++) {
        l = shortIR_LF.getDis();
        r = shortIR_RF.getDis();
        delay(1);
        if (r > l) {
            md.setSpeeds(-autoAlignmentSpeed, autoAlignmentSpeed);
        } else if (r < l) {
            md.setSpeeds(autoAlignmentSpeed, -autoAlignmentSpeed);
        }
    }
    brake();
}

void adjustDistance() {
    int speed = 100;
    int l, r, frontDis;
    for (int i = 0; i < 1200; i++) {
        l = shortIR_LF.getDis();
        r = shortIR_RF.getDis();
        delay(7);
        frontDis = max(l, r);
        
        if (frontDis < 500) {
            md.setSpeeds(speed, speed);
        } else if (frontDis > 510) {
            md.setSpeeds(-speed, -speed);
        } else {
            break;
        }
    }
    brake();
}

void brake() {
    // digitalWrite(2, LOW);
    // digitalWrite(7, LOW);
    // digitalWrite(4, LOW);
    // digitalWrite(8, LOW);
    // digitalWrite(9, 255);
    // digitalWrite(10, 255);

    for (int i = 3; i > 0; i--) {
        md.setBrakes(385, 400);
    //     md.setBrakes(300, 300);
    }
}