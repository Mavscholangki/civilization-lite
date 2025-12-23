#include "LoadingScene.h"
#include "GameScene.h"

USING_NS_CC;

Scene* LoadingScene::createScene() {
    return LoadingScene::create();
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

    // 初始化进度
    _nextScene = nullptr;

    // 开始模拟加载
    this->schedule(CC_SCHEDULE_SELECTOR(LoadingScene::onLoadingComplete), 0.05f);

    return true;
}

void LoadingScene::setNextScene(cocos2d::Scene* scene) {
    // 修复：保持引用计数
    if (_nextScene) {
        _nextScene->release();
    }
    _nextScene = scene;
    if (_nextScene) {
        _nextScene->retain();  // 保留引用
    }
}

LoadingScene::~LoadingScene() {
    // 清理时释放引用
    if (_nextScene) {
        _nextScene->release();
        _nextScene = nullptr;
    }
}

void LoadingScene::onLoadingComplete(float dt) {
    static float progress = 0.0f;
    static int step = 0;
    static const char* steps[] = {
        "Initializing game systems...",
        "Generating world map...",
        "Creating civilizations...",
        "Setting up AI opponents...",
        "Loading textures and assets...",
        "Finalizing game state...",
        "Starting game..."
    };

    // 更新进度
    progress += 1.5f; // 控制加载速度
    if (progress > 100.0f) {
        progress = 100.0f;
    }

    _progressBar->setPercentage(progress);

    // 每15%更新一次状态文本
    if (progress >= 15.0f * (step + 1) && step < 6) {
        step++;
        if (step < 7) {
            _loadingLabel->setString(steps[step]);
        }
    }

    // 加载完成后跳转
    if (progress >= 100.0f) {
        this->unschedule(CC_SCHEDULE_SELECTOR(LoadingScene::onLoadingComplete));

        // 延迟0.5秒后跳转
        this->scheduleOnce([this](float dt) {
            Scene* targetScene = nullptr;

            if (_nextScene) {
                targetScene = _nextScene;
            }
            else {
                // 默认跳转到游戏场景
                targetScene = GameScene::createScene();
            }

            if (targetScene) {
                Director::getInstance()->replaceScene(TransitionFade::create(0.8f, targetScene));
            }
            }, 0.5f, "jump_to_game");
    }
}