#ifndef __ABSTRACT_UNIT_H__
#define __ABSTRACT_UNIT_H__

#include "cocos2d.h"
#include "Utils/HexUtils.h"

class AbstractUnit : public cocos2d::Node {
public:
    Hex gridPos;

    virtual std::string getUnitName() = 0;
    virtual int getBaseAttack() = 0;
    virtual int getMaxMoves() = 0;

    // 【注意】这里必须是 virtual，且有默认实现
    virtual bool initUnit(Hex startPos);

    // 【注意】这里必须是 virtual，且有默认实现
    virtual bool canFoundCity() { return false; }

    // 【注意】这里必须是 virtual，且Settler需要重写它
    virtual std::string getSpriteName() { return ""; }

    void moveTo(Hex targetPos, HexLayout* layout);

    // -------------------------------------------------
protected: // 必须是 protected，子类 Settler 才能修改它的颜色
    cocos2d::Node* _visual;
    // -------------------------------------------------
};

#endif