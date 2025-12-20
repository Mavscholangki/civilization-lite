// GameManager.cpp
#include "GameManager.h"
#include "Player.h"
#include <algorithm>

USING_NS_CC;

GameManager* GameManager::s_instance = nullptr;

// ==================== 单例模式 ====================

GameManager* GameManager::getInstance() {
    if (!s_instance) {
        s_instance = new GameManager();
    }
    return s_instance;
}

GameManager::GameManager()
    : m_gameState(GameState::INITIALIZING) {
    CCLOG("GameManager created");
}

GameManager::~GameManager() {
    cleanup();
    CCLOG("GameManager destroyed");
}

// ==================== 游戏初始化 ====================

bool GameManager::initialize(const GameConfig& config) {
    m_gameConfig = config;
    m_gameState = GameState::INITIALIZING;
    m_gameStats = GameStats();
    m_currentPlayerIndex = 0;

    m_players.clear();
    m_playerOrder.clear();

    CCLOG("GameManager initialized");
    return true;
}

// ==================== 玩家管理 ====================

void GameManager::addPlayer(Player* player) {
    if (!player) {
        CCLOG("Cannot add null player");
        return;
    }

    // 检查是否已存在相同ID的玩家
    for (auto existingPlayer : m_players) {
        if (existingPlayer->getPlayerId() == player->getPlayerId()) {
            CCLOG("Player with ID %d already exists", player->getPlayerId());
            return;
        }
    }

    m_players.push_back(player);
    m_playerOrder.push_back(player->getPlayerId());

    // 如果是第一个玩家且游戏未开始，设置为当前玩家
    if (m_players.size() == 1 && m_gameState == GameState::INITIALIZING) {
        m_currentPlayerIndex = 0;
    }

    // 更新游戏统计
    m_gameStats.totalPlayers = m_players.size();
    m_gameStats.activePlayers = m_players.size(); // 初始所有玩家都活跃

    CCLOG("Player %d added to game. Total players: %d",
        player->getPlayerId(), m_players.size());
}

void GameManager::removePlayer(int playerId) {
    auto it = std::find_if(m_players.begin(), m_players.end(),
        [playerId](Player* player) {
            return player->getPlayerId() == playerId;
        });

    if (it != m_players.end()) {
        // 从玩家顺序中移除
        auto orderIt = std::find(m_playerOrder.begin(), m_playerOrder.end(), playerId);
        if (orderIt != m_playerOrder.end()) {
            m_playerOrder.erase(orderIt);

            // 调整当前玩家索引
            if (m_currentPlayerIndex >= m_playerOrder.size()) {
                m_currentPlayerIndex = 0;
            }
        }

        // 从玩家列表中移除
        m_players.erase(it);

        // 更新游戏统计
        m_gameStats.totalPlayers = m_players.size();

        CCLOG("Player %d removed from game. Remaining players: %d",
            playerId, m_players.size());
    }
    else {
        CCLOG("Player %d not found in game", playerId);
    }
}

Player* GameManager::getPlayer(int playerId) const {
    for (auto player : m_players) {
        if (player->getPlayerId() == playerId) {
            return player;
        }
    }
    return nullptr;
}

Player* GameManager::getCurrentPlayer() const {
    if (m_playerOrder.empty() || m_currentPlayerIndex >= m_playerOrder.size()) {
        return nullptr;
    }

    int currentPlayerId = m_playerOrder[m_currentPlayerIndex];
    return getPlayer(currentPlayerId);
}

Player* GameManager::getNextPlayer() const {
    if (m_playerOrder.empty()) {
        return nullptr;
    }

    int nextIndex = (m_currentPlayerIndex + 1) % m_playerOrder.size();
    int nextPlayerId = m_playerOrder[nextIndex];
    return getPlayer(nextPlayerId);
}

// ==================== 回合管理 ====================

