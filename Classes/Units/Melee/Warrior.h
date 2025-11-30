/*
* 勇士
*/
#ifndef __WARRIOR_H__
#define __WARRIOR_H__

#include "../Base/AbstractUnit.h"

class Warrior : public AbstractUnit {
public:
    // 标准的 create 实现
    static Warrior* create(Hex pos) {
        Warrior* pRet = new Warrior();
        if (pRet && pRet->initUnit(pos)) { // 调用父类的初始化
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    // --- 重写虚函数 (硬编码数值) ---

    virtual std::string getUnitName() override {
        return "勇士";
    }

    virtual int getBaseAttack() override {
        return 20; // 勇士攻击力 20
    }

    virtual int getMaxMoves() override {
        return 2; // 移动力 2
    }

    virtual std::string getSpriteName() override {
        return "units/warrior.png";
    }
};

#endif