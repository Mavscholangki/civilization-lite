#include "HUDLayer.h"
#include "City/BaseCity.h"
#include <functional>

USING_NS_CC;
using namespace cocos2d::ui;

// 辅助宏：用于快速设置按钮样式，确保其可点击
#define SETUP_BUTTON(btn, size) \
    btn->ignoreContentAdaptWithSize(false); \
    btn->setContentSize(size); \
    btn->setTouchEnabled(true); \
    btn->setSwallowTouches(true);

bool HUDLayer::init() {
    if (!Layer::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    CCLOG("HUDLayer::init - Creating HUD layer...");

    // 1. 初始化变量
    _techTreePanel = nullptr;
    _techTree = nullptr;
    _isTechTreeOpen = false;
    _cultureTreePanel = nullptr;
    _cultureTree = nullptr;
    _isCultureTreeOpen = false;
    _policyPanel = nullptr;
    _policyManager = nullptr;
    _isPolicyPanelOpen = false;

    // 2. 创建各 UI 模块
    createCiv6StyleResourceDisplay();
    createCiv6StyleNextTurnButton();
    createCiv6StyleFunctionButtons();

    // 3. 单位详情面板 (左下角)
    _unitPanel = LayerColor::create(Color4B(20, 30, 50, 230), 260, 160);
    _unitPanel->setPosition(Vec2(20, 20));
    _unitPanel->setVisible(false);
    _unitPanel->setLocalZOrder(10);
    this->addChild(_unitPanel);

    _unitNameLabel = Label::createWithSystemFont("Unit Name", "Arial-BoldMT", 22);
    _unitNameLabel->setAnchorPoint(Vec2(0, 1));
    _unitNameLabel->setPosition(Vec2(15, 145));
    _unitNameLabel->setTextColor(Color4B(255, 255, 200, 255));
    _unitPanel->addChild(_unitNameLabel);

    _unitStatLabel = Label::createWithSystemFont("Stats...", "Arial", 16);
    _unitStatLabel->setAnchorPoint(Vec2(0, 1));
    _unitStatLabel->setPosition(Vec2(15, 105));
    _unitStatLabel->setDimensions(230, 0);
    _unitStatLabel->setTextColor(Color4B(200, 230, 255, 255));
    _unitPanel->addChild(_unitStatLabel);

    // 4. 建城按钮 (修复交互核心)
    _btnBuildCity = Button::create();
    SETUP_BUTTON(_btnBuildCity, Size(160, 45));
    _btnBuildCity->setPosition(Vec2(visibleSize.width / 2, 100));
    _btnBuildCity->setVisible(false);
    _btnBuildCity->setLocalZOrder(20);

    // 按钮背景渲染
    auto buildCityBg = LayerColor::create(Color4B(50, 100, 50, 255), 160, 45);
    buildCityBg->setIgnoreAnchorPointForPosition(false);
    buildCityBg->setAnchorPoint(Vec2(0.5, 0.5));
    buildCityBg->setPosition(Vec2(80, 22.5));
    _btnBuildCity->addChild(buildCityBg, -1);

    _btnBuildCity->setTitleText("FOUND CITY");
    _btnBuildCity->setTitleFontSize(18);
    _btnBuildCity->setTitleColor(Color3B(255, 215, 0));

    _btnBuildCity->addClickEventListener([this](Ref* sender) {
        CCLOG("HUD: Build City button clicked!");
        if (_onBuildCity) _onBuildCity();
        });
    this->addChild(_btnBuildCity);

    // 5. 事件监听：处理 UI 面板关闭
    auto dispatcher = Director::getInstance()->getEventDispatcher();
    dispatcher->addEventListenerWithSceneGraphPriority(
        EventListenerCustom::create("tech_tree_closed", [this](EventCustom*) { this->closeTechTree(); }), this);
    dispatcher->addEventListenerWithSceneGraphPriority(
        EventListenerCustom::create("culture_tree_closed", [this](EventCustom*) { this->closeCultureTree(); }), this);
    dispatcher->addEventListenerWithSceneGraphPriority(
        EventListenerCustom::create("policy_panel_closed", [this](EventCustom*) { this->closePolicyPanel(); }), this);

    return true;
}

void HUDLayer::createCiv6StyleResourceDisplay() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 顶部背景条：文明6风格通常是深蓝/黑色半透明，带细金边
    _topBar = LayerColor::create(Color4B(15, 20, 30, 240), visibleSize.width, 45);
    _topBar->setPosition(Vec2(0, visibleSize.height - 45));
    _topBar->setLocalZOrder(5);
    this->addChild(_topBar);

    // 装饰线：顶部一条细细的金色边框
    auto goldLine = LayerColor::create(Color4B(180, 160, 80, 200), visibleSize.width, 2);
    goldLine->setPosition(0, 0); // 放在顶部条的下边缘
    _topBar->addChild(goldLine);

    float startX = 40;
    float spacing = 190;

    auto createResourceItem = [this](Node*& container, Label*& label, const std::string& defaultPath, float x, const std::string& initialText) {
        container = Node::create();
        container->setPosition(Vec2(x, 22.5));
        _topBar->addChild(container);

        auto icon = Sprite::create(defaultPath);
        if (icon) {
            icon->setScale(0.8f);
            icon->setPosition(Vec2(0, 0));
            container->addChild(icon);
        }
        else {
            auto placeholder = DrawNode::create();
            placeholder->drawSolidCircle(Vec2::ZERO, 15, 0, 16, Color4F::GRAY);
            container->addChild(placeholder);
        }

        // 初始化时直接应用统一的格式逻辑
        label = Label::createWithSystemFont(initialText, "Arial-BoldMT", 18);
        label->setAnchorPoint(Vec2(0, 0.5));
        label->setPosition(Vec2(25, 0));
        container->addChild(label);
        };

    // 统一初始化文字逻辑
    createResourceItem(_goldContainer, _goldLabel, "res/icon_gold.png", startX, "Gold: 0");
    createResourceItem(_scienceContainer, _scienceLabel, "res/icon_science.png", startX + spacing, "Science: 0");
    createResourceItem(_cultureContainer, _cultureLabel, "res/icon_culture.png", startX + spacing * 2, "Culture: 0");

    // 回合数初始化：修正颜色透明度为 255 (原本是 0)
    _turnLabel = Label::createWithSystemFont("Turn 1", "Arial-BoldMT", 18);
    _turnLabel->setAnchorPoint(Vec2(1, 0.5));
    _turnLabel->setPosition(Vec2(visibleSize.width - 40, 22.5));
    _turnLabel->setTextColor(Color4B(255, 255, 200, 255)); // 确保透明度为 255
    _topBar->addChild(_turnLabel);
}

