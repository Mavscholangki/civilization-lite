#ifndef __JETFLIGHTER_H__
#define __JETFLIGHTER_H__

#include "../Base/AbstractUnit.h"

/**
 * @file JetFighter.h
 * @brief 喷气战斗机单位类
 * 
 * 喷气战斗机是一种超现代飞行军事单位
 * 拥有最高的攻击力、生命值和综合作战能力
 */

/**
 * @class JetFighter
 * @brief 喷气战斗机单位
 * 
 * 喷气战斗机的特点：
 * - 攻击力为120（最高的攻击力）
 * - 生命值200（最高的生命值）
 * - 移动力10格/回合（最快的移动速度）
 * - 攻击范围为5（最远的攻击射程）
 * - 能飞行（可越过所有地形障碍）
 * - 视野范围6（最广阔的视野）
 * - 不能建立城市
 * 
 * 喷气战斗机是游戏中最强大的单位
 * 各项属性都是同类单位中的最高值
 * 集侦察、攻击、防御于一身，是终极战争兵器
 */
class JetFighter : public AbstractUnit {
public:

    /**
     * @brief 获取单位名称
     * @return 喷气战斗机名称
     */
    std::string getUnitName() const override { 
        return "JetFighter"; 
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
        return "units/jetFighter.png"; 
    }

    /**
     * @brief 获取最大生命值
     * @return 200 点生命值（最高）
     * 
     * 喷气战斗机拥有最强的生存能力
     */
    int getMaxHp() const override { 
        return 200; 
    }

    /**
     * @brief 获取基础攻击力
     * @return 120（最强的远程轰炸力）
     * 
     * 喷气战斗机拥有最高的攻击伤害输出
     */
    int getBaseAttack() const override { 
        return 120; 
    }

    /**
     * @brief 获取最大移动力
     * @return 10 格/回合（最快的移动速度）
     * 
     * 喷气战斗机可以快速穿越整个战场
     */
    int getMaxMoves() const override { 
        return 10; 
    }

    /**
     * @brief 获取攻击范围
     * @return 5（最远的远程轰炸射程）
     * 
     * 喷气战斗机可以从远距离打击目标
     */
    int getAttackRange() const override { 
        return 5; 
    }

    /**
     * @brief 检查单位是否能飞行
     * @return true（喷气战斗机能飞行）
     * 
     * 飞行能力使其可以无视所有地形障碍
     * 包括山脉、水域、丛林等
     */
    bool canFly() const override { 
        return true; 
    }

    /**
     * @brief 获取视野范围
     * @return 6（最广阔的空中视野）
     * 
     * 喷气战斗机拥有最广的侦察范围
     * 可以掌握整个战场的局势
     */
    int getVisionRange() const override { 
        return 6; 
    }

    // Cocos2d-x 标准宏，自动实现 create() 工厂函数
    CREATE_FUNC(JetFighter);
};

#endif // __JETFLIGHTER_H__