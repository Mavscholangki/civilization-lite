#ifndef __CROSSBOWMAN_H__
#define __CROSSBOWMAN_H__

#include "../Base/AbstractUnit.h"

/**
 * @file Crossbowman.h
 * @brief 弩手单位类
 * 
 * 弩手是一种中世纪远程军事单位
 * 具有更强的攻击力和生命值，介于弓箭手和火枪手之间
 */

/**
 * @class Crossbowman
 * @brief 弩手单位
 * 
 * 弩手的特点：
 * - 攻击力为35（中等远程伤害）
 * - 生命值150
 * - 移动力2格/回合
 * - 攻击范围为2（中距离射击）
 * - 不能建立城市
 * 
 * 与其他远程单位的对比：
 * - Archer（弓箭手）：攻击力15，生命值80（早期远程）
 * - Crossbowman（弩手）：攻击力35，生命值150（中期远程）
 * - Musketeers（火枪手）：攻击力60，生命值200（晚期远程）
 * 
 * 弩手是远程单位的中期升级版本
 * 提供比弓箭手更强的火力和防御能力
 */
class Crossbowman : public AbstractUnit {
public:

    /**
     * @brief 获取单位名称
     * @return 弩手名称
     */
    std::string getUnitName() const override { 
        return "Crossbowman"; 
    }

    /**
     * @brief 获取单位类型
     * @return 远程军事单位类型
     */
    UnitType getUnitType() const override { 
        return UnitType::RANGED; 
    }

    /**
     * @brief 获取单位精灵图片路径
     * @return 图片文件路径
     */
    std::string getSpritePath() const override { 
        return "units/crossbowman.png"; 
    }

    int getCost() const override {
        return 500;
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

    /**
     * @brief 获取最大生命值
     * @return 150 点生命值
     * 
     * 弩手相比弓箭手生命值提升50%
     * 相比火枪手生命值少25%
     */
    int getMaxHp() const override { 
        return 150; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 35（弩的中等远程伤害）
     * 
     * 弩手的攻击力是弓箭手的2.33倍
     * 但不及火枪手的60点攻击力
     */
    int getBaseAttack() const override { 
        return 35; 
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
     * @return 2（弩的中距离射击范围）
     * 
     * 弩手与弓箭手射程相同，都是2格
     * 但攻击力更强
     */
    int getAttackRange() const override { 
        return 2; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Crossbowman);
};

#endif // __CROSSBOWMAN_H__