#ifndef __CROSSBOWMAN_H__
#define __CROSSBOWMAN_H__

#include "../Base/AbstractUnit.h"

class Crossbowman : public AbstractUnit {
public:
    static Crossbowman* create(Hex pos) {
        Crossbowman* pRet = new Crossbowman();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    virtual std::string getUnitName() override {
        return "ÂÛ ÷";
    }

    virtual int getBaseAttack() override {
        return 25;
    }

    virtual int getMaxMoves() override {
        return 2;
    }

    virtual std::string getSpriteName() override {
        return "units/crossbowman.png";
    }
};

#endif