void HUDLayer::createCiv6StyleNextTurnButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建按钮，指定三种状态的图片路径
    // 路径：res/next_turn_normal.png, res/next_turn_pressed.png, res/next_turn_disabled.png
    _btnNextTurn = Button::create("res/next_turn_normal.png",
        "res/next_turn_pressed.png",
        "res/next_turn_disabled.png");

    // 2. 基础属性设置
    _btnNextTurn->ignoreContentAdaptWithSize(false);
    _btnNextTurn->setContentSize(Size(100, 100)); // 建议美术按 200x200 制作，此处缩放
    _btnNextTurn->setPosition(Vec2(visibleSize.width - 80, 80));
    _btnNextTurn->setLocalZOrder(20);
    _btnNextTurn->setTouchEnabled(true);
    _btnNextTurn->setSwallowTouches(true);

    // 3. 添加文字标签 (居中或偏下)
    auto nextTurnLabel = Label::createWithSystemFont("NEXT TURN", "Arial-BoldMT", 14);
    nextTurnLabel->setPosition(Vec2(50, -10)); // 放在圆钮下方
    nextTurnLabel->setTextColor(Color4B(255, 255, 200, 255));
    nextTurnLabel->enableOutline(Color4B::BLACK, 1);
    _btnNextTurn->addChild(nextTurnLabel);

    // 4. 点击事件
    _btnNextTurn->addClickEventListener([this](Ref* sender) {
        // 简单的点击反馈：轻微缩放
        auto scale = Sequence::create(ScaleTo::create(0.05f, 0.9f), ScaleTo::create(0.1f, 1.0f), nullptr);
        _btnNextTurn->runAction(scale);

        CCLOG("HUD: Next Turn Clicked");
        if (_onNextTurn) _onNextTurn();
        });

    this->addChild(_btnNextTurn);

    // 初始状态：如果没有图片文件，暂时用颜色块辅助调试
    if (_btnNextTurn->getRendererNormal()->getContentSize().width == 0) {
        auto debugCircle = DrawNode::create();
        debugCircle->drawSolidCircle(Vec2(50, 50), 45, 0, 32, Color4F(0.2f, 0.5f, 0.2f, 1.0f));
        _btnNextTurn->addChild(debugCircle, -1);
    }
}