void GameManager::endTurn() {
    Player* currentPlayer = getCurrentPlayer();
    if (!currentPlayer) {
        CCLOG("No current player to end turn");
        return;
    }

    // 通知当前玩家回合结束
    currentPlayer->onTurnEnd();
    notifyTurnEnd(currentPlayer->getPlayerId());

    // 切换到下一个玩家
    advanceToNextPlayer();

    // 如果一轮结束，开始新回合
    if (m_currentPlayerIndex == 0) {
        beginNewTurn();
    }

    // 开始下一个玩家的回合
    Player* nextPlayer = getCurrentPlayer();
    if (nextPlayer) {
        nextPlayer->onTurnBegin();
        notifyTurnStart(nextPlayer->getPlayerId());

        // 如果是AI玩家，处理AI回合
        if (!nextPlayer->getIsHuman()) {
            processAITurn(nextPlayer);
        }
    }

    // 检查胜利条件
    VictoryType victoryType = checkVictoryConditions();
    if (victoryType != VictoryType::NONE) {
        int winnerId = -1;

        // 根据胜利类型确定胜利者
        if (victoryType == VictoryType::SCIENCE) {
            // 找到达成科技胜利的玩家
            for (auto player : m_players) {
                if (player && player->checkScienceVictory()) {
                    winnerId = player->getPlayerId();
                    break;
                }
            }
        }
        else if (victoryType == VictoryType::DOMINATION) {
            // 找到控制所有首都的玩家
            for (auto player : m_players) {
                if (player && player->checkDominationVictory()) {
                    winnerId = player->getPlayerId();
                    break;
                }
            }
        }

        if (winnerId != -1) {
            notifyVictory(victoryType, winnerId);
            m_gameState = GameState::GAME_OVER;
            m_gameStats.victoryType = victoryType;
            m_gameStats.victoryPlayerId = winnerId;
        }
    }
}

void GameManager::advanceToNextPlayer() {
    if (m_playerOrder.empty()) {
        return;
    }

    // 找到下一个活跃玩家
    int attempts = 0;
    int nextIndex;

    do {
        nextIndex = (m_currentPlayerIndex + 1) % m_playerOrder.size();
        attempts++;

        if (attempts > m_playerOrder.size()) {
            // 所有玩家都不活跃
            CCLOG("All players are inactive or defeated");
            m_gameState = GameState::GAME_OVER;
            return;
        }

        Player* nextPlayer = getPlayer(m_playerOrder[nextIndex]);
        if (nextPlayer && nextPlayer->getState() == Player::PlayerState::ACTIVE) {
            m_currentPlayerIndex = nextIndex;
            break;
        }

        m_currentPlayerIndex = nextIndex;
    } while (true);
}

void GameManager::beginNewTurn() {
    m_gameStats.currentTurn++;
    CCLOG("=== Beginning Turn %d ===", m_gameStats.currentTurn);

    // 检查回合限制
    if (m_gameConfig.maxTurns > 0 && m_gameStats.currentTurn > m_gameConfig.maxTurns) {
        CCLOG("Maximum turns reached (%d). Game over!", m_gameConfig.maxTurns);
        m_gameState = GameState::GAME_OVER;
    }
}

void GameManager::processAITurn(Player* aiPlayer) {
    if (!aiPlayer || aiPlayer->getIsHuman()) {
        return;
    }

    CCLOG("Processing AI turn for player %d", aiPlayer->getPlayerId());

    // TODO: 简化AI逻辑
    // 1. 如果城市少于2个，生产移民者
    // 2. 如果有空闲建造者，让他们改进地块
    // 3. 如果科技未研究，继续研究
    // 4. 移动军事单位巡逻或攻击附近敌人

    CCLOG("AI player %d turn processed (simplified)", aiPlayer->getPlayerId());
}

// ==================== 胜利条件检查 ====================

VictoryType GameManager::checkVictoryConditions() {
    if (m_gameState != GameState::PLAYING) {
        return VictoryType::NONE;
    }

    // 检查科技胜利
    if (m_gameConfig.enableScienceVictory) {
        for (auto player : m_players) {
            if (player && player->checkScienceVictory()) {
                CCLOG("Player %d achieves science victory!", player->getPlayerId());
                return VictoryType::SCIENCE;
            }
        }
    }

    // 检查统治胜利
    if (m_gameConfig.enableDominationVictory && checkDominationVictory()) {
        return VictoryType::DOMINATION;
    }

    return VictoryType::NONE;
}

bool GameManager::checkScienceVictory() const {
    for (auto player : m_players) {
        if (player && player->checkScienceVictory()) {
            CCLOG("Player %d achieves science victory!", player->getPlayerId());
            return true;
        }
    }
    return false;
}

