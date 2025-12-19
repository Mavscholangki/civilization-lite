#ifndef __JET_FIGHTER_H__
#define __JET_FIGHTER_H__

#include "../Base/AbstractUnit.h"

class JetFighter : public AbstractUnit {
public:
    static JetFighter* create(Hex pos) {
        JetFighter* pRet = new JetFighter();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    virtual std::string getUnitName() override {
        return "ÅçÆøÊ½·É»ú";
    }

    virtual int getBaseAttack() override {
        return 50;
    }

    virtual int getMaxMoves() override {
        return 4;
    }

    virtual bool canFly() override {
        return true;
    }

    virtual std::string getSpriteName() override {
        return "units/jet_fighter.png";
    }
};

#endif