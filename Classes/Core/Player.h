#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "cocos2d.h"
#include "../Civilizations/BaseCiv.h"
#include "../Development/TechSystem.h"
#include "../Development/CultureSystem.h"
#include "../Development/PolicySystem.h"
#include "../City/Yield.h"
#include "../Units/Base/AbstractUnit.h"
#include "Development/ProductionProgram.h"
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

/**
 * 玩家类 - 代表游戏中的一个玩家，管理其资源、城市、单位和科技发展
 */
class Player : public cocos2d::Ref, public TechEventListener, public CultureEventListener {
public:
    /**
     * 玩家状态枚举
     */
    enum class PlayerState {
        ACTIVE,         // 活动状态
        DEFEATED,       // 已被击败
        VICTORIOUS,     // 已获胜
        AI_CONTROLLED   // AI控制
    };

    /**
     * 胜利进度追踪结构
     */
    struct VictoryProgress {
        bool hasSatelliteTech;         // 是否解锁卫星科技
        bool hasLaunchedSatellite;     // 是否发射卫星
        bool hasDominationVictory;     // 是否达成统治胜利
        std::vector<int> controlledCapitals; // 控制的玩家首都ID列表

        /**
         * 默认构造函数
         */
        VictoryProgress()
            : hasSatelliteTech(false), hasLaunchedSatellite(false),
            hasDominationVictory(false) {
        }
    };

    /**
     * 回合统计结构
     */
    struct TurnStats {
        int scienceGenerated;     // 本回合生成的科技值
        int cultureGenerated;     // 本回合生成的文化值
        int goldGenerated;        // 本回合生成的金币
        int unitsTrained;         // 本回合训练的单位数
        int buildingsConstructed; // 本回合建造的建筑数

        /**
         * 默认构造函数
         */
        TurnStats()
            : scienceGenerated(0), cultureGenerated(0), goldGenerated(0),
            unitsTrained(0), buildingsConstructed(0) {
        }
    };

    /**
     * 构造函数 - 初始化玩家对象
     * 注意：PolicyManager需要CultureTree指针作为参数
     */
    Player();

    /**
     * 创建玩家静态方法
     * @param playerId 玩家ID
     * @param civType 文明类型
     * @return 创建的玩家对象，失败返回nullptr
     */
    static Player* create(int playerId, CivilizationType civType);

    /**
     * 初始化玩家
     * @param playerId 玩家ID
     * @param civType 文明类型
     * @return 初始化成功返回true，失败返回false
     */
    bool init(int playerId, CivilizationType civType);

    /**
     * 析构函数
     */
    virtual ~Player();

    // ==================== 回合管理 ====================
    /**
     * 回合开始处理
     */
    void onTurnBegin();

    /**
     * 分发资源变化事件
     */
    void dispatchResourceChangedEvent();

    /**
     * 更新研究进度
     */
    void updateResearchProgress();

    /**
     * 回合结束处理
     */
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
    /**
     * 获取文明对象
     * @return 文明对象指针
     */
    BaseCiv* getCivilization() { return m_civilization; }

    /**
     * 获取科技树对象
     * @return 科技树对象引用
     */
    TechTree* getTechTree() { return &m_techTree; }

    /**
     * 获取文化树对象
     * @return 文化树对象引用
     */
    CultureTree* getCultureTree() { return &m_cultureTree; }

    /**
     * 获取政策管理器对象
     * @return 政策管理器对象引用
     */
    PolicyManager* getPolicyManager() { return &m_policyManager; }

    /**
     * 获取当前回合统计
     * @return 回合统计结构引用
     */
    const TurnStats& getTurnStats() const { return m_turnStats; }

    // ==================== 城市管理 ====================
    /**
     * 添加城市
     * @param city 城市对象
     */
    void addCity(BaseCity* city);

    /**
     * 移除城市
     * @param city 城市对象
     */
    void removeCity(BaseCity* city);

    /**
     * 获取所有城市
     * @return 城市列表
     */
    const std::vector<BaseCity*>& getCities() const { return m_cities; }

    /**
     * 获取首都
     * @return 首都城市对象，没有首都返回nullptr
     */
    BaseCity* getCapital() const;

    /**
     * 获取城市数量
     * @return 城市数量
     */
    int getCityCount() const { return m_cities.size(); }

    /**
     * 计算所有城市的总产出
     * @return 总产出结构
     */
    Yield calculateTotalYield() const;

    /**
     * 计算基础产出（无加成）
     * @return 基础产出结构
     */
    Yield calculateBaseYield() const;

    /**
     * 检查城市是否是首都
     * @param city 城市对象
     * @return 是首都返回true，否则返回false
     */
    bool isCapital(BaseCity* city) const;

    /**
     * 添加控制的玩家首都ID
     * @param playerId 玩家ID
     */
    void addControlledCapital(int playerId);

