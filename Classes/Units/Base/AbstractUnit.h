#ifndef __ABSTRACT_UNIT_H__
#define __ABSTRACT_UNIT_H__

#include "cocos2d.h"
#include "../../Utils/HexUtils.h"
#include "../../Utils/PathFinder.h"
#include <functional>

/**
 * @brief 单位类型枚举
 * 用于AI判断优先攻击目标或UI分类
 */
enum class UnitType {
    CIVILIAN, // 平民（无战斗力，如工人、移民）
    MELEE,    // 近战
    RANGED,   // 远程
    SIEGE,     // 攻城
	AVIATION   // 航空
};

/**
 * @brief 单位状态枚举
 */
enum class UnitState {
    IDLE,       // 闲置
    MOVING,     // 正在移动动画中
    ATTACKING,  // 正在攻击动画中
    DEAD        // 已死亡
};

/**
 * @class AbstractUnit
 * @brief 所有游戏单位的抽象基类
 *
 * 职责：
 * 1. 管理单位的核心数据（血量、位置、归属）
 * 2. 处理通用视觉表现（移动动画、血条更新、选中高亮）
 * 3. 定义子类必须实现的接口（攻击力、移动力等配置）
 */

class AbstractUnit : public cocos2d::Node {
public:
    AbstractUnit();
    virtual ~AbstractUnit();

    using CheckCityCallback = std::function<bool(Hex)>;
    CheckCityCallback onCheckCity;

    // ==========================================
    // 1. 初始化与生命周期
    // ==========================================

     /**
     * @brief 标准 Cocos 初始化 (由 CREATE_FUNC 调用)
     */
    virtual bool init() override { return Node::init(); }

    /**
     * @brief 初始化单位
     * @param ownerId 玩家ID (0:玩家, 1:敌人, etc.)
     * @param startPos 初始网格坐标
     * @return true 初始化成功
     */
    virtual bool initUnit(int ownerId, Hex startPos);

    /**
     * @brief 回合开始时的重置逻辑
     * 恢复移动力，重置状态颜色
     */
    virtual void onTurnStart();

    /**
	*  @brief 设置是否已行动
    */
    void setActed(bool acted) { _hasActed = acted; }

    /**
	* @brief 查询是否已行动
    */
    bool hasActed() const { return _hasActed; }

    // ==========================================
    // 2. 纯虚函数 (必须由子类实现 - 配置数据)
    // ==========================================

    virtual std::string getUnitName() const = 0;    // 单位名称
    virtual UnitType getUnitType() const = 0;       // 单位类型
    virtual std::string getSpritePath() const = 0;  // 图片路径

    // --- 基础属性 ---
    virtual int getMaxHp() const = 0;       // 最大生命值
    virtual int getBaseAttack() const = 0;  // 基础攻击力
    virtual int getMaxMoves() const = 0;    // 最大移动力
    virtual int getAttackRange() const = 0; // 攻击距离 (1=近战)
    virtual int getVisionRange() const { return 2; } // 视野范围 (默认2)
	virtual int getCost() const = 0;          // 购买/训练成本
	virtual int getMaintenanceCost() const = 0; // 维护费用
	virtual int getProductionCost() const = 0;   // 生产力所需

    // ==========================================
    // 3. 特殊能力开关 (虚函数，默认为 false)
    // ==========================================
    virtual bool ismilitary() const { return true; } // 是否为军队
    virtual bool canFoundCity() const { return false; } // 能否建城
    virtual bool canFly() const { return false; }       // 能否飞行
    virtual bool canMoveAfterAttack() const { return false; } // 攻击后能否移动

    // ==========================================
    // 4. 核心行为逻辑
    // ==========================================

    /**
     * @brief 移动到目标位置
     * @param targetPos 目标六边形坐标
     * @param layout 用于将Hex转换为屏幕坐标的布局
     * @param pathCost 路径消耗的移动力（默认-1表示使用直线距离）
     */
    void moveTo(Hex targetPos, HexLayout* layout, int pathCost = -1);

    /**
     * @brief 受到伤害
     * @param damage 伤害数值
     * @return 实际扣除的血量
     */
    int takeDamage(int damage);

    /**
     * @brief 俘虏单位 (强制变更所属权)
     * @param newOwnerId 新的所属玩家ID
     */
    void capture(int newOwnerId);

    /**
     * @brief 刷新外观颜色 (用于初始化或被俘虏后)
     */
    void updateVisualColor();

    /**
     * @brief 攻击目标单位
     * @param target 目标指针
     * @param layout 用于计算动画方向
     */

    void attack(AbstractUnit* target, HexLayout* layout);

    /**
     * @brief 瞬间传送 (无动画，直接设置坐标)
     */
    void teleportTo(Hex pos, HexLayout* layout);

    /**
     * @brief 设置选中状态 (显示光圈)
     */
    void setSelected(bool selected);

    // ==========================================
    // 5. 状态查询 (Getters)
    // ==========================================

    bool isAlive() const { return _currentHp > 0; }
    bool canAct() const { return _currentMoves > 0 && isAlive(); }

    int getOwnerId() const { return _ownerId; }
    Hex getGridPos() const { return _gridPos; }
    int getCurrentHp() const { return _currentHp; }
    int getCurrentMoves() const { return _currentMoves; }
    UnitState getState() const { return _state; }

    // 计算综合战斗力 (考虑血量损耗)
    int getCombatPower() const;


    // ==========================================
    // 6. 可达范围 (range)
    // ==========================================
    /**
     * @brief 显示移动范围
     * @param layout 布局工具
     * @param getCost 地形消耗回调
     */
    void showMoveRange(HexLayout* layout, std::function<int(Hex)> getCost);

    /**
     * @brief 隐藏移动范围
     */
    void hideMoveRange();

protected:
    // --- 内部辅助 ---
    void updateHpBar();      // 刷新血条UI
    void onDeath();          // 死亡处理

    // --- 成员变量 ---
    int _ownerId;            // 所属玩家
    Hex _gridPos;            // 逻辑坐标
    int _currentHp;          // 当前血量
    int _currentMoves;       // 当前移动力
    UnitState _state;        // 动作状态
    bool _hasActed;

    // --- 视觉节点 ---
    cocos2d::Sprite* _unitSprite;    // 单位外观
    cocos2d::Sprite* _selectionRing; // 选中光圈
    cocos2d::DrawNode* _hpBarNode;       // 血条绘制节点

    cocos2d::DrawNode* _rangeNode; // 可达边框

};

#endif // __ABSTRACT_UNIT_H__