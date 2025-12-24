#ifndef __SWORDSMAN_H__
#define __SWORDSMAN_H__

#include "../Base/AbstractUnit.h"

/**
 * @file Swordsman.h
 * @brief 剑士单位类
 * 
 * 剑士是一种古代近战军事单位
 * 具有均衡的攻击力和生命值，是早期到中期的主力单位
 */

/**
 * @class Swordsman
 * @brief 剑士单位
 * 
 * 剑士的特点：
 * - 攻击力为30（中等的近战伤害）
 * - 生命值150（良好的生存能力）
 * - 移动力2格/回合
 * - 攻击范围为1（仅近战相邻攻击）
 * - 不能建立城市
 * 
 * 与其他近战单位的对比：
 * - Warrior（战士）：攻击力20，生命值100（最初级）
 * - Swordsman（剑士）：攻击力30，生命值150（中级，均衡）
 * - LineInfantry（列兵）：攻击力40，生命值200（高级，坚固）
 * 
 * 剑士是战士的升级版本，提供了更好的生存能力和输出平衡
 */
class Swordsman : public AbstractUnit {
public:
    Swordsman() : AbstractUnit() { cost = 40; purchaseCost = 500; prereqTechID = 7; } // 前置科技：铁器(ID 7)

    /**
     * @brief 获取单位名称
     * @return 剑士名称
     */
    std::string getUnitName() const override { 
        return "Swordsman"; 
    }

    /**
     * @brief 获取单位类型
     * @return 近战军事单位类型
     */
    UnitType getUnitType() const override { 
        return UnitType::MELEE; 
    }

    int getMaintenanceCost() const override {
        return 2;
    }

    bool ismilitary() const override {
        return true;
    }
    /**
     * @brief 获取单位精灵图片路径
     * @return 图片文件路径
     */

    std::string getSpritePath() const override { 
        return "units/swordsman.png"; 
    }
    i
    /**
     * @brief 获取最大生命值
     * @return 150 点生命值
     * 
     * 剑士的生命值介于战士和列兵之间
     * 提供不错的防御能力
     */
    int getMaxHp() const override { 
        return 150; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 30（剑士平衡的近战伤害）
     * 
     * 剑士的攻击力介于战士和列兵之间
     * 提供稳定的输出能力
     */
    int getBaseAttack() const override { 
        return 30; 
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
     * 剑士是近战单位，只能攻击相邻的敌人
     */
    int getAttackRange() const override { 
        return 1; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Swordsman);
};

#endif // __SWORDSMAN_H__