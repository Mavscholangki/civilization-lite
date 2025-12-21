#ifndef __CATAPULT_H__
#define __CATAPULT_H__

#include "../Base/AbstractUnit.h"

/**
 * @class Catapult
 * @brief 弩车单位（攻城军事单位）
 * 
 * 弩车的特点：
 * - 攻击力为25（攻城特化）
 * - 生命值60（脆弱）
 * - 移动力1格/回合（笨重）
 * - 攻击范围为3（远程炮击）
 * - 不能建立城市
 */
class Catapult : public AbstractUnit {
public:

    /**
     * @brief 获取单位名称
     * @return 弩车名称
     */
    std::string getUnitName() const override { 
        return "Catapult"; 
    }

    /**
     * @brief 获取单位类型
     * @return 攻城军事单位类型
     */
    UnitType getUnitType() const override { 
        return UnitType::SIEGE; 
    }

    /**
     * @brief 获取单位精灵图片路径
     * @return 图片文件路径
     */
    std::string getSpritePath() const override { 
        return "units/catapult.png"; 
    }
    int getCost() const override {
        return 200;
    }

    int getMaintenanceCost() const override {
        return 2;
    }

    int getProductionCost() const override {
        return 30;
    }

    bool ismilitary() const override {
        return true;
    }

    /**
     * @brief 获取最大生命值
     * @return 60 点生命值
     */
    int getMaxHp() const override { 
        return 60; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 25（弩车的强大攻城能力）
     */
    int getBaseAttack() const override { 
        return 25; 
    }

    /**
     * @brief 获取最大移动力
     * @return 1 格/回合（移动缓慢）
     */
    int getMaxMoves() const override { 
        return 1; 
    }

    /**
     * @brief 获取攻击范围
     * @return 3（长距离炮击）
     */
    int getAttackRange() const override { 
        return 3; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Catapult);
};

#endif // __CATAPULT_H__