void HUDLayer::createCiv6StyleFunctionButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float startY = visibleSize.height - 100;
    float startX = 45;

    // 使用 [=, &this] 确保能访问 startX 和类的成员变量
    // 这里 btn 的参数改为由外部指针的地址或引用传入
    auto createCiv6Btn = [=](Button*& btn, const std::string& iconText, Color4F themeColor, float y, std::function<void()> cb) {
        btn = Button::create();
        SETUP_BUTTON(btn, Size(64, 64));
        btn->setPosition(Vec2(startX, y)); // 现在 startX 可以正常访问了

        auto drawNode = DrawNode::create();
        Vec2 center(32, 32);

        // 绘制文明6风格背景
        drawNode->drawSolidCircle(center, 30, 0, 32, Color4F(0, 0, 0, 0.6f));
        drawNode->drawSolidCircle(center, 26, 0, 32, themeColor);
        drawNode->drawCircle(center, 30, 0, 32, false, Color4F(0.8f, 0.7f, 0.3f, 1.0f));
        drawNode->drawCircle(center, 27, 0, 32, false, Color4F(1.0f, 0.9f, 0.5f, 0.8f));
        btn->addChild(drawNode, -1);

        btn->setTitleText(iconText);
        btn->setTitleFontSize(28);
        btn->setTitleFontName("Arial-BoldMT");

        // 点击反馈动画
        btn->addTouchEventListener([btn](Ref* sender, Widget::TouchEventType type) {
            if (type == Widget::TouchEventType::BEGAN) btn->runAction(ScaleTo::create(0.05f, 0.9f));
            else if (type == Widget::TouchEventType::ENDED || type == Widget::TouchEventType::CANCELED)
                btn->runAction(ScaleTo::create(0.1f, 1.0f));
            });

        btn->addClickEventListener([cb](Ref*) { cb(); });
        this->addChild(btn, 15);
        };

    // 调用时直接传入成员变量引用
    createCiv6Btn(_btnTechTree, "S", Color4F(0.1f, 0.4f, 0.7f, 0.8f), startY, [this]() { this->openTechTree(); });
    createCiv6Btn(_btnCultureTree, "C", Color4F(0.5f, 0.2f, 0.6f, 0.8f), startY - 75, [this]() { this->openCultureTree(); });
    createCiv6Btn(_btnPolicySystem, "G", Color4F(0.2f, 0.5f, 0.2f, 0.8f), startY - 150, [this]() { this->openPolicyPanel(); });
}

void HUDLayer::updateResources(int gold, int science, int culture, int turn) {
    _turnLabel->setString("Turn " + std::to_string(turn));
    _goldLabel->setString("Gold: " + std::to_string(gold));
    _scienceLabel->setString("Science: " + std::to_string(science));
    _cultureLabel->setString("Culture: " + std::to_string(culture));

    // 同步更新子面板数据
    updateSciencePerTurn(science / 10 + 1);
    updateCulturePerTurn(culture / 10 + 1);
}

