#ifndef __CANNON_H__
#define __CANNON_H__

#include "../Base/AbstractUnit.h"

class Cannon : public AbstractUnit {
public:
    static Cannon* create(Hex pos) {
        Cannon* pRet = new Cannon();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    virtual std::string getUnitName() override {
        return "»ğÅÚ";
    }

    virtual int getBaseAttack() override {
        return 40;
    }

    virtual int getMaxMoves() override {
        return 1;
    }

    virtual std::string getSpriteName() override {
        return "units/cannon.png";
    }
};

#endif