    /**
     * 移除控制的玩家首都ID
     * @param playerId 玩家ID
     */
    void removeControlledCapital(int playerId);

    /**
     * 获取控制的首都列表
     * @return 控制的玩家首都ID列表
     */
    std::vector<int> getControlledCapitals() const;

    /**
     * 检查是否控制指定玩家的首都
     * @param playerId 玩家ID
     * @return 控制返回true，否则返回false
     */
    bool controlsCapitalOf(int playerId) const;

    /**
     * 获取首都玩家ID
     * @return 玩家ID
     */
    int getCapitalPlayerId() const { return m_playerId; }

    // ==================== 单位管理 ====================
    /**
     * 添加单位
     * @param unit 单位对象
     */
    void addUnit(AbstractUnit* unit);

    /**
     * 移除单位
     * @param unit 单位对象
     */
    void removeUnit(AbstractUnit* unit);

    /**
     * 获取所有单位
     * @return 单位列表
     */
    const std::vector<AbstractUnit*>& getUnits() const { return m_units; }

    /**
     * 获取单位数量
     * @return 单位数量
     */
    int getUnitCount() const { return m_units.size(); }

    // ==================== 起始单位管理 ====================
    /**
     * 创建起始开拓者单位
     * @param parentNode 父节点
     * @param getStartHexFunc 获取起始位置函数
     * @param addToMapFunc 添加到地图函数
     * @param checkCityFunc 检查城市函数
     * @return 创建的开拓者单位，失败返回nullptr
     */
    AbstractUnit* createStartingSettler(cocos2d::Node* parentNode,
        std::function<Hex()> getStartHexFunc,
        std::function<void(AbstractUnit*)> addToMapFunc,
        std::function<bool(Hex)> checkCityFunc);

    /**
     * 设置地图回调函数
     * @param getStartHexFunc 获取起始位置函数
     * @param addToMapFunc 添加到地图函数
     * @param checkCityFunc 检查城市函数
     * @param getTerrainCostFunc 获取地形消耗函数
     */
    void setMapCallbacks(std::function<Hex()> getStartHexFunc,
        std::function<void(AbstractUnit*)> addToMapFunc,
        std::function<bool(Hex)> checkCityFunc,
        std::function<int(Hex)> getTerrainCostFunc);

    /**
     * 添加单位到地图
     * @param unit 单位对象
     */
    void addToMapFunc(AbstractUnit* unit) { m_addToMapFunc(unit); }

    // ==================== 科技系统接口 ====================
    /**
     * 科技激活回调
     * @param techId 科技ID
     * @param techName 科技名称
     * @param effect 效果描述
     */
    virtual void onTechActivated(int techId, const std::string& techName,
        const std::string& effect) override;

    /**
     * 研究进度回调
     * @param techId 科技ID
     * @param currentProgress 当前进度
     * @param totalCost 总成本
     */
    virtual void onResearchProgress(int techId, int currentProgress,
        int totalCost) override;

    /**
     * 尤里卡触发回调
     * @param techId 科技ID
     * @param techName 科技名称
     */
    virtual void onEurekaTriggered(int techId, const std::string& techName) override;

    /**
     * 更新解锁的生产项目
     */
    void updateUnlockedProduction();

    /**
     * 获取解锁的生产项目
     * @param Units 单位列表输出
     * @param Districts 区域列表输出
     * @param Buildings 建筑列表输出
     */
    void getUnlockedProduction(std::vector<ProductionProgram*>& Units,
        std::vector<ProductionProgram*>& Districts,
        std::vector<ProductionProgram*>& Buildings);

    /**
     * 添加科技值
     * @param amount 科技值数量
     */
    void addScience(int amount);

    /**
     * 获取每回合科技产出
     * @return 每回合科技产出值
     */
    int getSciencePerTurn() const;

    /**
     * 设置当前研究科技
     * @param techId 科技ID
     */
    void setCurrentResearch(int techId);

    /**
     * 获取当前研究科技ID
     * @return 科技ID，无研究返回-1
     */
    int getCurrentResearchTechId() const { return m_techTree.getCurrentResearch(); }

    // ==================== 文化系统接口 ====================
    /**
     * 文化解锁回调
     * @param cultureId 文化ID
     * @param cultureName 文化名称
     * @param effect 效果描述
     */
    virtual void onCultureUnlocked(int cultureId, const std::string& cultureName,
        const std::string& effect) override;

    /**
     * 文化进度回调
     * @param cultureId 文化ID
     * @param currentProgress 当前进度
     * @param totalCost 总成本
     */
    virtual void onCultureProgress(int cultureId, int currentProgress,
        int totalCost) override;

    /**
     * 灵感触发回调
     * @param cultureId 文化ID
     * @param cultureName 文化名称
     */
    virtual void onInspirationTriggered(int cultureId,
        const std::string& cultureName) override;

    /**
     * 添加文化值
     * @param amount 文化值数量
     */
    void addCulture(int amount);

