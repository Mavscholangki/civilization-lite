#include "GameScene.h"
#include "../Map/GameMapLayer.h"
#include "../UI/HUDLayer.h" // 引用 UI 头文件
#include "../UI/CityProductionPanel.h"
#include "../Development/TechSystem.h"
#include "../Units/Base/AbstractUnit.h"  // 如果需要AbstractUnit的完整定义
#include "../Development/CultureSystem.h"

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    // 1. 创建地图层
    _mapLayer = GameMapLayer::create();
    if (!_mapLayer) return false;
    this->addChild(_mapLayer, 0);

    // 2. 创建HUD层
    _hudLayer = HUDLayer::create();
    if (!_hudLayer) return false;
    this->addChild(_hudLayer, 100);

    // --- 核心联动 ---
    // 当地图层汇报“选中了单位”时 -> 让 HUD 层显示面板
    _mapLayer->setOnUnitSelectedCallback([this](AbstractUnit* unit) {
        // 初始化科技树
        initTechTree();

        // 设置回调函数
        setupCallbacks();
    });

    // 3. 创建生产面板 (来自 feature/productionPanel 分支)
    auto productionPanelLayer = CityProductionPanel::create();
    this->addChild(productionPanelLayer, 120);

    // 4. 初始化科技树和回调 (来自 main 分支)
    initTechTree();
    initCultureTree();
    initPolicySystem();
    setupCallbacks();

    return true;
}

void GameScene::initTechTree() {
    _techTree = new TechTree();
    if (!_techTree) {
        CCLOG("ERROR: Failed to create TechTree!");
        return;
    }
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

    // 设置当前研究文化（例如01：法典）
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
	// 创建政策管理器实例
    _policyManager = new PolicyManager();

    if (!_policyManager) {
        CCLOG("ERROR: Failed to create PolicyManager!");
        return;
    }

	// 初始化政策数据
    _policyManager->initializePolicies();

	// 将政策管理器与文化系统关联
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

	// 设置初始政策槽
    if (_cultureTree) {
        const int* slots = _cultureTree->getActivePolicySlots();
        _policyManager->setPolicySlots(slots[0], slots[1], slots[2], slots[3]);
        CCLOG("Initial policy slots: Military=%d, Economic=%d, Diplomatic=%d, Wildcard=%d",
            slots[0], slots[1], slots[2], slots[3]);
    }

    // 设置HUD层的政策管理层
    if (_hudLayer) {
        _hudLayer->setPolicyManager(_policyManager);
        CCLOG("PolicyManager set on HUDLayer");
    }

    CCLOG("Policy system initialized with %zu unlocked policies",
        _policyManager->getUnlockedPolicies().size());
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

    // 下一回合按钮回调
    _hudLayer->setNextTurnCallback([this]() {
        _mapLayer->onNextTurnAction();
        if (_techTree) {
            int sciencePerTurn = 5; // �?���?��市、建筑等计算
            _techTree->updateProgress(sciencePerTurn);
        }

        // 文化系统更新
        if (_cultureTree) {
            int culturePerTurn = 3; // �?���?��念�?、剧院等计算
            _cultureTree->updateProgress(culturePerTurn);
        }

        // 新功能：政策系统更新（如果需要每回合处理什么）
        if (_policyManager) {
			// 例如：检查政策组合效果是否持续触发
            _policyManager->checkPolicyCombos();
        }
        // 更新资源显示
        static int turn = 1; turn++;
        static int gold = 0; gold += 5;
        static int science = 0; science += 5;
        static int culture = 0; culture += 3;
        _hudLayer->updateResources(gold, science, culture, turn);
        });
}

void GameScene::onExit() {
    if (_techTree) {
        delete _techTree;
        _techTree = nullptr;
    }

    // 清理文化树
    if (_cultureTree) {
        delete _cultureTree;
        _cultureTree = nullptr;
    }

    // 清理政策管理层
    if (_policyManager) {
        delete _policyManager;
        _policyManager = nullptr;
    }

    // 调用父类的onExit
    Scene::onExit();
}