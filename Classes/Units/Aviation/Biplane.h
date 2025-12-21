#ifndef __BIPLANE_H__
#define __BIPLANE_H__

#include "../Base/AbstractUnit.h"

/**
 * @file Biplane.h
 * @brief 双翼飞机单位类
 * 
 * 双翼飞机是一种现代飞行军事单位
 * 具有最高的移动速度和远程攻击能力
 */

/**
 * @class Biplane
 * @brief 双翼飞机单位
 * 
 * 双翼飞机的特点：
 * - 攻击力为60（最强攻击力）
 * - 生命值120
 * - 移动力6格/回合（最快移动速度）
 * - 攻击范围为3（远程轰炸）
 * - 能飞行（可越过所有地形障碍）
 * - 视野范围4（最广视野）
 * - 不能建立城市
 * 
 * 双翼飞机是最强大的空中单位，适合进行远程轰炸和快速侦察
 */
class Biplane : public AbstractUnit {
public:

    /**
     * @brief 获取单位名称
     * @return 双翼飞机名称
     */
    std::string getUnitName() const override { 
        return "Biplane"; 
    }

    /**
     * @brief 获取单位类型
     * @return 飞行军事单位类型
     */
    UnitType getUnitType() const override { 
        return UnitType::AVIATION; 
    }

    /**
     * @brief 获取单位精灵图片路径
     * @return 图片文件路径
     */
    std::string getSpritePath() const override { 
        return "units/biplane.png"; 
    }

    int getCost() const override {
        return 1000; 
	}

    int getMaintenanceCost() const override {
        return 5;
    }

    int getProductionCost() const override {
        return 60;
	}

    bool ismilitary() const override {
        return true; 
	}
    /**
     * @brief 获取最大生命值
     * @return 120 点生命值
     */
    int getMaxHp() const override { 
        return 120; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 60（最强的远程攻击力）
     */
    int getBaseAttack() const override { 
        return 60; 
    }

    /**
     * @brief 获取最大移动力
     * @return 6 格/回合（最快的移动速度）
     */
    int getMaxMoves() const override { 
        return 6; 
    }

    /**
     * @brief 获取攻击范围
     * @return 3（远程轰炸射程）
     */
    int getAttackRange() const override { 
        return 3; 
    }

    /**
     * @brief 检查单位是否能飞行
     * @return true（双翼飞机能飞行）
     * 
     * 飞行能力使其可以无视山脉、水域等地形障碍
     */
    bool canFly() const override { 
        return true; 
    }

    /**
     * @brief 获取视野范围
     * @return 4（广阔的空中视野）
     * 
     * 空中单位拥有更广阔的视野，可以侦察更大区域
     */
    int getVisionRange() const override { 
        return 4; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(Biplane);
};

#endif // __BIPLANE_H__