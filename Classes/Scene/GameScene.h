// GameScene.h
#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "Map/TileData.h"
#include "Utils/HexUtils.h"
#include "City/BaseCity.h"
#include "SelectionScene.h"

USING_NS_CC;

class TechTree;
class CultureTree;
class PolicyManager;
class GameManager;
class Player;

class BaseCity;
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

    // 覆盖层相关
    void createCoverLayer();          // 创建覆盖层（与LoadingScene一样）
    void updateCoverProgress(float progress); // 更新覆盖层进度
    void removeCoverLayer(float fadeTime = 0.5f); // 移除覆盖层

    virtual void onExit() override;
    TileData getTileData(Hex h);
    void updateProductionPanel(int playerID, BaseCity* currentCity);
    // 添加获取当前玩家的方法
    Player* getCurrentPlayer() const;


private:
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

    // 覆盖层相关成员
    cocos2d::LayerColor* _coverLayer;          // 覆盖层背景
    cocos2d::Label* _coverTitleLabel;          // 标题
    cocos2d::Label* _coverSubtitleLabel;       // 副标题
    cocos2d::ProgressTimer* _coverProgressBar; // 进度条
    cocos2d::Label* _coverLoadingLabel;        // 加载文本
    cocos2d::Label* _coverTipLabel;            // 提示文本
    cocos2d::Label* _coverVersionLabel;        // 版本信息

    bool _coverLayerCreated;                    // 覆盖层是否已创建
    Node* _coverContainer;
    LayerColor* _coverProgressBarForeground;
};

#endif