#ifndef URM37_h
#define URM37_h

class URM37 {
public:
    URM37();
    
    void init(int URPWM, int URTRIG);
    
    int getDis();
    
private:
    int _URPWM;
    int _URTRIG;
    uint8_t EnPwmCmd[4]={0x44,0x02,0xbb,0x01};
};

#endif
