#include "GameScene.h"
#include "../Map/GameMapLayer.h"
#include "../UI/HUDLayer.h"
#include "../Development/TechSystem.h"
#include "../Development/CultureSystem.h"
#include "../Units/Base/AbstractUnit.h"  // 如果需要AbstractUnit的完整定义

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    // 创建地图层
    _mapLayer = GameMapLayer::create();
    if (!_mapLayer) return false;
    this->addChild(_mapLayer, 0);

    // 创建HUD层
    _hudLayer = HUDLayer::create();
    if (!_hudLayer) return false;
    this->addChild(_hudLayer, 100);

    // 初始化树
    initTechTree();
    initCultureTree();
    initPolicySystem();

    // 设置回调函数
    setupCallbacks();

    return true;
}

void GameScene::initTechTree() {
    // 创建科技系统实例
    _techTree = new TechTree();

    if (!_techTree) {
        CCLOG("ERROR: Failed to create TechTree!");
        return;
    }

    // 这里可以初始化科技树数据
    // 例如：_techTree->initialize();

    // 设置HUD层的科技系统
    if (_hudLayer && _techTree) {
        _hudLayer->setTechTree(_techTree);
    }

    CCLOG("TechTree system initialized");
}

void GameScene::initCultureTree() {
    // 创建文化系统实例
    _cultureTree = new CultureTree();

    if (!_cultureTree) {
        CCLOG("ERROR: Failed to create CultureTree!");
        return;
    }

    // 初始化文化树
    _cultureTree->initializeCultureTree();

    // 设置一个默认的当前研究文化（例如101：法典）
    std::vector<int> unlockable = _cultureTree->getUnlockableCultureList();
    if (!unlockable.empty()) {
        _cultureTree->setCurrentResearch(unlockable[0]);
        CCLOG("Set default culture research to: %d", unlockable[0]);
    }
    else {
        CCLOG("No unlockable cultures available");
    }

    // 设置HUD层的文化系统
    if (_hudLayer && _cultureTree) {
        _hudLayer->setCultureTree(_cultureTree);
    }

    CCLOG("CultureTree system initialized");
}

void GameScene::initPolicySystem() {
    // 创建政策管理器
    _policyManager = new PolicyManager();

    if (!_policyManager) {
        CCLOG("ERROR: Failed to create PolicyManager!");
        return;
    }

    // 初始化政策系统
    _policyManager->initializePolicies();

    // 设置文化树引用
    if (_cultureTree) {
        // 设置政府获取回调
        _policyManager->setGovernmentGetter([this]() {
            return _cultureTree->getCurrentGovernment();
            });

        // 设置政策获取回调
        _policyManager->setPolicyGetter([this](int cultureId) {
            return _cultureTree->getPoliciesUnlockedByCulture(cultureId);
            });

        // 设置政策解锁回调
        _policyManager->setPolicyUnlockedCallback([this](int policyId) {
            CCLOG("Policy %d unlocked via callback", policyId);
            });

        // 关键：将PolicyManager作为监听器添加到文化系统
        _cultureTree->addEventListener(_policyManager);
        CCLOG("PolicyManager registered as CultureEventListener");
    }

    // 设置初始政策槽位
    if (_cultureTree) {
        const int* slots = _cultureTree->getActivePolicySlots();
        _policyManager->setPolicySlots(slots[0], slots[1], slots[2], slots[3]);
        CCLOG("Initial policy slots: Military=%d, Economic=%d, Diplomatic=%d, Wildcard=%d",
            slots[0], slots[1], slots[2], slots[3]);
    }

    // 设置HUD层的政策管理器
    if (_hudLayer) {
        _hudLayer->setPolicyManager(_policyManager);
        CCLOG("PolicyManager set on HUDLayer");
    }

    CCLOG("Policy system initialized with %zu unlocked policies",
        _policyManager->getUnlockedPolicies().size());
}

void GameScene::setupCallbacks() {
    // --- 回调设置 ---

    // 地图层选中单位 -> 更新HUD显示
    _mapLayer->setOnUnitSelectedCallback([this](AbstractUnit* unit) {
        if (unit) {
            _hudLayer->showUnitInfo(unit);
        }
        else {
            _hudLayer->hideUnitInfo();
        }
        });

    // HUD建城按钮 -> 地图层建城动作
    _hudLayer->setBuildCityCallback([this]() {
        _mapLayer->onBuildCityAction();
        });

    // 下一回合按钮回调
    _hudLayer->setNextTurnCallback([this]() {
        // 地图层下一回合逻辑
        _mapLayer->onNextTurnAction();

        // 科技系统更新
        if (_techTree) {
            int sciencePerTurn = 5; // 可根据城市、建筑等计算
            _techTree->updateProgress(sciencePerTurn);
        }

        // 文化系统更新
        if (_cultureTree) {
            int culturePerTurn = 3; // 可根据纪念碑、剧院等计算
            _cultureTree->updateProgress(culturePerTurn);
        }

        // 新增：政策系统更新（如果需要每回合处理什么）
        if (_policyManager) {
            // 例如：检查政策组合效果是否持续激活
            _policyManager->checkPolicyCombos();
        }

        // 更新资源显示
        static int turn = 1;
        turn++;
        static int gold = 0;
        gold += 5; // 每回合+5金币
        static int science = 0;
        science += 5; // 每回合+5科研
        static int culture = 0;
        culture += 3; // 每回合+3文化

        _hudLayer->updateResources(gold, science, culture, turn);
        });
}

void GameScene::onExit() {
    // 清理科技树
    if (_techTree) {
        delete _techTree;
        _techTree = nullptr;
    }

    // 清理文化树
    if (_cultureTree) {
        delete _cultureTree;
        _cultureTree = nullptr;
    }

    // 清理政策管理器
    if (_policyManager) {
        delete _policyManager;
        _policyManager = nullptr;
    }

    // 调用父类的onExit
    Scene::onExit();
}