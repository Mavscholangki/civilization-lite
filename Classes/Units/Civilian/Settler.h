#ifndef __SETTLER_H__
#define __SETTLER_H__

// 注意路径：Base 在 Settler 的上一级(Civilian)的上一级(Units)里面
#include "../Base/AbstractUnit.h" 

class Settler : public AbstractUnit {
public:
    static Settler* create(Hex pos) {
        Settler* pRet = new Settler();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet; return nullptr;
    }

    // 重写基本属性
    virtual std::string getUnitName() override { return "Settler"; }
    virtual int getBaseAttack() override { return 0; }
    virtual int getMaxMoves() override { return 2; }
    virtual bool canFoundCity() override { return true; }

    // 【关键修改 3】必须实现这个纯虚函数，否则报错 C2259
    virtual std::string getSpriteName() override { return "units/settler.png"; }

    // 重写初始化
    virtual bool initUnit(Hex startPos) override {
        // 先调用父类初始化
        if (!AbstractUnit::initUnit(startPos)) return false;

        // 修改颜色
        auto draw = dynamic_cast<cocos2d::DrawNode*>(_visual);
        if (draw) {
            draw->clear();
            draw->drawDot(cocos2d::Vec2(0, 0), 12, cocos2d::Color4F::MAGENTA);
        }
        return true;
    }
};

#endif