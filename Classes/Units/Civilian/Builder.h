#ifndef __BUILDER_H__
#define __BUILDER_H__

#include "../Base/AbstractUnit.h"

/**
 * @file Builder.h
 * @brief 建筑工人单位类
 * 
 * 建筑工人是一种民间工作单位
 * 专门用于改良地块和建造基础设施
 */

/**
 * @class Builder
 * @brief 建筑工人单位
 * 
 * 建筑工人的特点：
 * - 攻击力为0，无法进行战斗
 * - 可以改良地块（农田、矿山、道路等）
 * - 生命值100
 * - 移动力2格/回合
 * - 不能建立城市（定居者才能建城）
 * 
 * 与定居者的区别：
 * - Settler（定居者）：建立城市，一次性消耗
 * - Builder（建筑工人）：改良地块，可重复使用
 */
class Builder : public AbstractUnit {
public:

    Builder() { cost = 50; purchaseCost = 200; }

    /**
     * @brief 获取单位名称
     * @return 建筑工人名称
     */
    std::string getUnitName() const override { 
        return "Builder"; 
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
        return "units/builder.png"; 
    }

    int getCost() const override {
        return 200;
    }

    int getMaintenanceCost() const override {
        return 0;
	}

    int getProductionCost() const override {
        return 50;
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
     * @return 0（建筑工人无战斗力）
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
     * @return 0（建筑工人无法攻击）
     */
    int getAttackRange() const override { 
        return 0; 
    }

    /**
     * @brief 检查是否能改良地块
     * @return true（建筑工人可以改良地块）
     * 
     * 建筑工人可以进行以下改良：
     * - 建造农田（提升食物产出）
     * - 建造矿山（提升生产力）
     * - 建造道路（加快移动速度）
     * - 建造防御工事（防守提升）
     * 
     * @note 建议在 AbstractUnit 中添加虚函数 canBuildImprovements()
     */
    bool canBuildImprovements() const { 
        return true; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Builder);
};

#endif // __BUILDER_H__