bool GameManager::checkDominationVictory() const {
    if (m_gameState != GameState::PLAYING || !m_gameConfig.enableDominationVictory) {
        return false;
    }

    // 找出所有活跃玩家的首都信息
    std::map<int, int> controllerCount; // 控制者ID -> 控制的首都数量

    for (const auto& pair : m_capitalInfo) {
        const CapitalInfo& info = pair.second;

        // 只考虑活跃玩家的首都
        Player* owner = getPlayer(info.ownerPlayerId);
        if (!owner || owner->getState() != Player::PlayerState::ACTIVE) {
            continue;
        }

        int controllerId = info.currentControllerId;
        controllerCount[controllerId]++;
    }

    // 检查是否有玩家控制了所有首都
    int totalActiveCapitals = 0;
    for (const auto& pair : m_capitalInfo) {
        const CapitalInfo& info = pair.second;
        Player* owner = getPlayer(info.ownerPlayerId);
        if (owner && owner->getState() == Player::PlayerState::ACTIVE) {
            totalActiveCapitals++;
        }
    }

    for (const auto& pair : controllerCount) {
        int controllerId = pair.first;
        int controlledCapitals = pair.second;

        if (controlledCapitals == totalActiveCapitals && totalActiveCapitals > 0) {
            // 该玩家控制了所有活跃玩家的首都！
            Player* winner = getPlayer(controllerId);
            if (winner) {
                CCLOG("Player %d controls all %d active capitals! Domination victory!",
                    controllerId, totalActiveCapitals);

                // 标记胜利玩家
                winner->m_vicprogress.hasDominationVictory = true;

                return true;
            }
        }
    }

    return false;
}

void GameManager::registerCapital(int playerId, BaseCity* capitalCity) {
    if (!capitalCity) {
        CCLOG("Cannot register null capital for player %d", playerId);
        return;
    }

    CapitalInfo info(playerId, capitalCity);
    m_capitalInfo[playerId] = info;
}

void GameManager::updateCapitalControl(int capitalOwnerId, int newControllerId) {
    auto it = m_capitalInfo.find(capitalOwnerId);
    if (it != m_capitalInfo.end()) {
        CapitalInfo& info = it->second;
        int oldController = info.currentControllerId;

        info.currentControllerId = newControllerId;
        info.isCaptured = (newControllerId != capitalOwnerId);

        // 更新玩家控制的首都列表
        Player* oldControllerPlayer = getPlayer(oldController);
        Player* newControllerPlayer = getPlayer(newControllerId);

        if (oldControllerPlayer && oldController != newControllerId) {
            oldControllerPlayer->removeControlledCapital(capitalOwnerId);
        }

        if (newControllerPlayer) {
            newControllerPlayer->addControlledCapital(capitalOwnerId);
        }

        CCLOG("Capital of player %d is now controlled by player %d",
            capitalOwnerId, newControllerId);

        // 检查是否达成统治胜利
        checkDominationVictory();
    }
    else {
        CCLOG("Warning: Cannot find capital info for player %d", capitalOwnerId);
    }
}

CapitalInfo GameManager::getCapitalInfoForPlayer(int playerId) const {
    auto it = m_capitalInfo.find(playerId);
    if (it != m_capitalInfo.end()) {
        return it->second;
    }
    return CapitalInfo();
}

// ==================== 事件通知 ====================

void GameManager::notifyTurnStart(int playerId) {
    if (m_onTurnStartCallback) {
        m_onTurnStartCallback(playerId);
    }
}

void GameManager::notifyTurnEnd(int playerId) {
    if (m_onTurnEndCallback) {
        m_onTurnEndCallback(playerId);
    }
}

void GameManager::notifyVictory(VictoryType victoryType, int winnerPlayerId) {
    if (m_onVictoryCallback) {
        m_onVictoryCallback(victoryType, winnerPlayerId);
    }

    CCLOG("=== VICTORY! ===");
    CCLOG("Type: %s",
        victoryType == VictoryType::SCIENCE ? "Science" :
        victoryType == VictoryType::DOMINATION ? "Domination" : "Unknown");
    CCLOG("Winner: Player %d", winnerPlayerId);
}

// ==================== 序列化 ====================

ValueMap GameManager::toValueMap() const {
    ValueMap data;

    data["gameState"] = (int)m_gameState;
    data["currentTurn"] = m_gameStats.currentTurn;
    data["victoryType"] = (int)m_gameStats.victoryType;
    data["victoryPlayerId"] = m_gameStats.victoryPlayerId;
    data["currentPlayerIndex"] = m_currentPlayerIndex;

    // 玩家顺序
    ValueVector playerOrderVec;
    for (int playerId : m_playerOrder) {
        playerOrderVec.push_back(Value(playerId));
    }
    data["playerOrder"] = playerOrderVec;

    // 游戏配置
    ValueMap configData;
    configData["maxTurns"] = m_gameConfig.maxTurns;
    configData["enableScienceVictory"] = m_gameConfig.enableScienceVictory;
    configData["enableDominationVictory"] = m_gameConfig.enableDominationVictory;
    data["gameConfig"] = configData;

    return data;
}

