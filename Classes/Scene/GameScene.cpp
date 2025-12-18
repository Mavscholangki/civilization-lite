#include "GameScene.h"
#include "../Map/GameMapLayer.h"
#include "../UI/HUDLayer.h" // 引用 UI 头文件

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    auto mapLayer = GameMapLayer::create();
    this->addChild(mapLayer, 0);

    auto hudLayer = HUDLayer::create();
    this->addChild(hudLayer, 100);


    // --- 核心联动 ---
    // 当地图层汇报“选中了单位”时 -> 让 HUD 层显示面板
    mapLayer->setOnUnitSelectedCallback([hudLayer](AbstractUnit* unit) {
        if (unit) {
            hudLayer->showUnitInfo(unit);
        }
        else {
            hudLayer->hideUnitInfo();
        }
        });

    hudLayer->setBuildCityCallback([mapLayer]() {
        mapLayer->onBuildCityAction();
        });

    // 连接下一回合
    hudLayer->setNextTurnCallback([mapLayer, hudLayer]() {
        mapLayer->onNextTurnAction();
        // 简单模拟回合数增加
        static int turn = 1;
        turn++;
        static int gold = 0;
        gold += 5; // 假装每回合+5块钱
        hudLayer->updateResources(gold, 0, turn);
        });

    return true;
}