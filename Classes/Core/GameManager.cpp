// GameManager.cpp
#include "GameManager.h"
#include "Player.h"
// 【修复】必须包含 BaseCity 的头文件，因为我们要调用它的方法
#include "../City/BaseCity.h" 
// 【修复】包含 AbstractUnit 以便创建单位
#include "../Units/Base/AbstractUnit.h"
#include <algorithm>
#include "../Scene/GameScene.h" 
#include "../Map/GameMapLayer.h" 
#include <set>                       // 必需：用于防重叠
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

    // ========== 关键：增加引用计数 ==========
    player->retain();  // 防止被自动释放

    m_players.push_back(player);
    m_playerOrder.push_back(player->getPlayerId());

    // 如果是第一个玩家且游戏未开始，设置为当前玩家
    if (m_players.size() == 1 && m_gameState == GameState::INITIALIZING) {
        m_currentPlayerIndex = 0;
    }

    // 更新游戏统计
    m_gameStats.totalPlayers = m_players.size();
    m_gameStats.activePlayers = m_players.size();

    CCLOG("Player %d added to game. Total players: %d (retained)",
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

        // ========== 关键：减少引用计数 ==========
        (*it)->release();  // 对应上面的retain

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

void GameManager::setCurrentPlayer(int index)
{
    m_currentPlayerIndex = 0;
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

    CCLOG("=== Ending turn for Player %d ===", currentPlayer->getPlayerId());

    // 1. 当前玩家回合结束
    currentPlayer->onTurnEnd();
    notifyTurnEnd(currentPlayer->getPlayerId());

    // 2. 检查当前玩家是否被击败（没有城市）
    if (currentPlayer->getState() == Player::PlayerState::DEFEATED) {
        CCLOG("Player %d is defeated, removing from turn order",
            currentPlayer->getPlayerId());
        // 处理玩家失败逻辑...
    }

    // 3. 切换到下一个玩家
    advanceToNextPlayer();

    // 4. 如果一轮结束，增加回合数
    if (m_currentPlayerIndex == 0) {
        beginNewTurn();
    }

    // 5. 新玩家回合开始
    Player* nextPlayer = getCurrentPlayer();
    if (nextPlayer) {
        CCLOG("=== Beginning turn for Player %d ===", nextPlayer->getPlayerId());

        // 5.1 玩家回合开始（这里会计算和更新资源）
        nextPlayer->onTurnBegin();

        // 5.2 通知UI玩家切换
        notifyTurnStart(nextPlayer->getPlayerId());

        // 5.3 发送玩家切换事件（UI会监听这个事件来更新显示）
        ValueMap switchData;
        switchData["player_id"] = nextPlayer->getPlayerId();
        switchData["turn"] = m_gameStats.currentTurn;

        // 【新增】发送资源更新事件
        ValueMap resourceData;
        resourceData["player_id"] = nextPlayer->getPlayerId();
        resourceData["gold"] = nextPlayer->getGold();
        resourceData["science"] = nextPlayer->getSciencePerTurn();
        resourceData["culture"] = nextPlayer->getCulturePerTurn();
        resourceData["turn"] = m_gameStats.currentTurn;
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
            "player_turn_resource_update", &resourceData
        );

        // ==================== 【AI 逻辑触发点】 ====================
        if (!nextPlayer->getIsHuman()) {
            this->processAITurn(nextPlayer);
        }
        // ========================================================
    }

    // 6. 检查胜利条件
    VictoryType victoryType = checkVictoryConditions();
    if (victoryType != VictoryType::NONE) {
        int winnerId = -1;
        for (Player* checked : m_players)
        {
            if (checked->m_vicprogress.hasDominationVictory || checked->m_vicprogress.hasLaunchedSatellite)
                winnerId = checked->getPlayerId();
        }
        if (winnerId != -1) {
            notifyVictory(victoryType, winnerId);
            m_gameState = GameState::GAME_OVER;
        }
    }
}

