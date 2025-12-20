#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "../Base/AbstractUnit.h"

/**
 * @class Archer
 * @brief 弓箭手单位（远程军事单位）
 * 
 * 弓箭手的特点：
 * - 攻击力为15
 * - 生命值80
 * - 移动力2格/回合
 * - 攻击范围为2（远程）
 * - 不能建立城市
 */
class Archer : public AbstractUnit {
public:

    /**
     * @brief 获取单位名称
     * @return 弓箭手名称
     */
    std::string getUnitName() const override { 
        return "Archer"; 
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
        return "units/archer.png"; 
    }

    /**
     * @brief 获取最大生命值
     * @return 80 点生命值
     */
    int getMaxHp() const override { 
        return 80; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 15（弓箭手的远程攻击力）
     */
    int getBaseAttack() const override { 
        return 15; 
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
     * @return 2（远程可达）
     */
    int getAttackRange() const override { 
        return 2; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Archer);
};

#endif // __ARCHER_H__