#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

class TechTree;
class HUDLayer;
class GameMapLayer;
class AbstractUnit;
class CityProductionPanel;

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual void onExit() override;

    CREATE_FUNC(GameScene);

private:
    // 私有实现方法
    void initTechTree();
    void setupCallbacks();

    // 私有成员（使用前向声明的指针）
    TechTree* _techTree;    // 科技系统实例
    HUDLayer* _hudLayer;    // HUD层引用
    GameMapLayer* _mapLayer; // 地图层引用

    CityProductionPanel* _productionPanelLayer; ///<
};

#endif