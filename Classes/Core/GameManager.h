// GameManager.h
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "cocos2d.h"
#include "Player.h"
#include <vector>
#include <map>

const int MAX_AI_PLAYERS = 6;   // 最大AI玩家数量，基于游戏规模
const int MIN_AI_PLAYERS = 1;   // 最小AI玩家数量

// 游戏状态枚举
enum class GameState {
    INITIALIZING,      // 初始化中
    PLAYING,           // 进行中
    PAUSED,            // 暂停
    GAME_OVER          // 游戏结束
};

// 胜利类型枚举
enum class VictoryType {
    NONE,              // 无胜利
    SCIENCE,           // 科技胜利
    DOMINATION         // 征服胜利
};

// 游戏配置结构
struct GameConfig {
    int maxTurns = 500;               // 最大回合数
    bool enableScienceVictory = true; // 启用科技胜利
    bool enableDominationVictory = true; // 启用征服胜利

    GameConfig() {}
};

// 游戏统计结构
struct GameStats {
    int currentTurn = 1;           // 当前回合
    int totalPlayers = 0;          // 总玩家数
    int activePlayers = 0;         // 活跃玩家数
    VictoryType victoryType = VictoryType::NONE; // 胜利类型
    int victoryPlayerId = -1;      // 胜利玩家ID

    GameStats() {}
};

// 首都信息结构
struct CapitalInfo {
    int ownerPlayerId;           // 原始所有者玩家ID
    int currentControllerId;     // 当前控制者玩家ID（可能被占领）
    BaseCity* capitalCity;       // 首都城市指针
    bool isCaptured;             // 是否被占领

    /**
     * 默认构造函数，初始化所有字段为默认值
     */
    CapitalInfo()
        : ownerPlayerId(-1), currentControllerId(-1),
        capitalCity(nullptr), isCaptured(false) {
    }

    /**
     * 参数化构造函数
     * @param ownerId 所有者玩家ID
     * @param city 首都城市对象
     */
    CapitalInfo(int ownerId, BaseCity* city)
        : ownerPlayerId(ownerId), currentControllerId(ownerId),
        capitalCity(city), isCaptured(false) {
    }
};

/**
 * 游戏管理器类
 * 负责管理游戏状态、玩家、回合流程、胜利条件等核心游戏逻辑
 */
class GameManager {
public:
    /**
     * 获取游戏管理器单例实例
     * @return 游戏管理器单例指针
     */
    static GameManager* getInstance();

    /**
     * 初始化游戏
     * @param config 游戏配置参数
     * @return 初始化成功返回true，失败返回false
     */
    bool initialize(const GameConfig& config);

    /**
     * 添加玩家到游戏
     * @param player 玩家对象指针
     */
    void addPlayer(Player* player);

    /**
     * 从游戏中移除玩家
     * @param playerId 玩家ID
     */
    void removePlayer(int playerId);

    /**
     * 获取指定ID的玩家
     * @param playerId 玩家ID
     * @return 玩家对象指针，未找到返回nullptr
     */
    Player* getPlayer(int playerId) const;

    /**
     * 获取所有玩家列表
     * @return 玩家对象指针列表
     */
    const std::vector<Player*>& getAllPlayers() const { return m_players; }

    /**
     * 获取当前回合的玩家
     * @return 当前玩家对象指针
     */
    Player* getCurrentPlayer() const;

    /**
     * 设置当前玩家索引
     * @param index 玩家索引
     */
    void setCurrentPlayer(int index);

    /**
     * 获取下一个玩家
     * @return 下一个玩家对象指针
     */
    Player* getNextPlayer() const;

    /**
     * 结束当前玩家回合，切换到下一个玩家
     */
    void endTurn();

    /**
     * 初始化玩家起始单位
     * @param parentNode 父节点用于显示
     * @param getStartHexForPlayerFunc 获取玩家起始位置函数
     * @param addToMapFunc 添加到地图函数
     * @param checkCityFunc 检查城市函数
     * @param getTerrainCostFunc 获取地形消耗函数
     */
    void initializePlayerStartingUnits(cocos2d::Node* parentNode,
        std::function<Hex(int)> getStartHexForPlayerFunc,
        std::function<void(AbstractUnit*)> addToMapFunc,
        std::function<bool(Hex)> checkCityFunc,
        std::function<int(Hex)> getTerrainCostFunc);

    /**
     * 检查胜利条件
     * @return 当前的胜利类型
     */
    VictoryType checkVictoryConditions();

    /**
     * 获取游戏状态
     * @return 当前游戏状态
     */
    GameState getGameState() const { return m_gameState; }