bool GameManager::fromValueMap(const ValueMap& data) {
    if (data.find("gameState") != data.end()) {
        m_gameState = static_cast<GameState>(data.at("gameState").asInt());
    }

    if (data.find("currentTurn") != data.end()) {
        m_gameStats.currentTurn = data.at("currentTurn").asInt();
    }

    if (data.find("victoryType") != data.end()) {
        m_gameStats.victoryType = static_cast<VictoryType>(data.at("victoryType").asInt());
    }

    if (data.find("victoryPlayerId") != data.end()) {
        m_gameStats.victoryPlayerId = data.at("victoryPlayerId").asInt();
    }

    if (data.find("currentPlayerIndex") != data.end()) {
        m_currentPlayerIndex = data.at("currentPlayerIndex").asInt();
    }

    // 恢复玩家顺序
    if (data.find("playerOrder") != data.end()) {
        m_playerOrder.clear();
        const ValueVector& playerOrderVec = data.at("playerOrder").asValueVector();
        for (const Value& playerIdVal : playerOrderVec) {
            m_playerOrder.push_back(playerIdVal.asInt());
        }
    }

    // 恢复游戏配置
    if (data.find("gameConfig") != data.end()) {
        const ValueMap& configData = data.at("gameConfig").asValueMap();

        if (configData.find("maxTurns") != configData.end())
            m_gameConfig.maxTurns = configData.at("maxTurns").asInt();
        if (configData.find("enableScienceVictory") != configData.end())
            m_gameConfig.enableScienceVictory = configData.at("enableScienceVictory").asBool();
        if (configData.find("enableDominationVictory") != configData.end())
            m_gameConfig.enableDominationVictory = configData.at("enableDominationVictory").asBool();
    }

    CCLOG("GameManager loaded from saved data");
    debugPrintGameState();

    return true;
}

// ==================== 调试和清理 ====================

void GameManager::debugPrintGameState() const {
    CCLOG("=== GameManager State ===");
    CCLOG("Game State: %d", (int)m_gameState);
    CCLOG("Current Turn: %d", m_gameStats.currentTurn);
    CCLOG("Current Player Index: %d", m_currentPlayerIndex);
    CCLOG("Total Players: %d", m_players.size());
    CCLOG("Victory Type: %d", (int)m_gameStats.victoryType);
    CCLOG("Victory Player ID: %d", m_gameStats.victoryPlayerId);

    CCLOG("Player Order:");
    for (size_t i = 0; i < m_playerOrder.size(); i++) {
        Player* player = getPlayer(m_playerOrder[i]);
        if (player) {
            CCLOG("  [%zu] Player %d (%s) - Cities: %d, Units: %d, State: %d",
                i, player->getPlayerId(), player->getPlayerName().c_str(),
                player->getCityCount(), player->getUnitCount(),
                (int)player->getState());
        }
    }

    CCLOG("Capital Info:");
    for (const auto& pair : m_capitalInfo) {
        const CapitalInfo& info = pair.second;
        CCLOG("  Owner Player %d: Controller: %d, Captured: %s",
            info.ownerPlayerId, info.currentControllerId,
            info.isCaptured ? "Yes" : "No");
    }

    // 检查当前控制状态
    CCLOG("Capital Control Status:");
    for (auto player : m_players) {
        if (player) {
            std::vector<int> controlled = player->getControlledCapitals();
            CCLOG("  Player %d controls %zu capitals: ",
                player->getPlayerId(), controlled.size());
            for (int capitalOwner : controlled) {
                CCLOG("    - Capital of player %d", capitalOwner);
            }
        }
    }

    if (m_gameState == GameState::GAME_OVER && m_gameStats.victoryType != VictoryType::NONE) {
        CCLOG("Game Over! Winner: Player %d by %s victory",
            m_gameStats.victoryPlayerId,
            m_gameStats.victoryType == VictoryType::SCIENCE ? "Science" : "Domination");
    }

    CCLOG("=== End GameManager State ===");
}

void GameManager::cleanup() {
    CCLOG("GameManager cleanup started");

    // 注意：不删除玩家对象，由创建者管理
    m_players.clear();
    m_playerOrder.clear();

    m_gameState = GameState::INITIALIZING;
    m_currentPlayerIndex = 0;
    m_gameStats = GameStats();

    // 重置回调
    m_onTurnStartCallback = nullptr;
    m_onTurnEndCallback = nullptr;
    m_onVictoryCallback = nullptr;

    CCLOG("GameManager cleanup completed");
}


