#include "GameScene.h"
#include "../Map/GameMapLayer.h"
#include "../UI/HUDLayer.h"
#include "../UI/CityProductionPanel.h"
#include "../Core/GameManager.h"
#include "../Core/Player.h"

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

    // 2. 创建玩家
    m_humanPlayer = Player::create(0, CivilizationType::CHINA);
    if (m_humanPlayer) {
        m_humanPlayer->setIsHuman(true);
        m_humanPlayer->setPlayerName("Human Player");
        m_gameManager->addPlayer(m_humanPlayer);
        CCLOG("Human player created");
    }

    // 创建AI玩家
    Player* aiPlayerGermany = Player::create(1, CivilizationType::GERMANY);
    if (aiPlayerGermany) {
        aiPlayerGermany->setIsHuman(false);
        aiPlayerGermany->setPlayerName("AI Germany");
        m_gameManager->addPlayer(aiPlayerGermany);
    }

    Player* aiPlayerRussia = Player::create(2, CivilizationType::RUSSIA);
    if (aiPlayerRussia) {
        aiPlayerRussia->setIsHuman(false);
        aiPlayerRussia->setPlayerName("AI Russia");
        m_gameManager->addPlayer(aiPlayerRussia);
    }

    // 3. 创建地图层
    _mapLayer = GameMapLayer::create();
    if (!_mapLayer) return false;
    this->addChild(_mapLayer, 0);

    // 4. 创建HUD层
    _hudLayer = HUDLayer::create();
    if (!_hudLayer) return false;
    this->addChild(_hudLayer, 100);

    // 5. 创建生产面板层
    _productionPanelLayer = CityProductionPanel::create();
    _productionPanelLayer->setVisible(false);
    this->addChild(_productionPanelLayer, 120);

    // 6. 设置HUD层使用人类玩家的系统实例
    if (m_humanPlayer) {
        // 关键：这里设置的是Player内部的TechTree、CultureTree
        _hudLayer->setTechTree(m_humanPlayer->getTechTree());
        _hudLayer->setCultureTree(m_humanPlayer->getCultureTree());
        _hudLayer->setPolicyManager(m_humanPlayer->getPolicyManager());
        CCLOG("HUDLayer set to use human player's system instances");
    }

    // 7. 设置回调
    setupCallbacks();

    // 8. 设置游戏管理器回调
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

    CCLOG("GameScene initialized successfully");

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
        m_gameManager->endTurn();
        });

    _mapLayer->setOnCitySelectedCallback([this](BaseCity* city) {
        if (city) {
            _hudLayer->hideUnitInfo();
            _productionPanelLayer->setVisible(true);
        }
        else {
            _productionPanelLayer->setVisible(false);
        }
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