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
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 1. 背景：模拟黎明前的深蓝到微紫的渐变
    auto bg = LayerGradient::create(Color4B(10, 15, 30, 255), Color4B(40, 45, 65, 255));
    this->addChild(bg);

    // 2. 装饰性绘制 (DrawNode) - 营造侧边栏质感
    auto decorationNode = DrawNode::create();
    this->addChild(decorationNode);

    float menuX = origin.x + 250; // 菜单中心线靠近左侧

    // 绘制一条贯穿屏幕的细长竖线（深金色）
    Color4F civGold(0.75f, 0.65f, 0.35f, 0.5f);
    decorationNode->drawSegment(Vec2(menuX - 120, 0), Vec2(menuX - 120, visibleSize.height), 1.0f, civGold);

    // 3. 游戏标题：使用更具古典感的字体，并调整位置
    auto title = Label::createWithSystemFont("CIVILIZATION VI", "Georgia", 50); // Georgia更有衬线书卷气
    title->setAnchorPoint(Vec2(0, 0.5f));
    title->setPosition(menuX - 100, visibleSize.height - 150);
    title->setTextColor(Color4B(230, 210, 170, 255)); // 象牙白偏金
    title->enableShadow(Color4B::BLACK, Size(3, -3), 4);
    this->addChild(title);

    // 4. 按钮美化：增加悬停效果和下划线
    auto createMenuButton = [&](const std::string& text, float yPos, std::function<void(Ref*)> callback) {
        auto btn = Button::create();
        btn->setTitleText(text);
        btn->setTitleFontName("Arial");
        btn->setTitleFontSize(26);
        btn->setTitleColor(Color3B(180, 170, 150)); // 默认浅灰色
        btn->setAnchorPoint(Vec2(0, 0.5f));
        btn->setPosition(Vec2(menuX - 100, yPos));

        // 交互逻辑：点击变亮
        btn->addClickEventListener(callback);

        // 按钮下方的装饰细线
        decorationNode->drawSegment(Vec2(menuX - 100, yPos - 20), Vec2(menuX + 50, yPos - 20), 0.5f, civGold);

        return btn;
        };

    auto btnNewGame = createMenuButton("START JOURNEY", visibleSize.height / 2, CC_CALLBACK_1(MainMenuScene::onNewGameClicked, this));
    this->addChild(btnNewGame);

    auto btnExit = createMenuButton("RETIRE", visibleSize.height / 2 - 80, CC_CALLBACK_1(MainMenuScene::onExitClicked, this));
    this->addChild(btnExit);

    // 5. 底部版权/版本信息（极小、半透明）
    auto version = Label::createWithSystemFont("v1.0 - Built for Exploration", "Arial", 14);
    version->setAnchorPoint(Vec2(1, 0));
    version->setPosition(visibleSize.width - 20, 20);
    version->setOpacity(100);
    this->addChild(version);

    MusicManager::getInstance()->playMainMenuMusic();
    addAtmosphericCenterpiece(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    return true;
}

void MainMenuScene::addAtmosphericCenterpiece(Vec2 center) {
    // 获取实际屏幕尺寸，确保不依赖 init 的局部变量
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建容器
    auto orbitNode = Node::create();
    // 修正后的坐标运算：向右偏移屏幕宽度的 15%
    orbitNode->setPosition(center.x + visibleSize.width * 0.15f, center.y);
    this->addChild(orbitNode, 1);

    auto circles = DrawNode::create();
    orbitNode->addChild(circles);

    Color4F lineGold(0.75f, 0.65f, 0.35f, 0.4f);

    // 绘制圆环逻辑 (确保参数适应你的 Cocos2d-x 版本)
    // 如果报错，请尝试简化为: circles->drawCircle(Vec2::ZERO, 220, 0, 120, false, lineGold);
    circles->drawCircle(Vec2::ZERO, 220, 0, 120, false, 1.0f, 0.4f, lineGold);
    circles->drawCircle(Vec2::ZERO, 180, 0, 100, false, 0.3f, 1.0f, lineGold);
    circles->drawCircle(Vec2::ZERO, 140, 0, 80, false, 1.0f, 1.0f, lineGold);

    // --- 新增：星点装饰 ---
    for (int i = 0; i < 12; i++) {
        float angle = CC_DEGREES_TO_RADIANS(i * 30);
        float r = 140.0f;
        circles->drawDot(Vec2(cos(angle) * r, sin(angle) * r), 2.0f, lineGold);
    }

    circles->drawDot(Vec2::ZERO, 4, lineGold);

    // 动作逻辑：旋转
    orbitNode->runAction(RepeatForever::create(RotateBy::create(40.0f, 360)));

    // 呼吸动画
    circles->runAction(RepeatForever::create(Sequence::create(
        FadeTo::create(3.0f, 80),
        FadeTo::create(3.0f, 200),
        nullptr
    )));
}

void MainMenuScene::onNewGameClicked(Ref* sender)
{
    CCLOG("Start New Game...");

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