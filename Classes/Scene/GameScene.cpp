#include "GameScene.h"
#include "../Map/GameMapLayer.h"
#include "../UI/HUDLayer.h"
#include "../UI/CityProductionPanel.h"
#include "../Core/GameManager.h"
#include "../Core/Player.h"
#include "SelectionScene.h"
#include "../Audio/MusicManager.h"

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    CCLOG("GameScene::init() - 基本初始化开始");

    // 只做最基本的场景初始化
    _dataInitialized = false;
    _graphicsInitialized = false;
    _coverLayerCreated = false;  // 确保覆盖层状态正确

    // 设置背景颜色（立即可见，避免黑屏）
    auto bg = LayerColor::create(Color4B(50, 50, 70, 255));
    this->addChild(bg, -100);

    CCLOG("GameScene::init() - 基本初始化完成");
    return true;
}

bool GameScene::initGameData() {
    if (_dataInitialized) {
        return true;
    }

    CCLOG("GameScene::initGameData() - start");

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
    CivilizationType playerCiv = CivilizationType::CHINA;
    if (CivilizationSelectionScene::getSelectedCivilization() != CivilizationType::BASIC) {
        playerCiv = CivilizationSelectionScene::getSelectedCivilization();
    }

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

    // ==================== 播放文明音乐 ====================
    // 根据玩家选择的文明播放对应的背景音乐
    MusicManager::getInstance()->playCivilizationMusic(playerCiv);
    CCLOG("Started playing civilization music for civ type: %d", static_cast<int>(playerCiv));
    // ======================================================

    CCLOG("GameScene::initGameData() - end");
    _dataInitialized = true;
    return true;
}

bool GameScene::initGraphics() {
    if (_graphicsInitialized) return true;

    CCLOG("GameScene::initGraphics() - 图形初始化开始");

    // 确保在覆盖层下面创建游戏内容
    int zOrder = -1;

    // 1. 创建地图层
    _mapLayer = GameMapLayer::create();
    if (!_mapLayer) return false;
    this->addChild(_mapLayer, zOrder++);
    CCLOG("地图层创建完成，zOrder: %d", zOrder - 1);

    // 2. 创建HUD层
    _hudLayer = HUDLayer::create();
    if (!_hudLayer) return false;
    this->addChild(_hudLayer, zOrder++);
    CCLOG("HUD层创建完成，zOrder: %d", zOrder - 1);

    // 3. 创建生产面板层
    _productionPanelLayer = CityProductionPanel::create();
    _productionPanelLayer->setVisible(true);
    this->addChild(_productionPanelLayer, zOrder++);
    CCLOG("生产面板层创建完成，zOrder: %d", zOrder - 1);

    // 4. 设置HUD层使用人类玩家的系统实例
    if (m_humanPlayer) {
        _hudLayer->setTechTree(m_humanPlayer->getTechTree());
        _hudLayer->setCultureTree(m_humanPlayer->getCultureTree());
        _hudLayer->setPolicyManager(m_humanPlayer->getPolicyManager());
        CCLOG("HUDLayer设置完成");
    }

    // 5. 设置回调
    setupCallbacks();
    CCLOG("回调设置完成");

    // 6. 设置游戏管理器回调
    if (m_gameManager) {
        m_gameManager->setOnTurnStartCallback([this](int playerId) {
            Player* currentPlayer = m_gameManager->getCurrentPlayer();
            if (currentPlayer && currentPlayer->getIsHuman()) {
                CCLOG("Human player turn started");
                _hudLayer->setTechTree(currentPlayer->getTechTree());
                _hudLayer->setCultureTree(currentPlayer->getCultureTree());
                _hudLayer->setPolicyManager(currentPlayer->getPolicyManager());
            }
            });
    }

    CCLOG("GameScene::initGraphics() - 图形初始化完成");
    _graphicsInitialized = true;

    // 启动游戏循环（但等覆盖层移除后再开始真正的游戏逻辑）
    this->scheduleUpdate();
    CCLOG("游戏循环已启动");

    return true;
}

