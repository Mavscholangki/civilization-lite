//#ifndef __HEAVY_INFANTRY_H__
//#define __HEAVY_INFANTRY_H__
//
//#include "../Base/AbstractUnit.h"
//
//class HeavyInfantry : public AbstractUnit {
//public:
//    static HeavyInfantry* create(Hex pos) {
//        HeavyInfantry* pRet = new HeavyInfantry();
//        if (pRet && pRet->initUnit(pos)) {
//            pRet->autorelease();
//            return pRet;
//        }
//        delete pRet;
//        return nullptr;
//    }
//
//    virtual std::string getUnitName() override {
//        return "Æ¤¼×ÓÂÊ¿";
//    }
//
//    virtual int getBaseAttack() override {
//        return 30;
//    }
//
//    virtual int getMaxMoves() override {
//        return 2;
//    }
//
//    virtual std::string getSpriteName() override {
//        return "units/heavy_infantry.png";
//    }
//};
//
//#endif