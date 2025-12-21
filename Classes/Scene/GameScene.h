#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "Map/TileData.h"
#include "Map/GameMapLayer.h"

class TechTree;
class CultureTree;
class PolicyManager;


class HUDLayer;
class GameMapLayer;
class AbstractUnit;

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual void onExit() override;
	TileData getTileData(Hex h) { return _mapLayer->getTileData(h); }
    CREATE_FUNC(GameScene);

private:
    // 私有实现方法
    void initTechTree();
    void initCultureTree();
    void initPolicySystem();
    void setupCallbacks();

    // 私有成员（使用前向声明的指针）
    TechTree* _techTree;    // 科技系统实例
    CultureTree* _cultureTree;  // 文化系统实例
    PolicyManager* _policyManager = nullptr;// 政策系统实例
    HUDLayer* _hudLayer;    // HUD层引用
    GameMapLayer* _mapLayer; // 地图层引用
};

#endif // __GAME_SCENE_H__