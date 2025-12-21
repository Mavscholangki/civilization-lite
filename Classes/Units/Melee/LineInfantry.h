#ifndef __LINEINFANTRY_H__
#define __LINEINFANTRY_H__

#include "../Base/AbstractUnit.h"

/**
 * @file LineInfantry.h
 * @brief 列兵单位类
 * 
 * 列兵是一种现代近战军事单位
 * 具有更强的生命值和攻击力，相比战士更加坚韧
 */

/**
 * @class LineInfantry
 * @brief 列兵单位
 * 
 * 列兵的特点：
 * - 攻击力为40（强大的近战伤害）
 * - 生命值200（优秀的生存能力）
 * - 移动力2格/回合
 * - 攻击范围为1（仅近战相邻攻击）
 * - 不能建立城市
 * 
 * 与战士的区别：
 * - Warrior（战士）：攻击力20，生命值100（早期单位）
 * - LineInfantry（列兵）：攻击力40，生命值200（中期单位）
 * 
 * 列兵是战士的进阶版本，适合作为主力防线
 */
class LineInfantry : public AbstractUnit {
public:

    /**
     * @brief 获取单位名称
     * @return 列兵名称
     */
    std::string getUnitName() const override { 
        return "LineInfantry"; 
    }

    /**
     * @brief 获取单位类型
     * @return 近战军事单位类型
     */
    UnitType getUnitType() const override { 
        return UnitType::MELEE; 
    }

    int getCost() const override {
        return 600;
    }

    int getMaintenanceCost() const override {
        return 3;
	}

    int getProductionCost() const override {
        return 60;
	}

    bool ismilitary() const override {
        return true;
    }
    /**
     * @brief 获取单位精灵图片路径
     * @return 图片文件路径
     * 
     * @note 原文件名为 "lneInfantry.png"（拼写可能有误）
     *       建议检查资源文件夹中的实际文件名
     */
    std::string getSpritePath() const override { 
        return "units/lneInfantry.png"; 
    }

    /**
     * @brief 获取最大生命值
     * @return 200 点生命值（比战士高2倍）
     * 
     * 列兵具有优异的防御和生存能力
     */
    int getMaxHp() const override { 
        return 200; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 40（列兵强大的近战伤害）
     * 
     * 列兵的攻击力是战士的2倍
     * 在近战中能造成致命伤害
     */
    int getBaseAttack() const override { 
        return 40; 
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
     * @return 1（近战仅能攻击相邻格子）
     * 
     * 列兵是近战单位，只能攻击相邻的敌人
     */
    int getAttackRange() const override { 
        return 1; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(LineInfantry);
};

#endif // __LINEINFANTRY_H__