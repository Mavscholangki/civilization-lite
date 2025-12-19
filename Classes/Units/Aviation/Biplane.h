#ifndef __BIPLANE_H__
#define __BIPLANE_H__

#include "../Base/AbstractUnit.h"

class Biplane : public AbstractUnit {
public:
    static Biplane* create(Hex pos) {
        Biplane* pRet = new Biplane();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    virtual std::string getUnitName() override {
        return "ÂÝÐý½°·É»ú";
    }

    virtual int getBaseAttack() override {
        return 20;
    }

    virtual int getMaxMoves() override {
        return 3;
    }

    virtual bool canFly() override {
        return true;
    }

    virtual std::string getSpriteName() override {
        return "units/biplane.png";
    }
};

#endif