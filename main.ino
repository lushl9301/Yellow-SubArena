#include "SharpA21.h"  // Short-distance
#include "SharpA02.h"  // Long-distance
#include "URM37.h"     // Ultrasonic
#include "HMC5883L.h"  // Digital Compass
#include "Speed.h"     // Set Speed
#include "Cornering.h" // Turning

#define urPWM_F 2
#define urTRIG_F 3

#define urPWM_L 4
#define urTRIG_L 5

#define urPWM_R 6
#define urTRIG_R 7

#define motor_L A0
#define motor_R A1

/**********************/
#define shortIR_LF_in A2
#define shortIR_LR_in A3
#define longIR_F_in A4
/**********************/
URM37 u_F, u_L, u_R;
SharpA02 longIR_F;
SharpA21 shortIR_LF, shortIR_LR;

MotorShield md;
Speed sp;
Cornering cn;

void setPinsMode() {
    //analog pins no need
    //digital pins are set in URM37.h
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
    shortIR_LR.init(shortIR_LR_in);
    shortIR_LF.init(shortIR_LF_in);
    longIR_F.init(longIR_F_in);
    delay(10);
    while (!Serial.available() || Serial.read() != 'S') {
        delay(10);
    }
}

void loop() {

}