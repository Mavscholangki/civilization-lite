// LoadingScene.cpp - 修复版本
#include "LoadingScene.h"
#include "GameScene.h"
#include "Map/GameMapLayer.h"
#include "UI/HUDLayer.h"
#include "Core/GameManager.h"

USING_NS_CC;

Scene* LoadingScene::createScene(std::function<cocos2d::Scene* ()> sceneCreator) {
    auto scene = LoadingScene::create();
    auto loadingNode = static_cast<LoadingScene*>(scene);
    loadingNode->setSceneCreator(sceneCreator);
    loadingNode->startRealLoading();
    return scene;
}

bool LoadingScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // 1. 深色背景
    auto bg = LayerColor::create(Color4B(25, 25, 35, 255));
    this->addChild(bg);

    // 2. 游戏Logo
    auto title = Label::createWithSystemFont("CIVILIZATION LITE", "Arial", 60);
    title->setPosition(center.x, visibleSize.height - 150);
    title->setColor(Color3B(220, 200, 120));
    title->enableShadow(Color4B::BLACK, Size(2, -2), 2);
    this->addChild(title);

    // 3. 加载提示
    auto subtitle = Label::createWithSystemFont("Generating World...", "Arial", 30);
    subtitle->setPosition(center.x, center.y + 50);
    subtitle->setColor(Color3B(200, 200, 200));
    this->addChild(subtitle);

    // 4. 进度条背景
    auto progressBg = LayerColor::create(Color4B(50, 50, 60, 255));
    progressBg->setContentSize(Size(400, 20));
    progressBg->setPosition(center.x - 200, center.y - 10);
    this->addChild(progressBg);

    // 5. 进度条
    _progressBar = ProgressTimer::create(Sprite::create());
    _progressBar->setType(ProgressTimer::Type::BAR);
    _progressBar->setMidpoint(Vec2(0, 0));
    _progressBar->setBarChangeRate(Vec2(1, 0));
    _progressBar->setPercentage(0);
    _progressBar->setColor(Color3B(80, 160, 240));
    _progressBar->setContentSize(Size(400, 20));
    _progressBar->setPosition(center.x - 200, center.y - 10);
    this->addChild(_progressBar);

    // 6. 动态加载文本
    _loadingLabel = Label::createWithSystemFont("Initializing game systems...", "Arial", 22);
    _loadingLabel->setPosition(center.x, center.y - 50);
    _loadingLabel->setColor(Color3B(180, 180, 200));
    this->addChild(_loadingLabel);

    // 7. 小提示
    const char* tips[] = {
        "A civilization's greatness is measured by its cities.",
        "Science leads to progress, culture to enlightenment.",
        "Choose your research wisely - it shapes your empire.",
        "Expand carefully, for every city needs defense.",
        "Trade routes bring wealth and spread influence.",
        "Diplomacy can be as powerful as any army."
    };

    int tipIndex = rand() % 6;
    auto tipLabel = Label::createWithSystemFont(tips[tipIndex], "Arial", 18);
    tipLabel->setPosition(center.x, 80);
    tipLabel->setColor(Color3B(150, 150, 170));
    tipLabel->setOpacity(200);
    this->addChild(tipLabel);

    // 8. 版本信息
    auto versionLabel = Label::createWithSystemFont("v1.0.0 | Civilization Lite", "Arial", 16);
    versionLabel->setPosition(center.x, 40);
    versionLabel->setColor(Color3B(120, 120, 140));
    this->addChild(versionLabel);

    // 初始化状态
    _currentState = LoadingState::INITIALIZING;
    _currentResourceIndex = 0;
    _totalResources = 0;
    _successfullyLoaded = 0;
    _failedToLoad = 0;
    _elapsedTime = 0.0f;
    _minDisplayTime = 2.0f; // 最小显示2秒
    _preCreatedScene = nullptr;

    // 定义需要加载的资源列表
    _resourcesToLoad.clear();

    // 添加基本的UI纹理
    addResourcesToLoad({
        "Images/UI/button_normal.png",
        "Images/UI/button_pressed.png",
        "Images/UI/panel_bg.png",
        "Images/UI/progress_bar.png"
        });

    // 添加地形纹理
    addResourcesToLoad({
        "Images/Terrain/grass.png",
        "Images/Terrain/forest.png",
        "Images/Terrain/mountain.png",
        "Images/Terrain/water.png",
        "Images/Terrain/desert.png",
        "Images/Terrain/tundra.png"
        });

    // 添加单位纹理
    addResourcesToLoad({
        "Images/Units/settler.png",
        "Images/Units/warrior.png",
        "Images/Units/builder.png",
        "Images/Units/scout.png"
        });

    // 添加领袖图片
    addResourcesToLoad({
        "Images/Leaders/CivChina.png",
        "Images/Leaders/CivGermany.png",
        "Images/Leaders/CivRussia.png"
        });

    _totalResources = _resourcesToLoad.size();

    return true;
}

void LoadingScene::setSceneCreator(std::function<cocos2d::Scene* ()> creator) {
    _sceneCreator = creator;
}

void LoadingScene::addResourceToLoad(const std::string& resourcePath) {
    _resourcesToLoad.push_back(resourcePath);
    _totalResources = _resourcesToLoad.size();
}

void LoadingScene::addResourcesToLoad(const std::vector<std::string>& resources) {
    for (const auto& resource : resources) {
        _resourcesToLoad.push_back(resource);
    }
    _totalResources = _resourcesToLoad.size();
}

void LoadingScene::startRealLoading() {
    CCLOG("Starting real loading with %d resources", _totalResources);

    // 开始加载计时器
    _elapsedTime = 0.0f;
    _currentState = LoadingState::INITIALIZING;

    // 每帧更新加载状态
    this->schedule(CC_SCHEDULE_SELECTOR(LoadingScene::onLoadingUpdate), 0.016f);
}

