#include "GameScene.h"
#include "../Map/GameMapLayer.h"
#include "../UI/HUDLayer.h"
#include "../UI/CityProductionPanel.h"
#include "../Core/GameManager.h"
#include "../Core/Player.h"
#include "SelectionScene.h"

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    // 1. 初始化游戏管理器
    m_gameManager = GameManager::getInstance();

    GameConfig config;
    config.maxTurns = 300;
    config.enableScienceVictory = true;
    config.enableDominationVictory = true;

    if (!m_gameManager->initialize(config)) {
        CCLOG("Failed to initialize GameManager");
        return false;
    }

    // 2. 创建人类玩家
    CivilizationType playerCiv = CivilizationSelectionScene::getSelectedCivilization();
    m_humanPlayer = Player::create(0, playerCiv);
    if (m_humanPlayer) {
        m_humanPlayer->setIsHuman(true);
        m_humanPlayer->setPlayerName("Human Player");
        m_gameManager->addPlayer(m_humanPlayer);
        CCLOG("Human player created with civilization: %d", static_cast<int>(playerCiv));
    }

    // 3. 创建AI玩家 - 使用选择界面中的设置
    std::vector<AIPlayerSetting> aiSettings = CivilizationSelectionScene::getAIPlayerSettings();
    int aiPlayerCount = CivilizationSelectionScene::getAIPlayerCount();

    CCLOG("Creating %d AI players", aiPlayerCount);

    // 如果设置数量不等于显示数量，重新生成
    if (aiSettings.size() != aiPlayerCount) {
        aiSettings.clear();
        std::vector<CivilizationType> availableCivs;

        // 获取可用文明（排除玩家文明）
        if (playerCiv != CivilizationType::GERMANY) availableCivs.push_back(CivilizationType::GERMANY);
        if (playerCiv != CivilizationType::RUSSIA) availableCivs.push_back(CivilizationType::RUSSIA);
        if (playerCiv != CivilizationType::CHINA) availableCivs.push_back(CivilizationType::CHINA);

        // 如果可用文明不够，允许重复
        for (int i = 0; i < aiPlayerCount; i++) {
            CivilizationType civ;
            if (i < availableCivs.size()) {
                civ = availableCivs[i];
            }
            else {
                // 使用默认文明
                civ = CivilizationType::GERMANY;
            }

            std::string aiName = "AI Player " + std::to_string(i + 1);
            aiSettings.push_back(AIPlayerSetting(i + 1, civ, aiName));
        }
    }

    // 创建AI玩家
    for (int i = 0; i < aiPlayerCount; i++) {
        Player* aiPlayer = Player::create(i + 1, aiSettings[i].civilization);
        if (aiPlayer) {
            aiPlayer->setIsHuman(false);
            aiPlayer->setPlayerName(aiSettings[i].name);
            m_gameManager->addPlayer(aiPlayer);
            CCLOG("AI player %d created with civilization: %d",
                i + 1, static_cast<int>(aiSettings[i].civilization));
        }
    }

    // 4. 创建地图层
    _mapLayer = GameMapLayer::create();
    if (!_mapLayer) return false;
    this->addChild(_mapLayer, 0);

    // 5. 创建HUD层
    _hudLayer = HUDLayer::create();
    if (!_hudLayer) return false;
    this->addChild(_hudLayer, 100);

    // 6. 创建生产面板层
    _productionPanelLayer = CityProductionPanel::create();
    _productionPanelLayer->setVisible(false);
    this->addChild(_productionPanelLayer, 120);

    // 7. 设置HUD层使用人类玩家的系统实例
    if (m_humanPlayer) {
        _hudLayer->setTechTree(m_humanPlayer->getTechTree());
        _hudLayer->setCultureTree(m_humanPlayer->getCultureTree());
        _hudLayer->setPolicyManager(m_humanPlayer->getPolicyManager());
        CCLOG("HUDLayer set to use human player's system instances");
    }

    // 现在初始化这些系统只需调用Player的初始化方法
    initTechTree();
    initCultureTree();
    initPolicySystem();

    // 8. 设置回调
    setupCallbacks();

    // 9. 设置游戏管理器回调
    m_gameManager->setOnTurnStartCallback([this](int playerId) {
        Player* currentPlayer = m_gameManager->getCurrentPlayer();
        if (currentPlayer && currentPlayer->getIsHuman()) {
            CCLOG("Human player turn started");
            // 如果是人类玩家回合，确保HUD绑定的是当前玩家
            _hudLayer->setTechTree(currentPlayer->getTechTree());
            _hudLayer->setCultureTree(currentPlayer->getCultureTree());
            _hudLayer->setPolicyManager(currentPlayer->getPolicyManager());
        }
        });

    m_gameManager->setOnVictoryCallback([this](VictoryType victoryType, int winnerPlayerId) {
        // 胜利处理...
        });

    CCLOG("GameScene initialized successfully with %d total players",
        m_gameManager->getAllPlayers().size());

    return true;
}

