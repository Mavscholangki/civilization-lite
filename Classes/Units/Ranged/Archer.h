#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "../Base/AbstractUnit.h"

class Archer : public AbstractUnit {
public:
    static Archer* create(Hex pos) {
        Archer* pRet = new Archer();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    virtual std::string getUnitName() override {
        return "¹­¼ıÊÖ";
    }

    virtual int getBaseAttack() override {
        return 15;
    }

    virtual int getMaxMoves() override {
        return 2;
    }

    virtual std::string getSpriteName() override {
        return "units/archer.png";
    }
};

#endif