#ifndef __SETTLER_H__
#define __SETTLER_H__

#include "../Base/AbstractUnit.h"

/**
 * @class Settler
 * @brief 定居者单位
 * 
 * 定居者的特点：
 * - 攻击力为0，无法进行战斗
 * - 可以建立城市
 * - 生命值100
 * - 移动力2格/回合
 */
class Settler : public AbstractUnit {
public:

    using AbstractUnit::AbstractUnit;

    Settler() { cost = 80; purchaseCost = 400; }

    /**
     * @brief 获取单位名称
     * @return 定居者名称
     */
    std::string getUnitName() const override {
        return "Settler"; 
    }

    /**
     * @brief 获取单位类型
     * @return 民间单位类型
     */
    UnitType getUnitType() const override { 
        return UnitType::CIVILIAN;
    }

    /**
     * @brief 获取单位精灵图片路径
     * @return 图片文件路径
     */
    std::string getSpritePath() const override { 
        return "units/settler.png"; 
    }

    int getMaintenanceCost() const override {
        return 0;
    }

    bool ismilitary() const override {
        return false;
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
     * @return 0（定居者无法攻击）
     */
    int getBaseAttack() const override { 
        return 0; 
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
     * @return 0（定居者无法攻击）
     */
    int getAttackRange() const override {
        return 0; 
    }

    /**
     * @brief 是否能建立城市
     * @return true（定居者可以建城）
     */
    bool canFoundCity() const override {
        return true; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Settler);
};

#endif // __SETTLER_H__