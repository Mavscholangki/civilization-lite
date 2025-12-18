#include "GameScene.h"
#include "../Map/GameMapLayer.h"
<<<<<<< HEAD
#include "../UI/HUDLayer.h" // 引用 UI 头文件
<<<<<<< HEAD
#include "../UI/CityProductionPanel.h"
=======
#include "../UI/HUDLayer.h"
#include "../Development/TechSystem.h"
#include "../Units/Base/AbstractUnit.h"  // 如果需要AbstractUnit的完整定义
>>>>>>> a57c8333fd4baa8f82905889ae79c08aebf48c0e
=======
>>>>>>> feature/productionPanel

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

<<<<<<< HEAD
<<<<<<< HEAD
    auto productionPanelLayer = CityProductionPanel::create();
    this->addChild(productionPanelLayer, 120);
=======
>>>>>>> feature/productionPanel

    // --- 核心联动 ---
    // 当地图层汇报“选中了单位”时 -> 让 HUD 层显示面板
    mapLayer->setOnUnitSelectedCallback([hudLayer](AbstractUnit* unit) {
=======
    // 初始化科技树
    initTechTree();

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

void GameScene::setupCallbacks() {
    // --- 回调设置 ---

    // 地图层选中单位 -> 更新HUD显示
    _mapLayer->setOnUnitSelectedCallback([this](AbstractUnit* unit) {
>>>>>>> a57c8333fd4baa8f82905889ae79c08aebf48c0e
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
            // 假设每回合获得科研值
            int sciencePerTurn = 5; // 可根据城市、建筑等计算

            // 更新科技进度
            _techTree->addSciencePoints(sciencePerTurn);

            // 如果HUD层有更新科学值的方法，可以调用
            // _hudLayer->updateScience(sciencePerTurn);
        }

        // 更新资源显示
        static int turn = 1;
        turn++;
        static int gold = 0;
        gold += 5; // 每回合+5金币
        static int science = 0;
        science += 5; // 每回合+5科研

        _hudLayer->updateResources(gold, science, turn);
        });
}

void GameScene::onExit() {
    // 清理科技树
    if (_techTree) {
        delete _techTree;
        _techTree = nullptr;
    }

    // 调用父类的onExit
    Scene::onExit();
}