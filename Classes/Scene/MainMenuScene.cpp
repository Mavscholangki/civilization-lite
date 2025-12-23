#include "MainMenuScene.h"
#include "GameScene.h" // 点击开始后要跳转到 GameScene

USING_NS_CC;
using namespace cocos2d::ui; // 使用 UI 命名空间

Scene* MainMenuScene::createScene() {
    return MainMenuScene::create();
}

bool MainMenuScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // 1. 背景 (暂时用深灰色代替图片)
    auto bg = LayerColor::create(Color4B(40, 40, 50, 255));
    this->addChild(bg);

    // 2. 游戏标题
    auto title = Label::createWithSystemFont("CIVILIZATION LITE", "Arial", 60);
    title->setPosition(center.x, visibleSize.height - 150);
    title->enableShadow(Color4B::BLACK, Size(2, -2), 2); // 加阴影
    this->addChild(title);

    // 3. "新游戏" 按钮
    // 这里我们用 Button 类，它比 Menu 好用，支持按下变色、禁用等状态
    // 如果没有图片，可以用 create() 创建空按钮，然后加文字
    auto btnNewGame = Button::create();
    btnNewGame->setTitleText("NEW GAME");
    btnNewGame->setTitleFontSize(30);
    btnNewGame->setTitleColor(Color3B::WHITE);
    btnNewGame->setPosition(Vec2(center.x, center.y + 50));
    // 添加点击事件
    btnNewGame->addClickEventListener(CC_CALLBACK_1(MainMenuScene::onNewGameClicked, this));
    this->addChild(btnNewGame);

    // 4. "退出" 按钮
    auto btnExit = Button::create();
    btnExit->setTitleText("EXIT GAME");
    btnExit->setTitleFontSize(30);
    btnExit->setTitleColor(Color3B(200, 200, 200)); // 稍微灰一点
    btnExit->setPosition(Vec2(center.x, center.y - 50));
    btnExit->addClickEventListener(CC_CALLBACK_1(MainMenuScene::onExitClicked, this));
    this->addChild(btnExit);

    return true;
}

void MainMenuScene::onNewGameClicked(Ref* sender) {
    CCLOG("Start New Game...");

    // 先跳转到Loading场景，然后从Loading场景跳转到GameScene
    auto loadingScene = LoadingScene::createScene();
    auto loadingNode = static_cast<LoadingScene*>(loadingScene);

    // 设置要跳转的下一个场景（GameScene）
    auto gameScene = GameScene::createScene();
    loadingNode->setNextScene(gameScene);

    // 切换到Loading场景
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, loadingScene));
}

void MainMenuScene::onExitClicked(Ref* sender) {
    // 退出游戏
    Director::getInstance()->end();
}