void GameScene::createCoverLayer() {
    if (_coverLayerCreated) {
        CCLOG("覆盖层已存在，跳过创建");
        return;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // 1. 创建覆盖层背景 - 设置正确的混合模式
    _coverLayer = LayerColor::create(Color4B(25, 25, 35, 255));
    _coverLayer->setContentSize(visibleSize);
    _coverLayer->setOpacity(255);

    // 关键：设置混合模式，支持透明度变化
    _coverLayer->setBlendFunc(BlendFunc::ALPHA_PREMULTIPLIED);
    // 或者使用标准混合模式
    // _coverLayer->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);

    this->addChild(_coverLayer, 9999); // 放在最顶层

    // 2. 创建一个容器节点来包裹所有覆盖层UI元素
    auto uiContainer = Node::create();
    uiContainer->setContentSize(visibleSize);
    uiContainer->setPosition(0, 0);
    _coverLayer->addChild(uiContainer, 1);

    // 保存容器引用
    _coverContainer = uiContainer;

    // 3. 将所有UI元素添加到容器中
    // 游戏Logo
    _coverTitleLabel = Label::createWithSystemFont("CIVILIZATION LITE", "Arial", 60);
    _coverTitleLabel->setPosition(center.x, visibleSize.height - 150);
    _coverTitleLabel->setColor(Color3B(220, 200, 120));
    _coverTitleLabel->enableShadow(Color4B::BLACK, Size(2, -2), 2);
    uiContainer->addChild(_coverTitleLabel);

    // 加载提示
    _coverSubtitleLabel = Label::createWithSystemFont("Generating World...", "Arial", 30);
    _coverSubtitleLabel->setPosition(center.x, center.y + 50);
    _coverSubtitleLabel->setColor(Color3B(200, 200, 200));
    uiContainer->addChild(_coverSubtitleLabel);

    // 进度条背景
    auto progressBg = LayerColor::create(Color4B(50, 50, 60, 255));
    progressBg->setContentSize(Size(400, 20));
    progressBg->setPosition(center.x - 200, center.y - 10);
    _coverLayer->addChild(progressBg);

    // 进度条前景（使用LayerColor，更简单）
    _coverProgressBarForeground = LayerColor::create(Color4B(80, 160, 240, 255));
    _coverProgressBarForeground->setContentSize(Size(0, 20)); // 初始宽度为0
    _coverProgressBarForeground->setPosition(center.x - 200, center.y - 10);
    _coverProgressBarForeground->setAnchorPoint(Vec2(0, 0));
    _coverLayer->addChild(_coverProgressBarForeground);

    // 动态加载文本
    _coverLoadingLabel = Label::createWithSystemFont("Finalizing game initialization...", "Arial", 22);
    _coverLoadingLabel->setPosition(center.x, center.y - 50);
    _coverLoadingLabel->setColor(Color3B(180, 180, 200));
    uiContainer->addChild(_coverLoadingLabel);

    // 小提示（随机选择一条）
    const char* tips[] = {
        "A civilization's greatness is measured by its cities.",
        "Science leads to progress, culture to enlightenment.",
        "Choose your research wisely - it shapes your empire.",
        "Expand carefully, for every city needs defense.",
        "Trade routes bring wealth and spread influence.",
        "Diplomacy can be as powerful as any army."
    };

    int tipIndex = rand() % 6;
    _coverTipLabel = Label::createWithSystemFont(tips[tipIndex], "Arial", 18);
    _coverTipLabel->setPosition(center.x, 80);
    _coverTipLabel->setColor(Color3B(150, 150, 170));
    _coverTipLabel->setOpacity(200);
    uiContainer->addChild(_coverTipLabel);

    // 版本信息
    _coverVersionLabel = Label::createWithSystemFont("v1.0.0 | Civilization Lite", "Arial", 16);
    _coverVersionLabel->setPosition(center.x, 40);
    _coverVersionLabel->setColor(Color3B(120, 120, 140));
    uiContainer->addChild(_coverVersionLabel);

    _coverLayerCreated = true;
    CCLOG("GameScene覆盖层创建完成");
}

void GameScene::updateCoverProgress(float progress) {
    if (_coverProgressBarForeground) {
        float width = 400 * (progress / 100.0f);
        _coverProgressBarForeground->setContentSize(Size(width, 20));
    }
}

void GameScene::removeCoverLayer(float fadeTime) {
    if (!_coverLayerCreated || !_coverLayer) {
        CCLOG("覆盖层不存在或已移除，跳过");
        return;
    }

    CCLOG("开始移除GameScene覆盖层，淡出时间：%.2f秒", fadeTime);

    // 步骤1：进度条先快速淡出
    if (_coverProgressBar) {
        auto progressFadeOut = FadeOut::create(0.2f); // 进度条快速淡出
        auto removeProgress = CallFunc::create([this]() {
            if (_coverProgressBar) {
                _coverProgressBar->removeFromParent();
                _coverProgressBar = nullptr;
                CCLOG("进度条已淡出移除");
            }
            });
        _coverProgressBar->runAction(Sequence::create(progressFadeOut, removeProgress, nullptr));
    }

    // 同时移除或淡出进度条背景
    if (_coverContainer) {
        auto progressBg = _coverContainer->getChildByName("progress_bg");
        if (progressBg) {
            auto bgFadeOut = FadeOut::create(0.2f);
            progressBg->runAction(bgFadeOut);
        }
    }

    // 步骤2：等待进度条淡出完成后，整体淡出
    auto delay = DelayTime::create(0.25f); // 稍长一点，确保进度条淡出完成

    auto fadeOut = FadeOut::create(fadeTime);
    auto remove = CallFunc::create([this]() {
        if (_coverLayer) {
            this->removeChild(_coverLayer);
            _coverLayer = nullptr;
            _coverContainer = nullptr;

            // 清除所有子节点引用
            _coverTitleLabel = nullptr;
            _coverSubtitleLabel = nullptr;
            _coverLoadingLabel = nullptr;
            _coverTipLabel = nullptr;
            _coverVersionLabel = nullptr;

            _coverLayerCreated = false;
            CCLOG("覆盖层已完全移除");
        }
        });

    _coverLayer->runAction(Sequence::create(delay, fadeOut, remove, nullptr));
}

void GameScene::onEnter() {
    CCLOG("========== GameScene::onEnter() 开始 ==========");

    Scene::onEnter();

    // 创建覆盖层
    createCoverLayer();

    // 使用成员变量来跟踪进度，而不是局部变量
    // 移除旧的进度逻辑，使用新的状态机方法

    // 进度状态
    struct {
        float progress = 0.0f;
        float progressSpeed = 40.0f; // 每秒增加40%，约2.5秒完成
        bool dataInitializing = false;
        bool graphicsInitializing = false;
        bool dataInitialized = false;
        bool graphicsInitialized = false;
    } loadingState;

    loadingState.progress = 0.0f;

    // 每帧更新加载状态
    this->schedule([this, loadingState](float dt) mutable {
        if (!_coverLayerCreated || !_coverLayer) {
            CCLOG("覆盖层不存在，停止更新");
            this->unschedule("loading_update");
            return;
        }

        // 更新进度
        loadingState.progress += loadingState.progressSpeed * dt;
        if (loadingState.progress > 100.0f) loadingState.progress = 100.0f;

        CCLOG("当前进度: %.1f%%", loadingState.progress);

        // 更新进度条（立即更新，不平滑）
        if (_coverProgressBar) {
            _coverProgressBar->setPercentage(loadingState.progress);
        }
        updateCoverProgress(loadingState.progress);

        // 根据进度更新加载文本
        if (_coverLoadingLabel) {
            if (loadingState.progress < 25.0f) {
                _coverLoadingLabel->setString("Initializing game systems...");

                // 在低进度阶段就开始数据初始化（但不在UI线程阻塞）
                if (!loadingState.dataInitializing && !_dataInitialized) {
                    loadingState.dataInitializing = true;
                    CCLOG("开始数据初始化...");

                    // 在下一帧执行初始化，避免阻塞UI更新
                    Director::getInstance()->getScheduler()->performFunctionInCocosThread([this]() {
                        if (initGameData()) {
                            CCLOG("数据初始化完成");
                        }
                        else {
                            CCLOG("数据初始化失败");
                        }
                        });
                }
            }
            else if (loadingState.progress < 50.0f) {
                _coverLoadingLabel->setString("Loading game assets...");
            }
            else if (loadingState.progress < 75.0f) {
                _coverLoadingLabel->setString("Generating world map...");

                // 开始图形初始化
                if (!loadingState.graphicsInitializing && !_graphicsInitialized) {
                    loadingState.graphicsInitializing = true;
                    CCLOG("开始图形初始化...");

                    // 确保数据初始化已完成
                    if (_dataInitialized) {
                        Director::getInstance()->getScheduler()->performFunctionInCocosThread([this]() {
                            if (initGraphics()) {
                                CCLOG("图形初始化完成");
                            }
                            else {
                                CCLOG("图形初始化失败");
                            }
                            });
                    }
                }
            }
            else {
                _coverLoadingLabel->setString("Finalizing game state...");

                // 等待所有初始化完成
                if (_dataInitialized && _graphicsInitialized && loadingState.progress >= 100.0f) {
                    this->unschedule("loading_update");

                    // 最后再更新一次进度到100%
                    if (_coverProgressBar) {
                        _coverProgressBar->setPercentage(100.0f);
                    }

                    CCLOG("所有初始化完成，准备移除覆盖层");

                    // 延迟0.5秒后移除覆盖层
                    this->scheduleOnce([this](float dt) {
                        removeCoverLayer(0.8f);
                        CCLOG("========== GameScene初始化完成，覆盖层已移除 ==========");

                        //// 确保游戏循环已启动
                        //if (!this->isScheduled(schedule_selector(GameScene::update))) {
                        //    this->scheduleUpdate();
                        //    CCLOG("游戏循环已启动");
                        //}
                        }, 0.5f, "remove_cover_delay");
                }
            }
        }

        }, 0.016f, CC_REPEAT_FOREVER, 0.0f, "loading_update");
}

void GameScene::onExit() {
    CCLOG("GameScene onExit called");

    // 如果在地块选择模式下，取消它
    if (_mapLayer && _mapLayer->isSelectingTile()) {
        cancelTileSelection(false);  // 不触发回调，因为场景正在退出
    }

    // 停止游戏循环
    this->unscheduleUpdate();

    // 清理游戏管理器
    if (m_gameManager) {
        m_gameManager->cleanup();
    }

    Scene::onExit();
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

    // HUD建城按钮 -> 执行建城、隐藏按钮、显示生产面板
    _hudLayer->setBuildCityCallback([this]() {
        CCLOG("GameScene: Received Build City Request");

        // 1. 执行地图层的建城逻辑（创建城市模型、消耗开拓者）
        _mapLayer->onBuildCityAction();

        // 2. 立即通知 HUD 隐藏单位信息（包含 Found City 按钮）
        // 这解决了你提到的“可以无限点击”的问题
        _hudLayer->hideUnitInfo();

        // 3. 激活并显示生产面板（文明6风格：建城后右侧面板呼出按钮应该可见）
        if (_productionPanelLayer) {
            _productionPanelLayer->setVisible(true);
            // 如果你的 CityProductionPanel 有拉出的动画接口，可以在这里调用
            // _productionPanelLayer->slideIn(); 
            CCLOG("GameScene: City Production Panel is now visible");
        }
        });

    // 下一回合按钮回调 - 直接调用GameManager
    _hudLayer->setNextTurnCallback([this]() {
        Player* currentPlayer = m_gameManager->getCurrentPlayer();
        if (!currentPlayer || !currentPlayer->getIsHuman()) {
            //return; // 不是人类玩家回合，理论上按钮应不可点，此处做安全保护
        }
        });

    _mapLayer->setOnCitySelectedCallback([this](BaseCity* city) {
        if (city) 
        {
            _hudLayer->hideUnitInfo();
            updateProductionPanel(city->getOwnerPlayer(), city);
            _productionPanelLayer->setVisible(true);
        }
        else
        {
            _productionPanelLayer->setVisible(false);
            auto currentPlayer = this->getCurrentPlayer();
            // --- 新增：快捷研究逻辑 ---
            if (currentPlayer->getIsHuman() && m_gameManager->hasPendingDecisions(currentPlayer->getPlayerId()))
            {
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

TileData GameScene::getTileData(Hex h) { return _mapLayer->getTileData(h); }

void GameScene::updateProductionPanel(int playerID, BaseCity* currentCity)
{
    Player* currentPlayer = GameManager::getInstance()->getPlayer(playerID);
    std::vector<ProductionProgram*> districts;
    std::vector<ProductionProgram*> buildings;
    std::vector<ProductionProgram*> units;
    currentPlayer->getUnlockedProduction(units, districts, buildings);
    std::vector<ProductionProgram*> programs[3] = { districts, buildings, units };
    this->_productionPanelLayer->updateProductionPanel(playerID, currentCity, programs);
}

// 实现 selectTileFromOptions 函数
Hex GameScene::selectTileFromOptions(const std::vector<Hex>& allowedTiles) {
    // 注意：由于选择是异步的（需要用户点击），我们需要使用回调或等待机制
    // 这里返回一个默认值，实际选择通过回调处理
    CCLOG("selectTileFromOptions called with %zu allowed tiles", allowedTiles.size());

    if (!allowedTiles.empty()) {
        return allowedTiles[0]; // 返回第一个作为默认值
    }

    return Hex(0, 0); // 默认返回值
}

// 实现 setTileSelectionCallback 函数
void GameScene::setTileSelectionCallback(const std::function<void(Hex)>& callback,
    const std::function<void()>& cancelCallback) {
    if (_mapLayer) {
        // 使用已有的 selectTile 函数启动选择
        _mapLayer->enableTileSelection({}, callback, cancelCallback); // 空数组表示所有地块都可选
    }
}


// 实现 cancelTileSelection 函数
void GameScene::cancelTileSelection(bool triggerCallback) {
    if (_mapLayer && _mapLayer->isSelectingTile()) {
        _mapLayer->cancelTileSelection(triggerCallback);
        CCLOG("GameScene: Tile selection cancelled");
    }
    else {
        CCLOG("GameScene: Not in tile selection mode, nothing to cancel");
    }
}

// 修改现有的 selectTileAsync 函数，支持取消回调
void GameScene::selectTileAsync(const std::vector<Hex>& allowedTiles,
    const std::function<void(Hex)>& selectionCallback,
    const std::function<void()>& cancelCallback) {
    if (_mapLayer) {

        // 启用选择模式，传入取消回调
        _mapLayer->enableTileSelection(allowedTiles, selectionCallback, cancelCallback);
    }
    else {
        if (cancelCallback) {
            cancelCallback();  // 如果地图层不存在，直接调用取消回调
        }
    }
}