void HUDLayer::showUnitInfo(AbstractUnit* unit) {
    if (!unit) return;
    _unitPanel->setVisible(true);
    _unitNameLabel->setString(unit->getUnitName());
    _unitStatLabel->setString("Attack: " + std::to_string(unit->getBaseAttack()) +
        "\nMoves: " + std::to_string(unit->getMaxMoves()));

    if (unit->canFoundCity()) {
        _btnBuildCity->setVisible(true);
        _btnBuildCity->setTouchEnabled(true);
    }
    else {
        _btnBuildCity->setVisible(false);
    }
}

void HUDLayer::hideUnitInfo() {
    _unitPanel->setVisible(false);
    _btnBuildCity->setVisible(false);
}

// --- 回调与面板管理 ---

void HUDLayer::setNextTurnCallback(const std::function<void()>& cb) { _onNextTurn = cb; }
void HUDLayer::setBuildCityCallback(const std::function<void()>& cb) { _onBuildCity = cb; }

void HUDLayer::openTechTree() {
    if (_isTechTreeOpen || !_techTree) return;
    _techTreePanel = TechTreePanel::create();
    _techTreePanel->setTechTree(_techTree);
    this->addChild(_techTreePanel, 200);
    _isTechTreeOpen = true;
    _btnTechTree->setEnabled(false);
    _btnTechTree->setOpacity(150);
}

void HUDLayer::closeTechTree() {
    if (!_isTechTreeOpen || !_techTreePanel) return;
    _techTreePanel->removeFromParent();
    _techTreePanel = nullptr;
    _isTechTreeOpen = false;
    _btnTechTree->setEnabled(true);
    _btnTechTree->setOpacity(255);
}

void HUDLayer::setTechTree(TechTree* techTree) { _techTree = techTree; }

void HUDLayer::updateSciencePerTurn(int science) {
    if (_techTreePanel && _isTechTreeOpen) _techTreePanel->setSciencePerTurn(science);
}

void HUDLayer::openCultureTree() {
    if (_isCultureTreeOpen || !_cultureTree) return;
    _cultureTreePanel = CultureTreePanel::create();
    _cultureTreePanel->setCultureTree(_cultureTree);
    this->addChild(_cultureTreePanel, 200);
    _isCultureTreeOpen = true;
    _btnCultureTree->setEnabled(false);
}

void HUDLayer::closeCultureTree() {
    if (!_isCultureTreeOpen || !_cultureTreePanel) return;
    _cultureTreePanel->removeFromParent();
    _cultureTreePanel = nullptr;
    _isCultureTreeOpen = false;
    _btnCultureTree->setEnabled(true);
}

void HUDLayer::setCultureTree(CultureTree* cultureTree) { _cultureTree = cultureTree; }

void HUDLayer::updateCulturePerTurn(int culture) {
    if (_cultureTreePanel && _isCultureTreeOpen) _cultureTreePanel->setCulturePerTurn(culture);
}

void HUDLayer::openPolicyPanel() {
    if (_isPolicyPanelOpen || !_policyManager) return;
    _policyPanel = PolicyPanel::create();
    _policyPanel->setPolicyManager(_policyManager);
    _policyPanel->setCultureTree(_cultureTree);
    this->addChild(_policyPanel, 100);
    _isPolicyPanelOpen = true;
    _btnPolicySystem->setEnabled(false);
}

void HUDLayer::closePolicyPanel() {
    if (!_isPolicyPanelOpen || !_policyPanel) return;
    _policyPanel->removeFromParent();
    _policyPanel = nullptr;
    _isPolicyPanelOpen = false;
    _btnPolicySystem->setEnabled(true);
}

void HUDLayer::setPolicyManager(PolicyManager* policyManager) { _policyManager = policyManager; }