    /**
     * 获取每回合文化产出
     * @return 每回合文化产出值
     */
    int getCulturePerTurn() const;

    /**
     * 设置当前研究市政
     * @param cultureId 文化ID
     */
    void setCurrentCivic(int cultureId);

    /**
     * 获取当前研究市政ID
     * @return 市政ID，无研究返回-1
     */
    int getCurrentResearchCivicId() const { return m_cultureTree.getCurrentResearch(); }

    // ==================== 经济系统 ====================
    /**
     * 计算维护费用
     * @return 维护费用
     */
    int calculateMaintenanceCost() const;

    /**
     * 计算每回合净金币收入
     * @return 净金币收入
     */
    int calculateNetGoldPerTurn() const;

    /**
     * 检查是否能够支付
     * @param goldCost 金币成本
     * @return 能够支付返回true，否则返回false
     */
    bool canAfford(int goldCost) const { return m_gold >= goldCost; }

    /**
     * 花费金币
     * @param amount 花费金额
     */
    void spendGold(int amount);

    // ==================== 文明加成应用方法 ====================
    /**
     * 应用科技加成
     * @param baseScience 基础科技值
     * @return 加成后的科技值
     */
    int applyScienceBonus(int baseScience) const;

    /**
     * 应用文化加成
     * @param baseCulture 基础文化值
     * @return 加成后的文化值
     */
    int applyCultureBonus(int baseCulture) const;

    /**
     * 应用尤里卡加成
     * @param techId 科技ID
     */
    void applyEurekaBonus(int techId);

    /**
     * 应用灵感加成
     * @param cultureId 文化ID
     */
    void applyInspirationBonus(int cultureId);

    /**
     * 获取文明特性名称
     * @return 特性名称
     */
    std::string getCivilizationTraitName() const;

    /**
     * 获取文明特性描述
     * @return 特性描述
     */
    std::string getCivilizationTraitDescription() const;

    /**
     * 检查是否有指定文明加成
     * @param bonusName 加成名称
     * @return 有加成返回true，否则返回false
     */
    bool hasCivilizationBonus(const std::string& bonusName) const;

    // ==================== 胜利条件 ====================
    /**
     * 检查是否达成科技胜利
     * @return 达成返回true，否则返回false
     */
    bool checkScienceVictory() const;

    /**
     * 检查是否达成征服胜利
     * @return 达成返回true，否则返回false
     */
    bool checkDominationVictory() const;

    // ==================== 序列化 ====================
    /**
     * 将玩家状态序列化为ValueMap
     * @return 包含玩家状态的ValueMap
     */
    cocos2d::ValueMap toValueMap() const;

    /**
     * 从ValueMap反序列化玩家状态
     * @param data 包含玩家状态的ValueMap
     * @return 反序列化成功返回true，失败返回false
     */
    bool fromValueMap(const cocos2d::ValueMap& data);

    // ==================== 调试和测试 ====================
    /**
     * 调试输出玩家状态
     */
    void debugPrintStatus() const;

private:
    friend class GameManager;

    // ==================== 核心子系统 ====================
    BaseCiv* m_civilization = nullptr;      // 文明特性
    TechTree m_techTree;                    // 科技树
    CultureTree m_cultureTree;              // 文化树
    PolicyManager m_policyManager;          // 政策管理器
    std::vector<ProductionProgram*> unlockedUnits;     // 解锁的单位
    std::vector<ProductionProgram*> unlockedDistricts; // 解锁的区域
    std::vector<ProductionProgram*> unlockedBuildings; // 解锁的建筑

    // ==================== 游戏实体 ====================
    std::vector<BaseCity*> m_cities;        // 城市列表
    std::vector<AbstractUnit*> m_units;     // 单位列表

    // 起始单位回调函数
    std::function<Hex()> m_getStartHexFunc;             // 获取起始位置函数
    std::function<void(AbstractUnit*)> m_addToMapFunc;  // 添加到地图函数
    std::function<bool(Hex)> m_checkCityFunc;           // 检查城市函数
    std::function<int(Hex)> m_getTerrainCostFunc;       // 获取地形消耗函数

    // 起始单位
    AbstractUnit* m_startingSettler = nullptr;          // 起始开拓者

    // ==================== 回合统计 ====================
    TurnStats m_turnStats;                  // 回合统计

    // ==================== 胜利进度 ====================
    VictoryProgress m_vicprogress;          // 胜利进度

    // ==================== 私有方法 ====================
    /**
     * 更新政策槽位
     */
    void updatePolicySlots();

    /**
     * 创建文明
     * @param civType 文明类型
     */
    void createCivilization(CivilizationType civType);

    /**
     * 清理资源
     */
    void cleanupResources();

    /**
     * 应用所有文明加成
     */
    void applyAllCivilizationBonuses();

    /**
     * 更新文明加成状态
     */
    void updateCivilizationBonusState();
};

#endif // __PLAYER_H__