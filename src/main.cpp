#include "L3DDriver.h"
#include "Snake3D.h"
#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#define APDS9960_INT 2

L3DDriver *l;
unsigned int count = 0;
char tx = -1, ty = 0, tz = 0, tf = 0;
Snake3D* s;
char started = 0;
SparkFun_APDS9960 apds = SparkFun_APDS9960();
char isr_flag = 0;
int gesture = -1;

void interruptRoutine() {
    isr_flag = 1;
}

void handleGesture() {
    if (apds.isGestureAvailable()) {
        gesture = apds.readGesture();
        switch (gesture) {
            case DIR_UP:
                Serial.println("UP");
                break;
            case DIR_DOWN:
                Serial.println("DOWN");
                break;
            case DIR_LEFT:
                Serial.println("LEFT");
                break;
            case DIR_RIGHT:
                Serial.println("RIGHT");
                break;
            case DIR_NEAR:
                Serial.println("NEAR");
                break;
            case DIR_FAR:
                Serial.println("FAR");
                break;
            default:
                Serial.println("NONE");
        }
    }
}

void setup() {
    Serial.begin(9600);

    // 初始化APDS9960，注册中断引脚
    pinMode(APDS9960_INT, INPUT);
    
    if (apds.init()) {
        Serial.println(F("APDS-9960 initialization complete."));
    } else {
        Serial.println(F("Cannot initialize APDS-9960!"));
    }
    if (apds.enableGestureSensor(true)) {
        Serial.println(F("Gesture sensor is now running."));
    } else {
        Serial.println(F("Cannot initialize gesture sensor!"));
    }

    randomSeed(analogRead(0));
    l = new L3DDriver;
    Serial.println("here");
}


void loop() {
    // 扫描led
    if (count % 10 == 0) {
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
            started = 1;
            s = new Snake3D(l);
            attachInterrupt(digitalPinToInterrupt(APDS9960_INT), interruptRoutine, FALLING);
        }
    }

    // 蛇皮走位
    if (count % 15000 == 0 && started) {
        SnakeDirection* sd = NULL;
        if (gesture == DIR_UP || gesture < 0) sd = new SnakeDirection(FRONT);
        else if (gesture == DIR_DOWN) sd = new SnakeDirection(BACK);
        else if (gesture == DIR_LEFT) sd = new SnakeDirection(RIGHT);
        else if (gesture == DIR_RIGHT) sd = new SnakeDirection(LEFT);
        else if (gesture == DIR_FAR) sd = new SnakeDirection(DOWN);
        else if (gesture == DIR_NEAR) sd = new SnakeDirection(UP);
        s->move(sd);
        if (s->isOver) {
            delete s;
            started = false;
        }
    }

    // 判断手势
    if (count % 50 == 0 && isr_flag == 1) {
        detachInterrupt(digitalPinToInterrupt(APDS9960_INT));
        handleGesture();
        isr_flag = 0;
        attachInterrupt(digitalPinToInterrupt(APDS9960_INT), interruptRoutine, FALLING);
    }

    if (++count == 60000) count = 0;
}