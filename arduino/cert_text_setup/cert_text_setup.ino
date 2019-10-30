#include <stdio.h>
#include <DS1302.h>

#define ST_WORK 0
#define ST_SETUP 1

DS1302 rtc(2, 4, 3);

byte bitPins[6] = {13,12,11,10,9,8};
byte cathPins[3] = {7,6,5};
byte btnPins[2] = {A0,A1};
byte buzzPin = A2;
byte cath = 0;
byte bits[3] = {0,0,0};
byte setup_bit[3] = {1,1,1};
byte setup_cath = 0;
byte setup_blink = 0;
byte btn_lock = 0;

byte state = ST_WORK;

unsigned long t, cath_next, blink_next, setup_next, beep_next, lock_next;
byte cath_to = 1;
unsigned int blink_to = 500;
unsigned int beep_to = 100;
unsigned int lock_to = 300;
unsigned int setup_to = 5000;

void setup() {
    //rtc.writeProtect(false);
    //rtc.halt(false);

    //Time t(2019, 1, 1, 0, 0, 0, 3);
    //rtc.time(t);
  
    pinMode(btnPins[0],INPUT);
    pinMode(btnPins[1],INPUT);
  
    for(byte i=0; i<6; i++){
        pinMode(bitPins[i],OUTPUT);
    }
    for(byte i=0; i<3; i++){
        pinMode(cathPins[i],OUTPUT);
    }
}

void btnLock(unsigned long t){
    lock_next = t + lock_to;
    btn_lock = 1;
}
void btnUnlock(){
    btn_lock = 0;
}
void beep(unsigned long t){
    beep_next = t + beep_to;
    tone(buzzPin, 247, 50);
}
void silence(){
    noTone(buzzPin);
}

void clearBits(){
    // turn off all cathodes
    for( byte i=0; i<3; i++ ){
        digitalWrite(cathPins[i],LOW);
    }
}
void setBits( byte h, byte m, byte s ){
    bits[0] = h;
    bits[1] = m;
    bits[2] = s;
}

void drawBits( byte cath ){
    // turn off all cathodes
    for( byte i=0; i<3; i++ ){
        digitalWrite(cathPins[i],LOW);
    }

    digitalWrite(cathPins[cath],HIGH);
    for( byte i=0; i<6; i++ ){
        if( bits[cath] & 1<<i ){
            digitalWrite(bitPins[i], HIGH);
        } else {
            digitalWrite(bitPins[i], LOW);
        }
    }
}

void setTime( byte h, byte m, byte s ){
  //rtc.writeProtect(false);
  //rtc.halt(false);
  Time t(2019, 1, 1, h, m, s, 3);
  rtc.time(t);
  //rtc.writeProtect(true);
}

void loop() {
    t = millis();
    if( !btn_lock && state == ST_WORK &&  digitalRead(btnPins[0]) && digitalRead(btnPins[1]) ){
        state = ST_SETUP;
        Time rt = rtc.time();
        setup_bit[0] = rt.hr;
        setup_bit[1] = rt.min;
        setup_bit[2] = rt.sec;
        setup_next = t + setup_to;
        setup_blink = 0;
        btnLock(t);
        beep(t);
    }
    if( !btn_lock && state == ST_SETUP && digitalRead( btnPins[0] )){
        setup_cath += 1;
        if( setup_cath == 3 )
            setup_cath = 0;
        setup_next = t + setup_to;
        setup_blink = 0;
        btnLock(t);
        beep(t);
    }
    if( !btn_lock && state == ST_SETUP && digitalRead( btnPins[1] )){
        setup_bit[setup_cath] += 1;
        byte limit = 60;
        if( setup_cath == 0 )
            limit = 24;
        if( setup_bit[setup_cath] == limit )
            setup_bit[setup_cath] = 0;
        setup_next = t + setup_to;
        setup_blink = 0;
        btnLock(t);
        beep(t);
    }
    if( t > cath_next ){
        cath_next = t + cath_to;
        if(state == ST_WORK){
            Time rt = rtc.time();
            setBits( rt.hr, rt.min, rt.sec );
        }
        else if (state == ST_SETUP){
            setBits( setup_bit[0], setup_bit[1], setup_bit[2] );
            if(setup_blink) // blink current cathode
                bits[setup_cath] = 0;
        }
        drawBits(cath);
        cath++;
        if( cath == 3 ){
            cath = 0;
        }
    }
    if( state == ST_SETUP && t > blink_next ){
        blink_next = t + blink_to;
        setup_blink = !setup_blink;
    }
    if( state == ST_SETUP && t > setup_next ){
        state = ST_WORK;
        setTime(setup_bit[0], setup_bit[1], setup_bit[2]);
        setup_cath = 0;
    }
    if( state == ST_SETUP && t > beep_next ){
        silence();
    }
    if( state == ST_SETUP && t > lock_next ){
        btnUnlock();
    }
}
