#ifndef _L3DDRIVER_H
#define _L3DDRIVER_H 1

#include <Arduino.h>

#define LATCH_PIN 3
#define CLOCK_PIN 4
#define DATAS_PIN 2

struct coord {
    unsigned char x, y, z;
};

class L3DDriver {
    public:
        L3DDriver() : negativePin(5), positiveValue({0, 0, 0, 0, 0, 0, 0, 0}) {
            for (unsigned char i = 2; i < 13; i++) {
                pinMode(i, OUTPUT);
                digitalWrite(i, LOW);
            }
        }

        /*
         * 实际led灯坐标：
         * x为负极一侧，以电路板右上角往左上角为正方向，以0为首
         * y为正极一侧，以电路板右下角往右上角为正方向，以0为首
         * z为层数，以电路板最底层为第0层
         * 程序/电路坐标：
         * 第0层x：0246，y：0246
         * 第1层x：1357，y：0246
         * 第2层x：0246，y：1357
         * 第3层x：1357，y：1357
         */

        void lightOn(coord c) {
            lightCtrl(1, c);
        }

        void lightOff(coord c) {
            lightCtrl(0, c);
        }

        void lightCtrl(unsigned char action, coord c) {
            c = calcPos(c);
            unsigned char x = c.x, y = c.y, z = c.z;
            if (x > 7 || y > 7 || z > 3 || x < 0 || y < 0 || z < 0) return;

            if (action) 
                positiveValue[x] |= 1 << y;
            else 
                positiveValue[x] &= ~ (1 << y);
        }

        void scan() {
            digitalWrite(negativePin, HIGH);
            if (++negativePin == 13) negativePin = 5;
            if (positiveValue[negativePin - 5] == 0) return;
            digitalWrite(LATCH_PIN, LOW);
            shiftOut(DATAS_PIN, CLOCK_PIN, MSBFIRST, positiveValue[negativePin - 5]);
            digitalWrite(LATCH_PIN, HIGH);
            digitalWrite(negativePin, LOW);

        }

    private:
        unsigned char negativePin;
        unsigned char positiveValue[8];
        
        coord calcPos(coord orig) {
            if (orig.z == 0) {
                return {orig.x * 2, orig.y * 2, 0};
            } else if (orig.z == 1) {
                return {orig.x * 2 + 1, orig.y * 2, 1};
            } else if (orig.z == 2) {
                return {orig.x * 2, orig.y * 2 + 1, 2};
            } else {
                return {orig.x * 2 + 1, orig.y * 2 + 1, 3};
            }
        }
};

#endif