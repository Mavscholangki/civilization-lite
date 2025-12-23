#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "Map/TileData.h"
#include "Utils/HexUtils.h"
#include "SelectionScene.h"

// 前向声明
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
    virtual void onExit() override;
    TileData getTileData(Hex h);

    // 添加获取当前玩家的方法
    Player* getCurrentPlayer() const;

    CREATE_FUNC(GameScene);

private:
    // 私有实现方法 - 现在这些只是包装器，调用Player的方法
    void initTechTree();    // 已废弃，保持为空
    void initCultureTree(); // 已废弃，保持为空
    void initPolicySystem(); // 已废弃，保持为空
    void setupCallbacks();

    // 游戏管理器
    GameManager* m_gameManager;

    // 玩家引用（现在系统实例都在Player中）
    Player* m_humanPlayer;

    // 场景层引用
    HUDLayer* _hudLayer;
    GameMapLayer* _mapLayer;
    CityProductionPanel* _productionPanelLayer;
};

#endif // __GAME_SCENE_H__