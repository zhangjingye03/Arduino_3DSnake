#ifndef _SNAKE3D_H
#define _SNAKE3D_H 1

#include "L3DDriver.h"
#include <Arduino.h>

#define ALLOW_LOOPBOUND 1
#define UP 2
#define DOWN 3
#define LEFT 1
#define RIGHT 0
#define FRONT 4
#define BACK 5


struct SnakeBlock {
    char x, y, z;
    struct SnakeBlock* next;
};

class SnakeDirection {
    public:
        char x, y, z;
        SnakeDirection() : SnakeDirection(random(6)) {}
        SnakeDirection(unsigned char t) {
            switch (t) {
                case RIGHT: x = 1; y = 0; z = 0; break;
                case LEFT: x = -1; y = 0; z = 0; break;
                case UP: x = 0; y = 1; z = 0; break;
                case DOWN: x = 0; y = -1; z = 0; break;
                case FRONT: x = 0; y = 0; z = 1; break;
                case BACK: x = 0; y = 0; z = -1; break;
                default: x = y = 0;
            }
        }
        SnakeDirection(char x, char y, char z)
          : x(x), y(y), z(z) {}
        
        // 防止前进到相反方向
        bool isExclusion(SnakeDirection* d) {
            if (!d) return false;
            if (this->x * d->x != 0 && this->x + d->x == 0
             || this->y * d->y != 0 && this->y + d->y == 0
             || this->z * d->z != 0 && this->z + d->z == 0) return true;
            return false;
        }
};

class Snake3D {
    public:
        bool isOver = false;
        Snake3D(L3DDriver* ldr) : Snake3D(4, ldr) {}
        Snake3D(unsigned char size, L3DDriver* ldr)
          : size(size), currentLen(1), score(0), ldr(ldr) {
            SnakeBlock *l, *n;
            l = tail = new SnakeBlock;
            l->x = random(size);
            l->y = random(size);
            l->z = random(size);
            head = tail;
            head->next = NULL;
            lastDir = NULL;
            isOver = 0;
            hasFood = 0;
        }

        char move(SnakeDirection* dir) {
            if (!dir) return -1;
            if (isOver) return 0;
            if (dir->isExclusion(lastDir)) {
                delete dir;
                return 0;
            }
            delete lastDir;
            lastDir = dir;
            
            // 10%几率产生食物
            if (!hasFood && random(10) == 5) {
                foodX = random(size);
                foodY = random(size);
                foodZ = random(size);
                hasFood = 1;
                ldr->lightOn({foodX, foodY, foodZ});
                
                Serial.print("Food @ (");
                Serial.print(foodX, DEC);
                Serial.print(", ");
                Serial.print(foodY, DEC);
                Serial.print(", ");
                Serial.print(foodZ, DEC);
                Serial.println(")");
            }
            
            // 尝试移动蛇头
            char hx = head->x + dir->x, hy = head->y + dir->y, hz = head->z + dir->z;
            // 判断蛇头是否越界
            if (hx < 0 || hx >= size || hy < 0 
             || hy >= size || hz < 0 || hz >= size) {
                if (ALLOW_LOOPBOUND) {
                    // 允许越界时
                    // 将蛇头移动到对面边界，再判断是否撞身
                    if (hx < 0) hx = size - 1;
                    else if (hx >= size) hx = 0;
                    else if (hy < 0) hy = size - 1;
                    else if (hy >= size) hy = 0;
                    else if (hz < 0) hz = size - 1;
                    else hz = 0;
                } else {
                    // 不允许越界时，game over
                    gameOver();
                    return 0;
                }
            }
            ldr->lightOn({hx, hy, hz});

            // 蛇头碰到食物，填充老的方块
            if (hasFood) {
                if (hx == foodX && hy == foodY && hz == foodZ) {
                    eatFood();
                    return 1;
                }
            } 

            if (head == tail) ldr->lightOff({head->x, head->y, head->z});
            
            

            // 移动蛇身
            SnakeBlock* t = tail;
            while (t->next) {
                Serial.print("(");
                Serial.print(t->x, DEC);
                Serial.print(", ");
                Serial.print(t->y, DEC);
                Serial.print(", ");
                Serial.print(t->z, DEC);
                Serial.print(") ==> ");
                
                if (t == tail) ldr->lightOff({t->x, t->y, t->z});
                t->x = t->next->x;
                t->y = t->next->y;
                t->z = t->next->z;
                Serial.print("(");
                Serial.print(t->x, DEC);
                Serial.print(", ");
                Serial.print(t->y, DEC);
                Serial.print(", ");
                Serial.print(t->z, DEC);
                Serial.println(")");
                ldr->lightOn({t->x, t->y, t->z});

                // 判断蛇头是否撞到蛇身
                if (hx == t->x && hy == t->y && hz == t->z) {
                    gameOver();
                    return 0;
                }

                t = t->next;
            }
            head->x = hx; head->y = hy; head->z = hz;
            return 1;
        }

        void eatFood() {
            // 把食物所在地变成蛇头
            SnakeBlock* n = new SnakeBlock;
            n->x = foodX;
            n->y = foodY;
            n->z = foodZ;
            n->next = NULL;
            head->next = n;
            head = n;
            currentLen++;
            hasFood = 0;
                   
            Serial.print("Food (");
            Serial.print(n->x, DEC);
            Serial.print(", ");
            Serial.print(n->y, DEC);
            Serial.print(", ");
            Serial.print(n->z, DEC);
            Serial.println(") has been eaten.");
        }

        void gameOver() {
            isOver = 1;
            Serial.println("Game over");
        }
    private:
        unsigned char size, currentLen, hasFood, foodX, foodY, foodZ;
        unsigned int score;
        SnakeBlock *tail, *head;
        SnakeDirection* lastDir;
        L3DDriver* ldr;
};

#endif