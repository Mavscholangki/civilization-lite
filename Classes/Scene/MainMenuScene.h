#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h" // 引入 GUI 库
#include "LoadingScene.h" // 添加这行
#include "GameScene.h"    // 添加这行

class MainMenuScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(MainMenuScene);

private:
    // 按钮回调
    void onNewGameClicked(cocos2d::Ref* sender);
    void onExitClicked(cocos2d::Ref* sender);
};

#endif