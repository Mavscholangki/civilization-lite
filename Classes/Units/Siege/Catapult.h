#ifndef __CATAPULT_H__
#define __CATAPULT_H__

#include "../Base/AbstractUnit.h"

class Catapult : public AbstractUnit {
public:
    static Catapult* create(Hex pos) {
        Catapult* pRet = new Catapult();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    virtual std::string getUnitName() override {
        return "Í¶Ê¯Æ÷";
    }

    virtual int getBaseAttack() override {
        return 25;
    }

    virtual int getMaxMoves() override {
        return 1;
    }

    virtual std::string getSpriteName() override {
        return "units/catapult.png";
    }
};

#endif