#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "cocos2d.h"
#include "../Civilizations/BaseCiv.h"
#include "../Development/TechSystem.h"
#include "../Development/CultureSystem.h"
#include "../Development/PolicySystem.h"
#include "../City/Yield.h"
#include "../Units/Base/AbstractUnit.h"
#include <vector>
#include <memory>
#include <functional>

// 文明类型枚举
enum class CivilizationType {
    BASIC,      // 基本文明
    GERMANY,    // 德国
    CHINA,      // 中国
    RUSSIA      // 俄罗斯
};

// 前向声明
class BaseCity;
class AbstractUnit;

class Player : public cocos2d::Ref, public TechEventListener, public CultureEventListener {
public:
    // 玩家状态
    enum class PlayerState {
        ACTIVE,         // 活动状态
        DEFEATED,       // 已被击败
        VICTORIOUS,     // 已获胜
        AI_CONTROLLED   // AI控制
    };

    // 胜利进度结构
    struct VictoryProgress {
        bool hasSatelliteTech;         // 是否解锁卫星科技
        bool hasLaunchedSatellite;     // 是否发射卫星
        bool hasDominationVictory;     // 是否达成统治胜利
        std::vector<int> controlledCapitals; // 控制的玩家首都ID列表

        VictoryProgress()
            : hasSatelliteTech(false), hasLaunchedSatellite(false),
            hasDominationVictory(false) {
        }
    };

    // 回合统计
    struct TurnStats {
        int scienceGenerated;
        int cultureGenerated;
        int goldGenerated;
        int unitsTrained;
        int buildingsConstructed;

        TurnStats()
            : scienceGenerated(0), cultureGenerated(0), goldGenerated(0),
            unitsTrained(0), buildingsConstructed(0) {
        }
    };

    // 构造函数声明 (修改点：必须显式声明以初始化 PolicyManager)
    Player();

    // 创建玩家
    static Player* create(int playerId, CivilizationType civType);

    // 初始化
    bool init(int playerId, CivilizationType civType);

    // 析构函数
    virtual ~Player();

    // ==================== 回合管理 ====================
    void onTurnBegin();
    void dispatchResourceChangedEvent();
    void updateResearchProgress();
    void onTurnEnd();

    // ==================== 属性访问器 ====================
    // 基础属性
    CC_SYNTHESIZE(int, m_playerId, PlayerId);
    CC_SYNTHESIZE(std::string, m_playerName, PlayerName);
    CC_SYNTHESIZE(PlayerState, m_state, State);
    CC_SYNTHESIZE(cocos2d::Color3B, m_color, Color);
    CC_SYNTHESIZE(bool, m_isHuman, IsHuman);

    // 资源属性
    CC_SYNTHESIZE(int, m_gold, Gold);
    CC_SYNTHESIZE(int, m_amenities, Amenities);

    // 科研和文化相关属性
    CC_SYNTHESIZE(int, m_scienceStock, ScienceStock);     // 科技值储备（未用于研究的）
    CC_SYNTHESIZE(int, m_cultureStock, CultureStock);     // 文化值储备（未用于研究的）

    // ==================== 子系统访问器 ====================
    BaseCiv* getCivilization() { return m_civilization; }
    TechTree* getTechTree() { return &m_techTree; }
    CultureTree* getCultureTree() { return &m_cultureTree; }
    PolicyManager* getPolicyManager() { return &m_policyManager; }

    // 获取当前回合统计
    const TurnStats& getTurnStats() const { return m_turnStats; }

    // ==================== 城市管理 ====================
    void addCity(BaseCity* city);
    void removeCity(BaseCity* city);
    const std::vector<BaseCity*>& getCities() const { return m_cities; }
    BaseCity* getCapital() const;
    int getCityCount() const { return m_cities.size(); }

    // 计算所有城市的总产出
    Yield calculateTotalYield() const;
    Yield calculateBaseYield() const;  // 基础产出（无加成）

    bool isCapital(BaseCity* city) const;
    void addControlledCapital(int playerId);
    void removeControlledCapital(int playerId);
    std::vector<int> getControlledCapitals() const;
    bool controlsCapitalOf(int playerId) const;
    int getCapitalPlayerId() const { return m_playerId; } // 返回玩家ID，用于首都标识

