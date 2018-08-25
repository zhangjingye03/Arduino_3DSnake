#include "L3DDriver.h"
#include "Snake3D.h"
#include <Arduino.h>

L3DDriver *l;
unsigned int count = 0;
char tx = -1, ty = 0, tz = 0, tf = 0;
Snake3D* s;
char started = 0;

void setup() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    randomSeed(analogRead(0));
    l = new L3DDriver;
}


void loop() {
    // 扫描led
    if (count % 25 == 0) {
        l->scan();
    }

    
    // 开机动画 & 自检
    if (count % 300 == 0 && tf < 2) {
        if (++tx == 4) {
            tx = 0; 
            if (++ty == 4) {
                ty = 0; 
                if (++tz == 4) {
                    tz = 0;
                    tf++;
                }
            }
        }
        if (tf == 0) l->lightOn({tx, ty, tz});
        else if (tf == 1) l->lightOff({tx, ty, tz});
        else {
            delay(1000);
            digitalWrite(13, HIGH);
            started = 1;
            s = new Snake3D(l);
        }
    }

    // 蛇皮走位
    if (count % 10000 == 0 && started) {
        s->move(new SnakeDirection);
        if (s->isOver) {
            delete s;
            started = false;
        }
    }

    if (++count == 60000) count = 0;
}