void GameManager::initializePlayerStartingUnits(cocos2d::Node* parentNode,
    std::function<Hex(int)> getStartHexForPlayerFunc,
    std::function<void(AbstractUnit*)> addToMapFunc,
    std::function<bool(Hex)> checkCityFunc,
    std::function<int(Hex)> getTerrainCostFunc) {

    if (m_players.empty()) {
        CCLOG("No players to initialize starting units for");
        return;
    }

    CCLOG("Initializing starting units for %d players", m_players.size());

    for (auto player : m_players) {
        if (!player || player->getState() != Player::PlayerState::ACTIVE) {
            continue;
        }

        // 为每个玩家创建起始六边形函数
        auto getStartHexFunc = [getStartHexForPlayerFunc, player]() -> Hex {
            if (getStartHexForPlayerFunc) {
                return getStartHexForPlayerFunc(player->getPlayerId());
            }
            // 默认返回地图中心
            return Hex(55, 15);
            };

        // 设置玩家回调函数
        player->setMapCallbacks(getStartHexFunc, addToMapFunc, checkCityFunc, getTerrainCostFunc);

        // 创建起始开拓者
        auto settler = player->createStartingSettler(parentNode, getStartHexFunc, addToMapFunc, checkCityFunc);

        if (settler) {
            CCLOG("Successfully created starting settler for player %d", player->getPlayerId());
        }
        else {
            CCLOG("Failed to create starting settler for player %d", player->getPlayerId());
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

bool GameManager::hasPendingDecisions(int playerId) const {
    Player* player = getPlayer(playerId);
    if (!player) return false;

    // 检查科技树：如果没有当前研究，则有待决事项
    bool techIdle = (player->getCurrentResearchTechId() == -1);

    // 检查文化树：如果没有当前研究，则有待决事项
    bool cultureIdle = (player->getCurrentResearchCivicId() == -1);

    bool productionIdle = false;
    for (auto city : player->getCities())
    {
        if (city->currentProduction == nullptr && city->getSuspendedProductions().empty())
        {
            productionIdle = true;
            break;
        }
    }
    // 注意：这里只检查了“未选择研究”，未来可以扩展其他待决事项（如政策卡、单位指令）
    return (techIdle || cultureIdle || productionIdle);
}

// ==================== AI 逻辑 ====================

// ==================== AI 逻辑完整代码 ====================

void GameManager::processAITurn(Player* aiPlayer) {
    if (!aiPlayer || aiPlayer->getIsHuman()) {
        return;
    }

    CCLOG("=== AI Player %d Turn Start ===", aiPlayer->getPlayerId());

    // 0. 获取必要的环境对象
    auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
    // 如果没有获取到地图层，无法进行可视化操作，直接结束
    if (!gameScene || !gameScene->getMapLayer()) {
        endTurnWithDelay();
        return;
    }

    // 获取 HexLayout，用于单位移动和攻击的坐标转换
    HexLayout* layout = gameScene->getMapLayer()->getLayout();
    if (!layout) {
        endTurnWithDelay();
        return;
    }

    // 1. 寻找攻击目标（人类玩家）
    Player* humanPlayer = nullptr;
    for (auto p : m_players) {
        if (p->getIsHuman()) {
            humanPlayer = p;
            break;
        }
    }

    // 如果找不到人类玩家（比如都死光了），也要继续执行（比如建城），不能直接 return
    // 但战斗逻辑会依赖 humanPlayer

    // ==================== 2. AI 开局建城逻辑 ====================
    // 如果没有城市，优先遍历寻找 Settler 建城
    if (aiPlayer->getCityCount() == 0) {
        // 获取单位列表副本
        std::vector<AbstractUnit*> units = aiPlayer->getUnits();

        for (auto unit : units) {
            // 找到移民单位
            if (unit && unit->canFoundCity()) {
                Hex unitPos = unit->getGridPos();

                // 简单的合法性检查（可选）
                bool canSettle = true;
                if (aiPlayer->m_checkCityFunc) {
                    if (aiPlayer->m_checkCityFunc(unitPos)) canSettle = false;
                }

                if (canSettle) {
                    CCLOG("AI Player %d founding Capital at (%d, %d)", aiPlayer->getPlayerId(), unitPos.q, unitPos.r);

                    // 创建城市
                    std::string cityName = "City " + std::to_string(aiPlayer->getPlayerId());
                    BaseCity* newCity = BaseCity::create(aiPlayer->getPlayerId(), unitPos, cityName);

                    if (newCity) {
                        // 可视化：设置位置并添加到地图层
                        Vec2 pixelPos = layout->hexToPixel(unitPos);
                        newCity->setPosition(pixelPos);
                        gameScene->getMapLayer()->addChild(newCity, 10); // Z-Order 10

                        // 逻辑注册
                        this->registerCapital(aiPlayer->getPlayerId(), newCity);
                        aiPlayer->addCity(newCity);

                        // 移除移民
                        aiPlayer->removeUnit(unit);
                        unit->removeFromParent();

                        // 建城后跳出单位循环，防止指针失效
                        break;
                    }
                }
            }
        }
    }

    // ==================== 3. AI 单位战斗与移动逻辑 ====================

    // --- 3.1 预备防重叠集合 ---
    // occupiedOrReservedHexes 用于记录本回合某个格子是否“有人了”或者“即将有人去”
    std::set<Hex> occupiedOrReservedHexes;

    // 将所有活着的单位和城市位置加入占用列表
    for (auto p : m_players) {
        for (auto u : p->getUnits()) {
            if (u->isAlive()) occupiedOrReservedHexes.insert(u->getGridPos());
        }
        for (auto c : p->getCities()) {
            occupiedOrReservedHexes.insert(c->gridPos);
        }
    }

    // --- 3.2 预备死亡名单 ---
    // 防止多个 AI 单位对同一个必死的敌人进行“鞭尸”
    std::set<AbstractUnit*> dyingUnits;

    // --- 3.3 遍历 AI 单位执行行动 ---
    std::vector<AbstractUnit*> myUnits = aiPlayer->getUnits();

    for (auto unit : myUnits) {
        // 跳过无效单位、死单位、和移民（移民逻辑上面处理过了，或者让它呆在原地）
        if (!unit || !unit->isAlive() || unit->canFoundCity()) {
            if (unit && unit->isAlive()) occupiedOrReservedHexes.insert(unit->getGridPos());
            continue;
        }

        Hex currentPos = unit->getGridPos();
        // 既然轮到我动了，先把我的当前位置从占用表中移除
        occupiedOrReservedHexes.erase(currentPos);

        // 如果没有人类玩家，AI 就呆在原地
        if (!humanPlayer) {
            occupiedOrReservedHexes.insert(currentPos);
            continue;
        }

        // A. 寻找最近的且活着的敌人
        AbstractUnit* targetEnemy = nullptr;
        int minDistance = 9999;

        for (auto enemyUnit : humanPlayer->getUnits()) {
            // 忽略已死或即将被打死的敌人
            if (!enemyUnit->isAlive() || dyingUnits.count(enemyUnit)) continue;

            int dist = currentPos.distance(enemyUnit->getGridPos());
            if (dist < minDistance) {
                minDistance = dist;
                targetEnemy = enemyUnit;
            }
        }

        // B. 决策：攻击 还是 移动
        // 注意：这里假设 AbstractUnit 有 getAttackRange() 接口，默认近战为1
        int attackRange = unit->getAttackRange();
        if (attackRange <= 0) attackRange = 1; // 保底

        // 情况 1: 敌人在射程内 -> 发动攻击
        if (targetEnemy && minDistance <= attackRange) {
            CCLOG("AI Unit %s ATTACK -> %s", unit->getUnitName().c_str(), targetEnemy->getUnitName().c_str());

            // 执行攻击 (传入 layout 以便播放动画)
            unit->attack(targetEnemy, layout);

            // 伤害预估：防止鞭尸
            // 虽然伤害是延迟结算的，但我们假定本次攻击会造成面板伤害
            int predictedDamage = unit->getCombatPower();
            if (targetEnemy->getCurrentHp() - predictedDamage <= 0) {
                dyingUnits.insert(targetEnemy);
            }

            // 攻击后留在原地
            occupiedOrReservedHexes.insert(currentPos);
        }
        // 情况 2: 敌人在远处 -> 移动接近
        else if (targetEnemy) {
            Hex targetPos = targetEnemy->getGridPos();
            Hex bestMove = currentPos;
            int bestDist = minDistance;

            // 遍历周围 6 个格子寻找最佳移动点
            for (int i = 0; i < 6; i++) {
                Hex neighbor = currentPos.getNeighbor(i);

                // 【核心】检查格子是否被占用或预定
                if (occupiedOrReservedHexes.count(neighbor)) continue;

                // 可选：检查地形消耗 (如果你的 getTerrainCostFunc 可用)
                // if (aiPlayer->m_getTerrainCostFunc && aiPlayer->m_getTerrainCostFunc(neighbor) < 0) continue;

                // 贪心算法：找离敌人最近的格子
                int dist = neighbor.distance(targetPos);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestMove = neighbor;
                }
            }

            // 如果找到了合法的移动目标
            if (bestMove != currentPos) {
                CCLOG("AI Unit %s MOVE -> (%d, %d)", unit->getUnitName().c_str(), bestMove.q, bestMove.r);

                // 计算消耗 (简单逻辑：1格消耗1点，或者直接取距离)
                int cost = 1;

                // 执行移动
                unit->moveTo(bestMove, layout, cost);

                // 【核心】锁定新位置，防止后续单位重叠
                occupiedOrReservedHexes.insert(bestMove);
            }
            else {
                // 无路可走，留在原地
                occupiedOrReservedHexes.insert(currentPos);
            }
        }
        // 情况 3: 没有敌人 (虽然前面check了humanPlayer，但可能所有单位都死了)
        else {
            occupiedOrReservedHexes.insert(currentPos);
        }
    }

    // ==================== 4. AI 城市生产逻辑 ====================
    // 使用 ProductionProgram 避免 AbstractUnit 堆内存崩溃问题

    const std::vector<BaseCity*>& cities = aiPlayer->getCities();
    for (BaseCity* city : cities) {
        if (!city) continue;

        // 如果城市闲置
        if (city->getCurrentProduction() == nullptr) {
            CCLOG("AI: City %s starting production of Warrior", city->getCityName().c_str());

            // 创建生产项目 (注意参数匹配你的 ProductionProgram 构造函数)
            // 参数: Type, Name, Pos, Cost, canPurchase, purchaseCost
            ProductionProgram* warriorProd = new ProductionProgram(
                ProductionProgram::ProductionType::UNIT,
                "Warrior",
                Hex(), 0, true, 200 // 这里的cost和purchaseCost最好跟配置表一致
            );

            city->addNewProduction(warriorProd);
        }
    }

    // ==================== 5. 结束回合 ====================
    endTurnWithDelay();
}

// 辅助函数：延迟结束回合，让动画飞一会儿
void GameManager::endTurnWithDelay() {
    Director::getInstance()->getScheduler()->schedule(
        [this](float dt) {
            if (this->m_gameState == GameState::PLAYING) {
                this->endTurn();
            }
        },
        this,           // target
        0,              // interval
        0,              // repeat
        1.5f,           // delay (1.5秒，给移动和攻击动画留时间)
        false,          // paused
        "ai_turn_end"   // key
    );
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
    if (m_gameState != GameState::PLAYING) {  // 添加状态检查
        return false;
    }

    for (auto player : m_players) {
        if (player && player->checkScienceVictory()) {
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

void GameManager::setGameState(GameState state) {
    m_gameState = state;
    CCLOG("Game state changed to: %d", (int)state);
}