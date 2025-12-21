#ifndef __WARRIOR_H__
#define __WARRIOR_H__

#include "../Base/AbstractUnit.h"

/**
 * @class Warrior
 * @brief 战士单位（近战单位）
 * 
 * 战士的特点：
 * - 攻击力为20
 * - 生命值100
 * - 移动力2格/回合
 * - 攻击范围为1（仅近战）
 * - 不能建立城市
 */
class Warrior : public AbstractUnit {
public:

    /**
     * @brief 获取单位名称
     * @return 战士名称
     */
    std::string getUnitName() const override { 
        return "Warrior"; 
    }

    /**
     * @brief 获取单位类型
     * @return 近战军事单位类型
     */
    UnitType getUnitType() const override { 
        return UnitType::MELEE; 
    }

    int getCost() const override {
        return 200;
    }

    int getMaintenanceCost() const override {
        return 1;
    }

    int getProductionCost() const override {
        return 30;
    }

    bool ismilitary() const override {
        return true;
    }
    /**
     * @brief 获取单位精灵图片路径
     * @return 图片文件路径
     */
    std::string getSpritePath() const override { 
        return "units/warrior.png"; 
    }

    /**
     * @brief 获取最大生命值
     * @return 100 点生命值
     */
    int getMaxHp() const override { 
        return 100; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 20（战士强大的攻击力）
     */
    int getBaseAttack() const override { 
        return 20; 
    }

    /**
     * @brief 获取最大移动力
     * @return 2 格/回合
     */
    int getMaxMoves() const override { 
        return 2; 
    }

    /**
     * @brief 获取攻击范围
     * @return 1（仅近战可达）
     */
    int getAttackRange() const override { 
        return 1; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Warrior);
};

#endif // __WARRIOR_H__