#ifndef __MUSKETEERS_H__
#define __MUSKETEERS_H__

#include "../Base/AbstractUnit.h"

/**
 * @file Musketeers.h
 * @brief 火枪手单位类
 * 
 * 火枪手是一种现代远程军事单位
 * 具有高攻击力和远程射程的平衡特性
 */

/**
 * @class Musketeers
 * @brief 火枪手单位
 * 
 * 火枪手的特点：
 * - 攻击力为60（远程单位中的强大伤害）
 * - 生命值200
 * - 移动力2格/回合
 * - 攻击范围为2（中距离射击）
 * - 不能建立城市
 * 
 * 与其他远程单位的对比：
 * - Archer（弓箭手）：攻击力15，射程2（早期远程）
 * - Crossbowman（弩手）：攻击力35，射程2（中期远程）
 * - Musketeers（火枪手）：攻击力60，射程2（晚期远程，强火力）
 * 
 * 火枪手是远程单位的终极形态，提供最强大的远程火力支援
 */
class Musketeers : public AbstractUnit {
public:

    /**
     * @brief 获取单位名称
     * @return 火枪手名称
     */
    std::string getUnitName() const override { 
        return "Musketeers"; 
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
        return "units/musketeers.png"; 
    }

    /**
     * @brief 获取最大生命值
     * @return 200 点生命值
     * 
     * 火枪手具有不错的生存能力
     */
    int getMaxHp() const override { 
        return 200; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 60（远程火枪的强大威力）
     * 
     * 注意：在此简化框架中，getBaseAttack() 指代远程单位的主要伤害手段
     * 火枪手的近战能力较弱，但远程火力强大
     */
    int getBaseAttack() const override { 
        return 60; 
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
     * @return 2（火枪中距离射击范围）
     * 
     * 火枪手可以隔着一个格子进行射击
     * 相比弓箭手，火力更强但射程相同
     */
    int getAttackRange() const override { 
        return 2; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Musketeers);
};

#endif // __MUSKETEERS_H__