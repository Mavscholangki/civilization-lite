// GameManager.h
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "cocos2d.h"
#include "Player.h"
#include <vector>
#include <map>

const int MAX_AI_PLAYERS = 6;  // 最大AI玩家数量，方便调试
const int MIN_AI_PLAYERS = 1;  // 最少AI玩家数量

// 游戏状态
enum class GameState {
    INITIALIZING,      // 初始化中
    PLAYING,           // 进行中
    PAUSED,            // 暂停
    GAME_OVER          // 游戏结束
};

// 胜利类型
enum class VictoryType {
    NONE,              // 无胜利
    SCIENCE,           // 科技胜利
    DOMINATION         // 统治胜利
};

// 游戏配置
struct GameConfig {
    int maxTurns = 500;               // 最大回合数
    bool enableScienceVictory = true; // 启用科技胜利
    bool enableDominationVictory = true; // 启用统治胜利

    GameConfig() {}
};

// 游戏统计
struct GameStats {
    int currentTurn = 1;           // 当前回合
    int totalPlayers = 0;          // 总玩家数
    int activePlayers = 0;         // 活跃玩家数
    VictoryType victoryType = VictoryType::NONE; // 胜利类型
    int victoryPlayerId = -1;      // 胜利玩家ID

    GameStats() {}
};

// 首都信息结构体
struct CapitalInfo {
    int ownerPlayerId;           // 原始所有者玩家ID
    int currentControllerId;     // 当前控制者玩家ID（可能被占领）
    BaseCity* capitalCity;       // 首都城市指针
    bool isCaptured;             // 是否被占领

    CapitalInfo()
        : ownerPlayerId(-1), currentControllerId(-1),
        capitalCity(nullptr), isCaptured(false) {
    }

    CapitalInfo(int ownerId, BaseCity* city)
        : ownerPlayerId(ownerId), currentControllerId(ownerId),
        capitalCity(city), isCaptured(false) {
    }
};

class GameManager {
public:
    // 单例模式获取实例
    static GameManager* getInstance();

    // 初始化游戏
    bool initialize(const GameConfig& config);

    // 添加玩家
    void addPlayer(Player* player);

    // 移除玩家
    void removePlayer(int playerId);

    // 获取玩家
    Player* getPlayer(int playerId) const;

    // 获取所有玩家
    const std::vector<Player*>& getAllPlayers() const { return m_players; }

    // 获取当前玩家
    Player* getCurrentPlayer() const;

    // 设置当前玩家
    void setCurrentPlayer(int index);

    // 获取下一个玩家
    Player* getNextPlayer() const;

    // 结束当前玩家回合
    void endTurn();

    // 初始化玩家起始单位
    void initializePlayerStartingUnits(cocos2d::Node* parentNode,
        std::function<Hex(int)> getStartHexForPlayerFunc,
        std::function<void(AbstractUnit*)> addToMapFunc,
        std::function<bool(Hex)> checkCityFunc,
        std::function<int(Hex)> getTerrainCostFunc);

    // 检查胜利条件
    VictoryType checkVictoryConditions();

    // 获取游戏状态
    GameState getGameState() const { return m_gameState; }

    // 获取游戏统计
    const GameStats& getGameStats() const { return m_gameStats; }

    // 获取游戏配置
    const GameConfig& getGameConfig() const { return m_gameConfig; }

    // 序列化游戏状态
    cocos2d::ValueMap toValueMap() const;

    // 反序列化游戏状态
    bool fromValueMap(const cocos2d::ValueMap& data);

    // 调试信息
    void debugPrintGameState() const;

    // 检查本回合是否还有未完成决策
    bool hasPendingDecisions(int playerId) const;

    // 清理资源
    void cleanup();

    void setGameState(GameState state);

    // 事件回调设置
    void setOnTurnStartCallback(std::function<void(int)> callback) { m_onTurnStartCallback = callback; }
    void setOnTurnEndCallback(std::function<void(int)> callback) { m_onTurnEndCallback = callback; }
    void setOnVictoryCallback(std::function<void(VictoryType, int)> callback) { m_onVictoryCallback = callback; }

private:
    GameManager();
    ~GameManager();

    // 切换到下一个玩家
    void advanceToNextPlayer();

    // 开始新回合
    void beginNewTurn();

    // 处理AI玩家回合
    void processAITurn(Player* aiPlayer);

    // 检查玩家是否被击败
    void checkDefeatedPlayers();

    // 检查统治胜利
    bool checkDominationVictory() const;
    void registerCapital(int playerId, BaseCity* capitalCity);
    void updateCapitalControl(int capitalOwnerId, int newControllerId);
    const std::map<int, CapitalInfo>& getCapitalInfo() const { return m_capitalInfo; }
    CapitalInfo getCapitalInfoForPlayer(int playerId) const;

    // 检查科技胜利
    bool checkScienceVictory() const;

    // 通知事件
    void notifyTurnStart(int playerId);
    void notifyTurnEnd(int playerId);
    void notifyVictory(VictoryType victoryType, int winnerPlayerId);

private:
    static GameManager* s_instance;

    GameState m_gameState;
    GameConfig m_gameConfig;
    GameStats m_gameStats;
    std::map<int, CapitalInfo> m_capitalInfo; // 玩家ID -> 首都信息

    std::vector<Player*> m_players;      // 玩家列表
    std::vector<int> m_playerOrder;      // 玩家顺序
    int m_currentPlayerIndex = 0;        // 当前玩家索引

    // 事件回调
    std::function<void(int)> m_onTurnStartCallback;
    std::function<void(int)> m_onTurnEndCallback;
    std::function<void(VictoryType, int)> m_onVictoryCallback;
};

#endif // __GAME_MANAGER_H__