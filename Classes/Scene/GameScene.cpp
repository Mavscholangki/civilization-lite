#include "GameScene.h"
#include "../Map/GameMapLayer.h"
#include "../UI/HUDLayer.h"
#include "../UI/CityProductionPanel.h" // 保留生产面板
#include "../Development/TechSystem.h" // 保留科技系统
#include "../Units/Base/AbstractUnit.h"

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

    // 3. 创建生产面板 (来自 feature/productionPanel 分支)
    _productionPanelLayer = CityProductionPanel::create();
    _productionPanelLayer->setVisible(false);
    this->addChild(_productionPanelLayer, 120);

    // 4. 初始化科技树和回调 (来自 main 分支)
    initTechTree();
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

    // HUD建城按钮 -> 地图层建城动作
    _hudLayer->setBuildCityCallback([this]() {
        _mapLayer->onBuildCityAction();
        });

    // 下一回合按钮回调
    _hudLayer->setNextTurnCallback([this]() {
        _mapLayer->onNextTurnAction();
        if (_techTree) {
            _techTree->addSciencePoints(5);
        }
        // 更新资源显示
        static int turn = 1; turn++;
        static int gold = 0; gold += 5;
        static int science = 0; science += 5;
        _hudLayer->updateResources(gold, science, turn);
        });

    _mapLayer->setOnCitySelectedCallback([this](BaseCity* city) {
        if (city) {
            // 1. 隐藏单位信息（因为选中了城市）
            _hudLayer->hideUnitInfo();

            // 2. 显示生产面板
            _productionPanelLayer->setVisible(true);

            // 3. 把选中的城市数据传给面板，让面板知道在给谁造东西
            // _productionPanelLayer->setTargetCity(city); 
        }
        else {
            // 点击空地，隐藏面板
            _productionPanelLayer->setVisible(false);
        }
        });

}

void GameScene::onExit() {
    if (_techTree) {
        delete _techTree;
        _techTree = nullptr;
    }
    Scene::onExit();
}