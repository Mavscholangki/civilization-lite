// GameScene.h
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "Map/TileData.h"
#include "Utils/HexUtils.h"
#include "SelectionScene.h"

class TechTree;
class CultureTree;
class PolicyManager;
class GameManager;
class Player;
class GameMapLayer;
class HUDLayer;
class AbstractUnit;
class CityProductionPanel;

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual void onEnter() override;  // 重要：重写onEnter

    // 分离的初始化函数
    bool initGameData();      // 初始化游戏数据（可以在LoadingScene中调用）
    bool initGraphics();      // 初始化图形（必须在场景进入导演后调用）

    virtual void onExit() override;
    TileData getTileData(Hex h);

    Player* getCurrentPlayer() const;
    CREATE_FUNC(GameScene);

private:
    void initTechTree();
    void initCultureTree();
    void initPolicySystem();
    void setupCallbacks();

    GameManager* m_gameManager;
    Player* m_humanPlayer;
    HUDLayer* _hudLayer;
    GameMapLayer* _mapLayer;
    CityProductionPanel* _productionPanelLayer;

    bool _dataInitialized;    // 数据是否已初始化
    bool _graphicsInitialized; // 图形是否已初始化
};

#endif