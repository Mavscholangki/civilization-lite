#include "HUDLayer.h"

USING_NS_CC;
using namespace cocos2d::ui;

bool HUDLayer::init() {
    if (!Layer::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // --- 1. 顶部资源栏 ---
    auto topBar = Layout::create();
    topBar->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    topBar->setBackGroundColor(Color3B(20, 20, 20));
    topBar->setBackGroundColorOpacity(200); // 半透明
    topBar->setContentSize(Size(visibleSize.width, 40));
    topBar->setPosition(Vec2(0, visibleSize.height - 40));
    this->addChild(topBar);

    _resLabel = Label::createWithSystemFont("Turn: 1  |  Gold: 0  |  Science: 0", "Arial", 18);
    _resLabel->setAnchorPoint(Vec2(0.5, 0.5));
    _resLabel->setPosition(Vec2(visibleSize.width / 2, 20)); // 相对于 topBar
    topBar->addChild(_resLabel);

    // --- 2. 下一回合按钮 ---
    auto btnNext = Button::create();
    btnNext->setTitleText("[ NEXT TURN ]");
    btnNext->setTitleFontSize(30);
    btnNext->setTitleColor(Color3B::GREEN);
    btnNext->setPosition(Vec2(visibleSize.width * 6 / 7 - 100, 60));
    btnNext->addClickEventListener([this](Ref*) {
        if (_onNextTurn) _onNextTurn();
        });
    this->addChild(btnNext);

    // --- 3. 单位详情面板 (初始隐藏) ---
    _unitPanel = Layout::create();
    _unitPanel->setContentSize(Size(250, 150));
    _unitPanel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    _unitPanel->setBackGroundColor(Color3B(0, 0, 50)); // 深蓝底色
    _unitPanel->setBackGroundColorOpacity(220);
    _unitPanel->setPosition(Vec2(20, 20)); // 左下角
    _unitPanel->setVisible(false); // 默认不显示
    this->addChild(_unitPanel);

    // --- 4. 建城按钮 (初始隐藏) ---
    _btnBuildCity = Button::create();
    _btnBuildCity->setTitleText("[ FOUND CITY ]");
    _btnBuildCity->setTitleFontSize(24);
    _btnBuildCity->setTitleColor(Color3B::MAGENTA);
    _btnBuildCity->setPosition(Vec2(visibleSize.width / 2, 60)); // 屏幕下方居中
    _btnBuildCity->setVisible(false); // 默认看不见
    _btnBuildCity->addClickEventListener([this](Ref*) {
        if (_onBuildCity) _onBuildCity();
        });
    this->addChild(_btnBuildCity);

    // --- 5. 科技树按钮（新增）---
    _btnTechTree = Button::create();
    _btnTechTree->setTitleText("[ TECH TREE ]");
    _btnTechTree->setTitleFontSize(20);
    _btnTechTree->setTitleColor(Color3B::BLUE);
    _btnTechTree->setPosition(Vec2(100, visibleSize.height - 70)); // 屏幕左上角
    _btnTechTree->addClickEventListener([this](Ref*) {
        this->openTechTree();
        });
    this->addChild(_btnTechTree);
    _techTreePanel = nullptr;
    _techTree = nullptr;
    _isTechTreeOpen = false;
    // --- 监听科技树关闭事件 ---
    auto listener = cocos2d::EventListenerCustom::create("tech_tree_closed",
        [this](cocos2d::EventCustom* event) {
            this->closeTechTree();
        });
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);


    // 面板里的内容
    _unitNameLabel = Label::createWithSystemFont("Unit Name", "Arial", 22);
    _unitNameLabel->setAnchorPoint(Vec2(0, 1));
    _unitNameLabel->setPosition(Vec2(10, 140));
    _unitPanel->addChild(_unitNameLabel);

    _unitStatLabel = Label::createWithSystemFont("Stats...", "Arial", 16);
    _unitStatLabel->setAnchorPoint(Vec2(0, 1));
    _unitStatLabel->setPosition(Vec2(10, 100));
    _unitStatLabel->setWidth(230); // 自动换行
    _unitPanel->addChild(_unitStatLabel);

    return true;
}

void HUDLayer::showUnitInfo(AbstractUnit* unit) {
    if (!unit) return;

    _unitPanel->setVisible(true);
    _unitNameLabel->setString(unit->getUnitName());

    // 拼接属性字符串
    std::string stats = "Attack: " + std::to_string(unit->getBaseAttack()) + "\n" +
        "Moves: " + std::to_string(unit->getMaxMoves());

    _unitStatLabel->setString(stats);
    if (unit->canFoundCity()) {
        _btnBuildCity->setVisible(true);
    }
    else {
        _btnBuildCity->setVisible(false);
    }
}

void HUDLayer::hideUnitInfo() {
    _unitPanel->setVisible(false);
    _btnBuildCity->setVisible(false); // 隐藏按钮
}

void HUDLayer::setNextTurnCallback(const std::function<void()>& cb) { _onNextTurn = cb; }

void HUDLayer::updateResources(int gold, int science, int turn) {
    char buf[100];
    sprintf(buf, "Turn: %d  |  Gold: %d  |  Science: %d", turn, gold, science);
    _resLabel->setString(buf);

    updateSciencePerTurn(science);
}

void HUDLayer::setBuildCityCallback(const std::function<void()>& cb) { _onBuildCity = cb; }

// 新增方法 - 科技树相关
void HUDLayer::openTechTree() {
    if (_isTechTreeOpen || !_techTree) return;

    // 创建科技树面板
    _techTreePanel = TechTreePanel::create();
    if (!_techTreePanel) return;

    // 设置科技系统
    _techTreePanel->setTechTree(_techTree);

    // 添加为子节点
    this->addChild(_techTreePanel, 100); // 使用较高的Z-order确保在最前面

    // 更新状态
    _isTechTreeOpen = true;

    // 禁用其他交互
    _btnTechTree->setEnabled(false);
    _btnTechTree->setTitleColor(Color3B::GRAY);
}

void HUDLayer::closeTechTree() {
    if (!_isTechTreeOpen || !_techTreePanel) return;

    // 移除科技树面板
    _techTreePanel->removeFromParent();
    _techTreePanel = nullptr;

    // 更新状态
    _isTechTreeOpen = false;

    // 重新启用按钮
    _btnTechTree->setEnabled(true);
    _btnTechTree->setTitleColor(Color3B::BLUE);
}

void HUDLayer::setTechTree(TechTree* techTree) {
    _techTree = techTree;
}

void HUDLayer::updateSciencePerTurn(int science) {
    if (_techTreePanel && _isTechTreeOpen) {
        _techTreePanel->setSciencePerTurn(science);
    }
}