    // ==================== 单位管理 ====================
    void addUnit(AbstractUnit* unit);
    void removeUnit(AbstractUnit* unit);
    const std::vector<AbstractUnit*>& getUnits() const { return m_units; }
    int getUnitCount() const { return m_units.size(); }

    // ==================== 起始单位管理 ====================
    AbstractUnit* createStartingSettler(cocos2d::Node* parentNode,
        std::function<Hex()> getStartHexFunc,
        std::function<void(AbstractUnit*)> addToMapFunc,
        std::function<bool(Hex)> checkCityFunc);

    // 设置布局和地图函数回调
    void setMapCallbacks(std::function<Hex()> getStartHexFunc,
        std::function<void(AbstractUnit*)> addToMapFunc,
        std::function<bool(Hex)> checkCityFunc,
        std::function<int(Hex)> getTerrainCostFunc);

    // ==================== 科技系统接口 ====================
    // TechEventListener 实现
    virtual void onTechActivated(int techId, const std::string& techName,
        const std::string& effect) override;
    virtual void onResearchProgress(int techId, int currentProgress,
        int totalCost) override;
    virtual void onEurekaTriggered(int techId, const std::string& techName) override;

    // 科技值管理
    void addScience(int amount);
    int getSciencePerTurn() const;
    void setCurrentResearch(int techId);
    int getCurrentResearchTechId() const { return m_techTree.getCurrentResearch(); }

    // ==================== 文化系统接口 ====================
    // CultureEventListener 实现
    virtual void onCultureUnlocked(int cultureId, const std::string& cultureName,
        const std::string& effect) override;
    virtual void onCultureProgress(int cultureId, int currentProgress,
        int totalCost) override;
    virtual void onInspirationTriggered(int cultureId,
        const std::string& cultureName) override;

    // 文化值管理
    void addCulture(int amount);
    int getCulturePerTurn() const;
    void setCurrentCivic(int cultureId);
    int getCurrentResearchCivicId() const { return m_cultureTree.getCurrentResearch(); }

    // ==================== 经济系统 ====================
    int calculateMaintenanceCost() const;
    int calculateNetGoldPerTurn() const;
    bool canAfford(int goldCost) const { return m_gold >= goldCost; }
    void spendGold(int amount);

    // ==================== 文明加成应用方法 ====================
    int applyScienceBonus(int baseScience) const;
    int applyCultureBonus(int baseCulture) const;
    void applyEurekaBonus(int techId);
    void applyInspirationBonus(int cultureId);
    std::string getCivilizationTraitName() const;
    std::string getCivilizationTraitDescription() const;
    bool hasCivilizationBonus(const std::string& bonusName) const;

    // ==================== 胜利条件 ====================
    bool checkScienceVictory() const;
    bool checkDominationVictory() const;

    // ==================== 序列化 ====================
    cocos2d::ValueMap toValueMap() const;
    bool fromValueMap(const cocos2d::ValueMap& data);

    // ==================== 调试和测试 ====================
    void debugPrintStatus() const;

private:
    friend class GameManager;
    // ==================== 核心子系统 ====================
    BaseCiv* m_civilization = nullptr;      // 文明特性
    TechTree m_techTree;                    // 科技树
    // 注意：m_cultureTree 必须在 m_policyManager 之前声明，以确保初始化顺序正确
    CultureTree m_cultureTree;              // 文化树
    PolicyManager m_policyManager;          // 政策管理器

    // ==================== 游戏实体 ====================
    std::vector<BaseCity*> m_cities;        // 城市列表
    std::vector<AbstractUnit*> m_units;     // 单位列表

    // 起始单位回调函数
    std::function<Hex()> m_getStartHexFunc;
    std::function<void(AbstractUnit*)> m_addToMapFunc;
    std::function<bool(Hex)> m_checkCityFunc;
    std::function<int(Hex)> m_getTerrainCostFunc;

    // 起始单位
    AbstractUnit* m_startingSettler = nullptr;

    // ==================== 回合统计 ====================
    TurnStats m_turnStats;

    // ==================== 胜利进度 ====================
    VictoryProgress m_vicprogress;

    // ==================== 私有方法 ====================
    void updatePolicySlots();
    void createCivilization(CivilizationType civType);
    void cleanupResources();
    void applyAllCivilizationBonuses();
    void updateCivilizationBonusState();
};

#endif // __PLAYER_H__