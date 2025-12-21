#ifndef __CANNON_H__
#define __CANNON_H__

#include "../Base/AbstractUnit.h"

class Cannon : public AbstractUnit {
public:
    std::string getUnitName() const override { 
        return "Cannon"; 
    }
    UnitType getUnitType() const override { 
        return UnitType::SIEGE;
    }
    std::string getSpritePath() const override { 
        return "units/cannon.png"; 
    }
    int getCost() const override {
        return 400;
    }

    int getMaintenanceCost() const override {
        return 4;
    }

    int getProductionCost() const override {
        return 50;
    }

    bool ismilitary() const override {
        return true;
    }

    int getMaxHp() const override {
        return 160; 
    }      // 攻城器械比较脆
    int getBaseAttack() const override { 
        return 90; 
    } // 攻击很高
    int getMaxMoves() const override { 
        return 2;
    }    // 移动很慢
    int getAttackRange() const override { 
        return 3; 
    } // 射程 2 格

    // 攻城单位通常攻击后不能移动 (覆盖基类默认值)
    bool canMoveAfterAttack() const override { 
        return false; 
    }

    CREATE_FUNC(Cannon);
};

#endif