    /**
     * 获取游戏统计信息
     * @return 游戏统计结构
     */
    const GameStats& getGameStats() const { return m_gameStats; }

    /**
     * 获取游戏配置
     * @return 游戏配置结构
     */
    const GameConfig& getGameConfig() const { return m_gameConfig; }

    /**
     * 将游戏状态序列化为ValueMap
     * @return 包含游戏状态的ValueMap
     */
    cocos2d::ValueMap toValueMap() const;

    /**
     * 从ValueMap反序列化游戏状态
     * @param data 包含游戏状态的ValueMap
     * @return 反序列化成功返回true，失败返回false
     */
    bool fromValueMap(const cocos2d::ValueMap& data);

    /**
     * 调试输出游戏状态
     */
    void debugPrintGameState() const;

    /**
     * 检查玩家是否有待处理决策
     * @param playerId 玩家ID
     * @return 有待处理决策返回true，否则返回false
     */
    bool hasPendingDecisions(int playerId) const;

    /**
     * 清理游戏管理器资源
     */
    void cleanup();

    /**
     * 设置游戏状态
     * @param state 新的游戏状态
     */
    void setGameState(GameState state);

    /**
     * 设置回合开始回调
     * @param callback 回调函数
     */
    void setOnTurnStartCallback(std::function<void(int)> callback) { m_onTurnStartCallback = callback; }

    /**
     * 设置回合结束回调
     * @param callback 回调函数
     */
    void setOnTurnEndCallback(std::function<void(int)> callback) { m_onTurnEndCallback = callback; }

    /**
     * 设置胜利回调
     * @param callback 回调函数
     */
    void setOnVictoryCallback(std::function<void(VictoryType, int)> callback) { m_onVictoryCallback = callback; }

private:
    /**
     * 私有构造函数，单例模式
     */
    GameManager();

    /**
     * 析构函数
     */
    ~GameManager();

    /**
     * 前进到下一个玩家
     */
    void advanceToNextPlayer();

    /**
     * 开始新回合
     */
    void beginNewTurn();

    /**
     * 处理AI玩家回合
     * @param aiPlayer AI玩家对象
     */
    void processAITurn(Player* aiPlayer);

    /**
     * 检查玩家是否被击败（目前为空实现）
     */
    void checkDefeatedPlayers() {};

    /**
     * 检查征服胜利条件
     * @return 满足征服胜利条件返回true，否则返回false
     */
    bool checkDominationVictory() const;

    /**
     * 注册首都信息
     * @param playerId 玩家ID
     * @param capitalCity 首都城市
     */
    void registerCapital(int playerId, BaseCity* capitalCity);

    /**
     * 更新首都控制权
     * @param capitalOwnerId 首都所有者ID
     * @param newControllerId 新控制者ID
     */
    void updateCapitalControl(int capitalOwnerId, int newControllerId);

    /**
     * 获取首都信息映射表
     * @return 首都信息映射表
     */
    const std::map<int, CapitalInfo>& getCapitalInfo() const { return m_capitalInfo; }

    /**
     * 获取指定玩家的首都信息
     * @param playerId 玩家ID
     * @return 首都信息结构
     */
    CapitalInfo getCapitalInfoForPlayer(int playerId) const;

    /**
     * 检查科技胜利条件
     * @return 满足科技胜利条件返回true，否则返回false
     */
    bool checkScienceVictory() const;

    /**
     * 通知回合开始
     * @param playerId 玩家ID
     */
    void notifyTurnStart(int playerId);

    /**
     * 通知回合结束
     * @param playerId 玩家ID
     */
    void notifyTurnEnd(int playerId);

    /**
     * 通知胜利
     * @param victoryType 胜利类型
     * @param winnerPlayerId 胜利玩家ID
     */
    void notifyVictory(VictoryType victoryType, int winnerPlayerId);

    /**
     * 延迟结束回合（用于AI回合后等待动画）
     */
    void endTurnWithDelay();

private:
    static GameManager* s_instance;          // 单例实例

    GameState m_gameState;                   // 游戏状态
    GameConfig m_gameConfig;                 // 游戏配置
    GameStats m_gameStats;                   // 游戏统计
    std::map<int, CapitalInfo> m_capitalInfo; // 玩家ID -> 首都信息

    std::vector<Player*> m_players;          // 玩家列表
    std::vector<int> m_playerOrder;          // 玩家顺序
    int m_currentPlayerIndex = 0;            // 当前玩家索引

    // 事件回调函数
    std::function<void(int)> m_onTurnStartCallback;
    std::function<void(int)> m_onTurnEndCallback;
    std::function<void(VictoryType, int)> m_onVictoryCallback;
};

#endif // __GAME_MANAGER_H__