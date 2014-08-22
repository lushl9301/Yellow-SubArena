#include "Speed.h"

Speed::Speed() {
    ;
}

Speed::Speed(MotorShield md) {
    _md = md;
}

void Speed::init(MotorShield md) {
    _md = md;
}

void Speed::setM1SpeedLvl(int lvl) {
    switch (lvl) {
        case 1:
            _md.setM1Speed(65);
            break;
        case 2:
            _md.setM1Speed(130);
            break;
        case 3:
            _md.setM1Speed(195);
            break;
        case 4:
            _md.setM1Speed(255);
            break;
        case -1:
            _md.setM1Speed(-65);
            break;
        case -2:
            _md.setM1Speed(-130);
            break;
        case -3:
            _md.setM1Speed(-195);
            break;
        case -4:
            _md.setM1Speed(-255);
            break;
        default:
            _md.setM1Speed(40);
            break;
    }
}

void Speed::setM2SpeedLvl(int lvl) {
    switch (lvl) {
        case 1:
            _md.setM2Speed(65);
            break;
        case 2:
            _md.setM2Speed(130);
            break;
        case 3:
            _md.setM2Speed(195);
            break;
        case 4:
            _md.setM2Speed(255);
            break;
        case -1:
            _md.setM2Speed(-65);
            break;
        case -2:
            _md.setM2Speed(-130);
            break;
        case -3:
            _md.setM2Speed(-195);
            break;
        case -4:
            _md.setM2Speed(-255);
            break;
        default:
            _md.setM2Speed(40);
            break;
    }
}

void Speed::setSpeedLvls(int m1Lvl, int m2Lvl) {
    setM1SpeedLvl(m1Lvl);
    setM2SpeedLvl(m2Lvl);
}

void Speed::setM1BrakeLvls(int lvl) {
    ;
}

void Speed::setM2BrakeLvls(int lvl) {
    ;
}

void Speed::setBrakeLvls(int m1Lvl, int m2Lvl) {
    ;    
}
