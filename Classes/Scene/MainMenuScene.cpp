#include "MainMenuScene.h"
#include "SelectionScene.h"
#include "../Audio/MusicManager.h"

USING_NS_CC;
using namespace cocos2d::ui;

Scene* MainMenuScene::createScene()
{
    return MainMenuScene::create();
}

bool MainMenuScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // 1. 背景
    auto bg = LayerColor::create(Color4B(40, 40, 50, 255));
    this->addChild(bg);

    // 2. 游戏标题
    auto title = Label::createWithSystemFont("CIVILIZATION LITE", "Arial", 60);
    title->setPosition(center.x, visibleSize.height - 150);
    title->enableShadow(Color4B::BLACK, Size(2, -2), 2);
    this->addChild(title);

    // 3. "新游戏" 按钮
    auto btnNewGame = Button::create();
    btnNewGame->setTitleText("NEW GAME");
    btnNewGame->setTitleFontSize(30);
    btnNewGame->setTitleColor(Color3B::WHITE);
    btnNewGame->setPosition(Vec2(center.x, center.y + 50));
    btnNewGame->addClickEventListener(CC_CALLBACK_1(MainMenuScene::onNewGameClicked, this));
    this->addChild(btnNewGame);

    // 4. "退出" 按钮
    auto btnExit = Button::create();
    btnExit->setTitleText("EXIT GAME");
    btnExit->setTitleFontSize(30);
    btnExit->setTitleColor(Color3B(200, 200, 200));
    btnExit->setPosition(Vec2(center.x, center.y - 50));
    btnExit->addClickEventListener(CC_CALLBACK_1(MainMenuScene::onExitClicked, this));
    this->addChild(btnExit);

    // 5. 播放主菜单背景音乐
    MusicManager::getInstance()->playMainMenuMusic();

    return true;
}

void MainMenuScene::onNewGameClicked(Ref* sender)
{
    CCLOG("Start New Game...");
    // 切换场景：带有淡入淡出特效 (1.0秒)
    auto gameScene = GameScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, gameScene));

    // 音乐保持播放，切换到文明选择界面
    auto selectionScene = CivilizationSelectionScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, selectionScene));
}

void MainMenuScene::onExitClicked(Ref* sender)
{
    // 停止音乐并清理
    MusicManager::getInstance()->stopMusic();
    MusicManager::destroyInstance();

    // 退出游戏
    Director::getInstance()->end();
}