// 这些函数现在只需要初始化玩家的系统
void GameScene::initTechTree() {
    // 现在由Player管理，这里不需要做任何事
    CCLOG("Note: TechTree is now managed by Player class");
}

void GameScene::initCultureTree() {
    // 现在由Player管理
    CCLOG("Note: CultureTree is now managed by Player class");
}

void GameScene::initPolicySystem() {
    // 现在由Player管理
    CCLOG("Note: Policy system is now managed by Player class");
}

void GameScene::setupCallbacks() {
    // 地图层选中单位 -> 更新HUD显示
    _mapLayer->setOnUnitSelectedCallback([this](AbstractUnit* unit) {
        if (unit) {
            _hudLayer->showUnitInfo(unit);
        }
        else {
            _hudLayer->hideUnitInfo();
        }
        });

    // HUD建城按钮 -> 地图层建城动画
    _hudLayer->setBuildCityCallback([this]() {
        _mapLayer->onBuildCityAction();
        });

    // 下一回合按钮回调 - 直接调用GameManager
    _hudLayer->setNextTurnCallback([this]() {
        Player* currentPlayer = m_gameManager->getCurrentPlayer();
        if (!currentPlayer || !currentPlayer->getIsHuman()) {
            //return; // 不是人类玩家回合，理论上按钮应不可点，此处做安全保护
        }

        // --- 新增：快捷研究逻辑 ---
        if (currentPlayer->getIsHuman() && m_gameManager->hasPendingDecisions(currentPlayer->getPlayerId())) {
            // 有待决事项，优先处理
            if (currentPlayer->getCurrentResearchTechId() == -1) {
                CCLOG("Tech tree idle. Opening tech panel...");
                _hudLayer->openTechTree(); // 打开科技树面板
                return; // 不进入下一回合
            }
            else if (currentPlayer->getCurrentResearchCivicId() == -1) {
                CCLOG("Culture tree idle. Opening culture panel...");
                _hudLayer->openCultureTree(); // 打开文化树面板
                return; // 不进入下一回合
            }
        }
        // --- 结束新增 ---

        // 没有待决事项，正常结束回合
        m_gameManager->endTurn();
        });

    // 添加资源更新事件监听
    auto resourceListener = cocos2d::EventListenerCustom::create(
        "player_turn_resource_update",
        [this](cocos2d::EventCustom* event) {
            ValueMap* data = static_cast<ValueMap*>(event->getUserData());
            if (data) {
                int playerId = data->at("player_id").asInt();
                Player* player = m_gameManager->getPlayer(playerId);
                if (player && player->getIsHuman()) {
                    // 更新HUD显示
                    _hudLayer->updateResources(
                        player->getGold(),
                        player->getSciencePerTurn(),
                        player->getCulturePerTurn(),
                        data->at("turn").asInt()
                    );

                    // 更新科技树/文化树的进度显示
                    _hudLayer->updateSciencePerTurn(player->getSciencePerTurn());
                    _hudLayer->updateCulturePerTurn(player->getCulturePerTurn());
                }
            }
        });
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(resourceListener, this);
}

Player* GameScene::getCurrentPlayer() const {
    return m_gameManager ? m_gameManager->getCurrentPlayer() : nullptr;
}

void GameScene::onExit() {
    // 清理游戏管理器
    if (m_gameManager) {
        m_gameManager->cleanup();
    }

    // 注意：现在不需要删除_techTree等，因为它们在Player内部
    Scene::onExit();
}

TileData GameScene::getTileData(Hex h) { return _mapLayer->getTileData(h); }
