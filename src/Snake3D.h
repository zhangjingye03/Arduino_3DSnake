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
            if (isOver) return 0;
            if (dir->isExclusion(lastDir)) {
                delete dir;
                return 0;
            }
            delete lastDir;
            lastDir = dir;
            
            // 25%几率产生食物
            if (!hasFood && random(4) == 0) {
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
            
            // 蛇头碰到食物
            // 这儿有BUG，没有处理越界后的蛇头碰到食物
            if (hasFood && head->x + dir->x == foodX && head->y + dir->y == foodY && head->z + dir->z == foodZ) {
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
                return 1;
            }
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
                t = t->next;
            }
            // 尝试移动蛇头
            if (t == tail) ldr->lightOff({t->x, t->y, t->z});
            t->x += dir->x;
            t->y += dir->y;
            t->z += dir->z;
            // 判断蛇头是否越界
            if (t->x < 0 || t->x >= size || t->y < 0 || t->y >= size || t->z < 0 || t->z >= size) {
                if (ALLOW_LOOPBOUND) {
                    // 允许越界时
                    // 将蛇头移动到对面边界，再判断是否撞身
                    if (t->x < 0) t->x = size - 1;
                    else if (t->x >= size) t->x = 0;
                    else if (t->y < 0) t->y = size - 1;
                    else if (t->y >= size) t->y = 0;
                    else if (t->z < 0) t->z = size - 1;
                    else t->z = 0;
                } else {
                    // 不允许越界时，game over
                    gameOver();
                    return 0;
                }
            }
            ldr->lightOn({t->x, t->y, t->z});
            // 判断蛇头是否撞到蛇身
            t = tail;
            while (t->next) {
                if (head->x == t->x && head->y == t->y && head->z == t->z) {
                    gameOver();
                    return 0;
                }
                t = t->next;
            }
            return 1;
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