void LoadingScene::onLoadingUpdate(float dt) {
    _elapsedTime += dt;

    // 更新进度条（混合真实进度和虚假进度）
    float fakeProgress = (_elapsedTime / _minDisplayTime) * 100.0f;
    float realProgress = 0.0f;

    switch (_currentState) {
    case LoadingState::INITIALIZING:
        _loadingLabel->setString("Initializing game systems...");
        fakeProgress = 0.0f;

        // 短暂初始化后进入资源加载阶段
        if (_elapsedTime > 0.3f) {
            _currentState = LoadingState::LOADING_TEXTURES;
            _currentResourceIndex = 0;
            _successfullyLoaded = 0;
            _failedToLoad = 0;
        }
        break;

    case LoadingState::LOADING_TEXTURES:
        _loadingLabel->setString("Loading basic assets...");

        // 计算真实进度
        if (_totalResources > 0) {
            realProgress = (float)_currentResourceIndex / (float)_totalResources * 100.0f;
        }

        // 异步加载资源
        if (_currentResourceIndex < _totalResources) {
            std::string resourcePath = _resourcesToLoad[_currentResourceIndex];

            // 异步加载纹理
            Director::getInstance()->getTextureCache()->addImageAsync(
                resourcePath,
                CC_CALLBACK_1(LoadingScene::onSingleResourceLoaded, this)
            );

            _currentResourceIndex++;
        }
        else {
            // 所有资源都已开始加载，等待回调完成
            if (_successfullyLoaded + _failedToLoad >= _totalResources) {
                CCLOG("All resources loaded: %d success, %d failed",
                    _successfullyLoaded, _failedToLoad);

                // 进入预创建场景阶段
                _currentState = LoadingState::PRE_CREATING_SCENE;
                _elapsedTime = 0.0f;
            }
        }
        break;

    case LoadingState::PRE_CREATING_SCENE:
        _loadingLabel->setString("Initializing game instances...");

        // 计算进度
        if (_elapsedTime < 0.5f) {
            // 模拟进度：75%到90%
            realProgress = 75.0f + (_elapsedTime / 0.5f) * 15.0f;
        }
        else {
            // 同步预创建游戏场景
            preCreateGameObjectsSync();
            _currentState = LoadingState::FINALIZING;
            _elapsedTime = 0.0f;
        }
        break;

    case LoadingState::FINALIZING:
        _loadingLabel->setString("Shifting the layer...");

        // 模拟最终阶段
        if (_elapsedTime < 0.5f) {
            realProgress = 90.0f + (_elapsedTime / 0.5f) * 10.0f;
        }
        else {
            _currentState = LoadingState::COMPLETE;
            realProgress = 100.0f;
        }
        break;

    case LoadingState::COMPLETE:
        _loadingLabel->setString("Updating global state...");
        realProgress = 100.0f;

        // 确保最小显示时间
        if (_elapsedTime >= _minDisplayTime) {
            this->unschedule(CC_SCHEDULE_SELECTOR(LoadingScene::onLoadingUpdate));

            // 立即跳转，不使用延迟
            goToNextScene();
        }
        break;
    }

    // 混合真实进度和虚假进度
    float displayProgress;
    if (_currentState == LoadingState::INITIALIZING) {
        displayProgress = fakeProgress;
    }
    else {
        // 使用真实进度为主，但确保不会倒退
        displayProgress = std::max(realProgress, _progressBar->getPercentage());

        // 如果真实进度太慢，稍微加点速
        if (displayProgress < fakeProgress && fakeProgress < 90.0f) {
            displayProgress += 0.5f;
        }
    }

    // 更新进度条
    _progressBar->setPercentage(displayProgress);

    // 更新加载标签
    if (_currentState == LoadingState::LOADING_TEXTURES && _totalResources > 0) {
        std::string progressText = StringUtils::format(
            "Loading assets... (%d/%d)",
            _currentResourceIndex, _totalResources
        );
        _loadingLabel->setString(progressText);
    }
}

void LoadingScene::onSingleResourceLoaded(cocos2d::Texture2D* texture) {
    if (texture) {
        _successfullyLoaded++;
    }
    else {
        _failedToLoad++;
    }
}

void LoadingScene::preCreateGameObjectsSync() {
    CCLOG("Synchronously pre-creating game scene...");

    // 直接创建游戏场景并完成所有初始化
    if (_sceneCreator) {
        _preCreatedScene = _sceneCreator();
    }
    else {
        _preCreatedScene = GameScene::createScene();
    }

    if (_preCreatedScene) {
        // 重要：保留引用，防止被自动释放池释放
        _preCreatedScene->retain();
        CCLOG("GameScene created and retained in LoadingScene");

        // 强制GameScene完成所有初始化
        auto gameScene = dynamic_cast<GameScene*>(_preCreatedScene);
        if (gameScene) {
            // GameScene的init已经在createScene中调用过
            CCLOG("GameScene already initialized in createScene");
        }
    }
}

void LoadingScene::goToNextScene() {
    CCLOG("Transitioning to game scene...");

    Scene* targetScene = nullptr;

    if (_sceneCreator) {
        // 使用场景创建函数
        targetScene = _sceneCreator();
    }
    else {
        // 默认创建游戏场景
        targetScene = GameScene::createScene();
    }

    if (targetScene) {
        // 直接切换到GameScene
    // GameScene会自己显示覆盖层并完成初始化
        auto gameScene = GameScene::createScene();
        Director::getInstance()->replaceScene(gameScene);
    }
    else {
        CCLOGERROR("Failed to create target scene!");
    }
}

void LoadingScene::onExit() {
    CCLOG("LoadingScene onExit called");
    Scene::onExit();
}