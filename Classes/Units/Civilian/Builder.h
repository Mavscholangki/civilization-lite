#ifndef __BUILDER_H__
#define __BUILDER_H__

#include "../Base/AbstractUnit.h"

class Builder : public AbstractUnit {
public:
    static Builder* create(Hex pos) {
        Builder* pRet = new Builder();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    // 重写基本属性
    virtual std::string getUnitName() override {
        return "建造者";
    }

    virtual int getBaseAttack() override {
        return 0;
    }

    virtual int getMaxMoves() override {
        return 2;
    }

    virtual std::string getSpriteName() override {
        return "units/builder.png";
    }

    // 【新增】建造值相关方法
    int getBuildingCharges() const {
        return buildingCharges;
    }

    void useBuildingCharge() {
        if (buildingCharges > 0) {
            buildingCharges--;
            if (buildingCharges == 0) {
                // 建造值耗尽，单位应该被删除
                this->removeFromParent();
            }
        }
    }

    // 【新增】敌方军队在同一位置时调用，转换所有权
    void beMutinied() {
        // 单位叛变，从地图上移除
        this->removeFromParent();
    }

protected:
    int buildingCharges = 3; // 建造值，初始为 3
};

#endif