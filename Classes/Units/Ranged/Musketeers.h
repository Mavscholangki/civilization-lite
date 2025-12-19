#ifndef __MUSKETEERS_H__
#define __MUSKETEERS_H__

#include "../Base/AbstractUnit.h"

class Musketeers : public AbstractUnit {
public:
    static Musketeers* create(Hex pos) {
        Musketeers* pRet = new Musketeers();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    virtual std::string getUnitName() override {
        return "»ğÇ¹ÊÖ";
    }

    virtual int getBaseAttack() override {
        return 35;
    }

    virtual int getMaxMoves() override {
        return 2;
    }

    virtual std::string getSpriteName() override {
        return "units/musketeers.png";
    }
};

#endif