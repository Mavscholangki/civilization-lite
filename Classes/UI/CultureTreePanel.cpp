#include "CultureTreePanel.h"
#include <sstream>
#include <algorithm>
#include <queue>
#include <queue>
#include <functional>

// CultureTreePanelEventListener 实现
void CultureTreePanelEventListener::onCultureUnlocked(int cultureId, const std::string& cultureName,
    const std::string& effect) {
    if (_owner) {
        _owner->handleCultureUnlocked(cultureId, cultureName, effect);
    }
}

void CultureTreePanelEventListener::onCultureProgress(int cultureId, int currentProgress, int totalCost) {
    if (_owner) {
        _owner->handleCultureProgress(cultureId, currentProgress, totalCost);
    }
}

void CultureTreePanelEventListener::onInspirationTriggered(int cultureId, const std::string& cultureName) {
    if (_owner) {
        _owner->handleInspirationTriggered(cultureId, cultureName);
    }
}

bool CultureTreePanel::init() {
    if (!Layer::init()) {
        return false;
    }

    // 设置触摸监听器
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    listener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    _eventListener = new CultureTreePanelEventListener(this);

    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建背景
    _background = LayerColor::create(Color4B(25, 15, 30, 240));
    _background->setContentSize(visibleSize);
    this->addChild(_background, -1);

    // 创建标题
    auto title = Label::createWithSystemFont(u8"文化树", "Arial", 40);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
    title->setColor(Color3B(230, 200, 255));
    this->addChild(title);

    // 创建滚动视图
    _scrollView = ScrollView::create();
    _scrollView->setContentSize(Size(visibleSize.width - 40, visibleSize.height - 250));
    _scrollView->setPosition(Vec2(20, 150));
    _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _scrollView->setBounceEnabled(true);
    _scrollView->setSwallowTouches(true);
    this->addChild(_scrollView);

    // 创建内容节点
    _contentNode = Node::create();
    _contentNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    _scrollView->addChild(_contentNode);

    // 设置滚动区域
    _scrollView->setInnerContainerSize(Size(COLUMN_SPACING * 10, visibleSize.height - 150));
    _contentNode->setPosition(0, 0);

    // 创建底部控制面板
    _controlPanel = LayerColor::create(Color4B(40, 30, 45, 220), visibleSize.width - 40, 120);
    _controlPanel->setPosition(Vec2(20, 20));
    this->addChild(_controlPanel);

    // 当前政体标签
    _currentGovernmentLabel = Label::createWithSystemFont(u8"当前政体：酋邦", "Arial", 24);
    _currentGovernmentLabel->setPosition(Vec2(visibleSize.width / 4, 80));
    _currentGovernmentLabel->setColor(Color3B(255, 200, 255));
    _controlPanel->addChild(_currentGovernmentLabel);

    // 政策卡槽位标签
    _policySlotsLabel = Label::createWithSystemFont(u8"政策卡槽位：[军0/经0/外0/通0]", "Arial", 20);
    _policySlotsLabel->setPosition(Vec2(visibleSize.width / 4, 45));
    _policySlotsLabel->setColor(Color3B(180, 180, 255));
    _controlPanel->addChild(_policySlotsLabel);

    // 每回合文化标签
    _culturePerTurnLabel = Label::createWithSystemFont(u8"每回合文化：0", "Arial", 22);
    _culturePerTurnLabel->setPosition(Vec2(visibleSize.width / 4 * 3, 65));
    _culturePerTurnLabel->setColor(Color3B(200, 220, 255));
    _controlPanel->addChild(_culturePerTurnLabel);

    // 创建关闭按钮
    auto closeButton = Layout::create();
    closeButton->setContentSize(Size(120, 50));
    closeButton->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    closeButton->setBackGroundColor(Color3B(180, 60, 60));
    closeButton->setBackGroundColorOpacity(200);
    closeButton->setTouchEnabled(true);
    closeButton->setPosition(Vec2(visibleSize.width - 140, visibleSize.height - 50));

    auto closeLabel = Label::createWithSystemFont(u8"关闭", "Arial", 28);
    closeLabel->setPosition(Vec2(60, 25));
    closeLabel->setColor(Color3B::WHITE);
    closeButton->addChild(closeLabel);

    closeButton->addTouchEventListener([this](Ref* sender, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::ENDED) {
            auto event = EventCustom("culture_tree_closed");
            this->getEventDispatcher()->dispatchEvent(&event);
        }
        });
    this->addChild(closeButton);

    return true;
}

void CultureTreePanel::setCultureTree(CultureTree* tree) {
    if (_cultureTree) {
        _cultureTree->removeEventListener(_eventListener);
    }

    _cultureTree = tree;

    if (_cultureTree) {
        _cultureTree->addEventListener(_eventListener);
        refreshUI();
        updateControlPanel();
    }
}

void CultureTreePanel::setCulturePerTurn(int culture) {
    if (_culturePerTurnLabel) {
        std::string text = u8"每回合文化：" + std::to_string(culture);
        _culturePerTurnLabel->setString(text);
    }
}

void CultureTreePanel::setAsCurrentResearch(int cultureId) {
    if (!_cultureTree) return;

    if (_cultureTree->setCurrentResearch(cultureId)) {
        // 设置成功，刷新UI
        refreshUI();
        updateControlPanel();

        // 可以添加音效或提示
    }
}

Node* CultureTreePanel::createCultureNodeUI(const CultureNode* cultureData) {
    if (!cultureData) return nullptr;

    // 创建容器节点
    auto container = Node::create();
    container->setContentSize(Size(NODE_WIDTH, NODE_HEIGHT));

    // 创建背景Layout
    auto background = Layout::create();
    background->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    background->setBackGroundColor(Color3B(64, 64, 89));
    background->setBackGroundColorOpacity(255);
    background->setContentSize(Size(NODE_WIDTH, NODE_HEIGHT));
    background->setTouchEnabled(false);  // 确保背景不拦截触摸
    background->setTag(100);
    container->addChild(background, -1);

    // 创建 MenuItem 并确保它能接收触摸事件
    auto menuItem = MenuItem::create();
    menuItem->setContentSize(Size(NODE_WIDTH, NODE_HEIGHT));
    menuItem->setTag(cultureData->id);

    // 重要：设置回调，让整个区域可点击
    menuItem->setCallback([this, techId = cultureData->id](Ref* sender) {
        CCLOG("Tech node clicked: %d", techId);

        if (!this->_detailPanel || this->_detailPanel->getTag() != techId) {
            this->showCultureDetail(techId);
        }
        else {
            if (this->_cultureTree &&
                this->_cultureTree->isUnlockable(techId) &&
                !this->_cultureTree->isActivated(techId)) {
                this->setAsCurrentResearch(techId);
                this->hideCultureDetail();
            }
        }
        });

    // 创建 Menu - 重要：设置 position 在中心，确保触摸区域正确
    auto menu = Menu::create(menuItem, nullptr);
    menu->setPosition(Vec2(NODE_WIDTH / 2, NODE_HEIGHT / 2));
    menuItem->setPosition(Vec2::ZERO);
    container->addChild(menu);

    // 添加边框
    auto border = DrawNode::create();
    border->drawRect(Vec2(0, 0), Vec2(NODE_WIDTH, NODE_HEIGHT),
        Color4F(1.0f, 1.0f, 1.0f, 0.5f));
    border->setTag(106);
    container->addChild(border);

    // 市政名称
    auto nameLabel = Label::createWithSystemFont(cultureData->name, "Arial", 18);
    nameLabel->setPosition(Vec2(NODE_WIDTH / 2, NODE_HEIGHT - 25));
    nameLabel->setColor(Color3B::WHITE);
    nameLabel->setDimensions(NODE_WIDTH - 10, 40);
    nameLabel->setAlignment(TextHAlignment::CENTER);
    nameLabel->setTag(101);
    container->addChild(nameLabel, 10);

    // 解锁进度（显示为百分比）
    auto progressLabel = Label::createWithSystemFont("0%", "Arial", 16);
    progressLabel->setPosition(Vec2(NODE_WIDTH / 2, NODE_HEIGHT / 2));
    progressLabel->setColor(Color3B(180, 180, 255));
    progressLabel->setTag(102);
    container->addChild(progressLabel, 10);

    // 效果图标或简要文本
    if (!cultureData->unlockedGovernmentList.empty()) {
        auto govIcon = Label::createWithSystemFont(u8"政体", "Arial", 14);
        govIcon->setPosition(Vec2(NODE_WIDTH / 2, NODE_HEIGHT / 4));
        govIcon->setColor(Color3B(255, 200, 100));
        govIcon->setTag(103);
        container->addChild(govIcon, 10);
    }

    // 政策卡槽位图标
    auto policyIcon = Label::createWithSystemFont(u8"政策", "Arial", 14);
    policyIcon->setPosition(Vec2(NODE_WIDTH / 4, 15));
    policyIcon->setColor(Color3B(150, 220, 150));
    policyIcon->setTag(104);
    container->addChild(policyIcon, 10);

    // 进度条背景
    auto progressBg = DrawNode::create();
    progressBg->drawSolidRect(Vec2(5, 35), Vec2(NODE_WIDTH - 5, 45),
        Color4F(0.2f, 0.2f, 0.2f, 1.0f));
    progressBg->setTag(105);
    container->addChild(progressBg, 5);

    // 进度条
    auto progressBar = DrawNode::create();
    progressBar->setTag(107);
    container->addChild(progressBar, 6);

    return container;
}

void CultureTreePanel::calculateCultureDepths() {
    if (!_cultureTree) return;

    // 初始化所有市政
    std::vector<int> allCultures = {
        101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113
    };

    // 计算深度
    for (int cultureId : allCultures) {
        auto cultureInfo = _cultureTree->getCultureInfo(cultureId);
        if (!cultureInfo) continue;

        if (cultureInfo->srcCultureList.empty()) {
            _cultureDepth[cultureId] = 0;
            _cultureByDepth[0].push_back(cultureId);
        }
        else {
            int maxDepth = -1;
            for (int parentId : cultureInfo->srcCultureList) {
                maxDepth = std::max(maxDepth, _cultureDepth[parentId] + 1);
            }
            _cultureDepth[cultureId] = maxDepth;
            _cultureByDepth[maxDepth].push_back(cultureId);
        }
    }
}

void CultureTreePanel::createSplineConnection(int fromCultureId, int toCultureId) {
    auto fromNode = _nodeUIMap[fromCultureId];
    auto toNode = _nodeUIMap[toCultureId];

    if (!fromNode || !toNode) return;

    Vec2 startPos = fromNode->getPosition() + Vec2(NODE_WIDTH, NODE_HEIGHT / 2);
    Vec2 endPos = toNode->getPosition() + Vec2(0, NODE_HEIGHT / 2);

    auto drawNode = DrawNode::create();

    // 连线颜色
    Color4F lineColor = Color4F(0.4f, 0.3f, 0.4f, 0.6f); // 默认紫色

    bool fromActivated = _cultureTree->isActivated(fromCultureId);
    bool toActivated = _cultureTree->isActivated(toCultureId);
    int fromProgress = _cultureTree->getCultureProgress(fromCultureId);
    int toProgress = _cultureTree->getCultureProgress(toCultureId);

    if (fromActivated && toActivated) {
        lineColor = Color4F(0.0f, 0.8f, 0.0f, 0.8f); // 绿色：都已激活
    }
    else if (fromActivated && toProgress > 0) {
        lineColor = Color4F(0.9f, 0.7f, 0.1f, 0.9f); // 金色：后续有进度
    }
    else if (fromProgress > 0 && toActivated) {
        lineColor = Color4F(0.9f, 0.7f, 0.1f, 0.9f); // 金色：前导有进度
    }

    // 绘制三段线：水平-垂直-水平
    float midX = (startPos.x + endPos.x) / 2;
    drawNode->drawSegment(startPos, Vec2(midX, startPos.y), 2.0f, lineColor);
    drawNode->drawSegment(Vec2(midX, startPos.y), Vec2(midX, endPos.y), 2.0f, lineColor);
    drawNode->drawSegment(Vec2(midX, endPos.y), endPos, 2.0f, lineColor);

    // 在拐角处添加圆点
    drawNode->drawSolidCircle(Vec2(midX, startPos.y), 4.0f,
        CC_DEGREES_TO_RADIANS(360), 8, lineColor);
    drawNode->drawSolidCircle(Vec2(midX, endPos.y), 4.0f,
        CC_DEGREES_TO_RADIANS(360), 8, lineColor);

    _contentNode->addChild(drawNode, -5);
}

void CultureTreePanel::updateNodeUIState(int cultureId, CultureNodeState state) {
    auto container = _nodeUIMap[cultureId];
    if (!container) return;

    // 获取菜单项
    Menu* menu = nullptr;
    MenuItem* menuItem = nullptr;

    for (auto child : container->getChildren()) {
        menu = dynamic_cast<Menu*>(child);
        if (menu) break;
    }

    if (menu && menu->getChildrenCount() > 0) {
        menuItem = dynamic_cast<MenuItem*>(menu->getChildren().at(0));
    }

    // 根据状态设置 MenuItem
    if (menuItem) {
        switch (state) {
            case CultureNodeState::LOCKED:
                menuItem->setEnabled(false);
                break;
            case CultureNodeState::UNLOCKABLE:
            case CultureNodeState::IN_PROGRESS:
                menuItem->setEnabled(true);
                break;
            case CultureNodeState::RESEARCHING:  // 新增状态
                menuItem->setEnabled(true);
                break;
            case CultureNodeState::ACTIVATED:
                menuItem->setEnabled(false);
                break;
        }
    }

    // 获取UI元素
    auto background = dynamic_cast<Layout*>(container->getChildByTag(100));
    auto nameLabel = dynamic_cast<Label*>(container->getChildByTag(101));
    auto progressLabel = dynamic_cast<Label*>(container->getChildByTag(102));
    auto border = dynamic_cast<DrawNode*>(container->getChildByTag(106));
    auto progressBar = dynamic_cast<DrawNode*>(container->getChildByTag(107));

    if (!background || !nameLabel || !progressLabel || !border) {
        return;
    }

    Color3B bgColor;
    Color3B nameColor = Color3B::WHITE;
    Color3B costColor = Color3B(180, 180, 255);
    Color4F borderColor = Color4F(1.0f, 1.0f, 1.0f, 0.3f);

    // 根据状态设置颜色
    switch (state) {
        case CultureNodeState::LOCKED:
            bgColor = Color3B(64, 64, 89);
            nameColor = Color3B(150, 150, 100);
            costColor = Color3B(120, 120, 180);
            borderColor = Color4F(0.5f, 0.5f, 0.5f, 0.3f);
            break;
        case CultureNodeState::UNLOCKABLE:
            bgColor = Color3B(89, 89, 127);
            borderColor = Color4F(0.8f, 0.8f, 0.2f, 0.8f);
            break;
        case CultureNodeState::RESEARCHING:  // 新增：研究中状态
            bgColor = Color3B(204, 153, 25);  // 金色
            nameColor = Color3B(255, 230, 100);
            borderColor = Color4F(1.0f, 0.9f, 0.2f, 1.0f);
            break;
        case CultureNodeState::IN_PROGRESS:
            bgColor = Color3B(127, 127, 76);
            break;
        case CultureNodeState::ACTIVATED:
            bgColor = Color3B(51, 127, 51);
            nameColor = Color3B(180, 255, 180);
            borderColor = Color4F(0.2f, 0.8f, 0.2f, 1.0f);
            break;
    }

    // 更新背景颜色
    background->setBackGroundColor(bgColor);

    // 更新边框
    border->clear();
    border->drawRect(Vec2(2, 2), Vec2(NODE_WIDTH - 2, NODE_HEIGHT - 2), borderColor);

    // 更新文本颜色
    nameLabel->setColor(nameColor);

    // 更新进度条和标签
    if (_cultureTree && progressLabel && progressBar) {
        int progress = _cultureTree->getCultureProgress(cultureId);
        int cost = _cultureTree->getCultureCost(cultureId);

        // 显示进度百分比
        if (cost > 0) {
            int percent = (progress * 100) / cost;
            progressLabel->setString(std::to_string(percent) + "%");
        }
        else {
            progressLabel->setString("0%");
        }

        if (progress > 0 && cost > 0) {
            float width = (NODE_WIDTH - 10) * ((float)progress / cost);
            progressBar->clear();

            Color4F progressColor;
            if (state == CultureNodeState::ACTIVATED) {
                progressColor = Color4F(0.1f, 0.8f, 0.1f, 1.0f);
            }
            else if (state == CultureNodeState::IN_PROGRESS) {
                progressColor = Color4F(0.8f, 0.4f, 0.8f, 1.0f);
            }
            else {
                progressColor = Color4F(0.4f, 0.4f, 0.8f, 1.0f);
            }

            progressBar->drawSolidRect(Vec2(5, 35), Vec2(5 + width, 45), progressColor);
        }
        else {
            progressBar->clear();
        }
    }
}

std::vector<int> CultureTreePanel::topologicalSort() {
    std::vector<int> result;
    if (!_cultureTree) return result;

    // 收集所有市政ID
    std::vector<int> allCultures = {
        101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113
    };

    // 计算入度
    std::unordered_map<int, int> inDegree;
    std::queue<int> zeroInDegree;

    for (int cultureId : allCultures) {
        auto cultureInfo = _cultureTree->getCultureInfo(cultureId);
        if (cultureInfo) {
            inDegree[cultureId] = static_cast<int>(cultureInfo->srcCultureList.size());
            if (inDegree[cultureId] == 0) {
                zeroInDegree.push(cultureId);
            }
        }
    }

    // 拓扑排序（Kahn算法）
    while (!zeroInDegree.empty()) {
        int current = zeroInDegree.front();
        zeroInDegree.pop();
        result.push_back(current);

        auto cultureInfo = _cultureTree->getCultureInfo(current);
        if (cultureInfo) {
            for (int childId : cultureInfo->dstCultureList) {
                if (--inDegree[childId] == 0) {
                    zeroInDegree.push(childId);
                }
            }
        }
    }

    return result;
}

void CultureTreePanel::showCultureDetail(int cultureId) {
    if (!_cultureTree) return;

    auto cultureInfo = _cultureTree->getCultureInfo(cultureId);
    if (!cultureInfo) return;

    // 隐藏旧的详情面板
    hideCultureDetail();

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建详情面板
    _detailPanel = LayerColor::create(Color4B(0, 0, 0, 220), 400, 300);
    float panelX = visibleSize.width - 420;
    float panelY = 150;
    _detailPanel->setPosition(Vec2(panelX, panelY));
    _detailPanel->setName("detail_panel");
    this->addChild(_detailPanel, 10);

    // 市政名称
    auto nameLabel = Label::createWithSystemFont(cultureInfo->name, "Arial", 28);
    nameLabel->setPosition(Vec2(200, 250));
    nameLabel->setColor(Color3B(255, 200, 255));
    _detailPanel->addChild(nameLabel);

    // 效果描述
    auto effectLabel = Label::createWithSystemFont(cultureInfo->effectDescription, "Arial", 18);
    effectLabel->setPosition(Vec2(200, 180));
    effectLabel->setDimensions(360, 100);
    effectLabel->setAlignment(TextHAlignment::LEFT);
    effectLabel->setVerticalAlignment(TextVAlignment::TOP);
    effectLabel->setColor(Color3B::WHITE);
    _detailPanel->addChild(effectLabel);

    // 解锁的政体
    if (!cultureInfo->unlockedGovernmentList.empty()) {
        std::string govText = u8"解锁政体：";
        for (auto gov : cultureInfo->unlockedGovernmentList) {
            switch (gov) {
                case GovernmentType::CHIEFDOM: govText += u8"酋邦 "; break;
                case GovernmentType::AUTOCRACY: govText += u8"独裁 "; break;
                case GovernmentType::OLIGARCHY: govText += u8"寡头 "; break;
                case GovernmentType::CLASSICAL_REPUBLIC: govText += u8"古典共和 "; break;
                case GovernmentType::MONARCHY: govText += u8"君主 "; break;
                case GovernmentType::THEOCRACY: govText += u8"神权 "; break;
                case GovernmentType::MERCHANT_REPUBLIC: govText += u8"商人共和国 "; break;
                case GovernmentType::DEMOCRACY: govText += u8"民主 "; break;
                case GovernmentType::COMMUNISM: govText += u8"共产主义 "; break;
                case GovernmentType::FASCISM: govText += u8"法西斯 "; break;
                case GovernmentType::CORPORATE_LIBERTY: govText += u8"公司自由 "; break;
                case GovernmentType::DIGITAL_DEMOCRACY: govText += u8"数字民主 "; break;
            }
        }

        auto govLabel = Label::createWithSystemFont(govText, "Arial", 16);
        govLabel->setPosition(Vec2(200, 120));
        govLabel->setDimensions(360, 60);
        govLabel->setColor(Color3B(255, 255, 150));
        _detailPanel->addChild(govLabel);
    }

    // 政策卡槽位
    std::string policyText = u8"政策卡槽位：";
    policyText += u8"军事" + std::to_string(cultureInfo->policySlotCount[0]) + " ";
    policyText += u8"经济" + std::to_string(cultureInfo->policySlotCount[1]) + " ";
    policyText += u8"外交" + std::to_string(cultureInfo->policySlotCount[2]) + " ";
    policyText += u8"通用" + std::to_string(cultureInfo->policySlotCount[3]);

    auto policyLabel = Label::createWithSystemFont(policyText, "Arial", 16);
    policyLabel->setPosition(Vec2(200, 80));
    policyLabel->setColor(Color3B(150, 220, 150));
    _detailPanel->addChild(policyLabel);

    // 当前进度
    int progress = _cultureTree->getCultureProgress(cultureId);
    int cost = _cultureTree->getCultureCost(cultureId);
    int percent = (cost > 0) ? (progress * 100 / cost) : 0;
    std::string progressText = u8"当前进度：" + std::to_string(progress) + "/" +
        std::to_string(cost) + " (" + std::to_string(percent) + "%)";
    auto progressLabel = Label::createWithSystemFont(progressText, "Arial", 18);
    progressLabel->setPosition(Vec2(200, 50));
    progressLabel->setColor(Color3B(255, 200, 100));
    _detailPanel->addChild(progressLabel);

    // 状态和操作按钮
    std::string statusText;
    if (_cultureTree->isActivated(cultureId)) {
        statusText = u8"已解锁";
    }
    else if (_cultureTree->getCurrentResearch() == cultureId) {
        statusText = u8"研究中";
    }
    else if (progress > 0) {
        statusText = u8"有进度";
    }
    else if (_cultureTree->isUnlockable(cultureId)) {
        statusText = u8"可解锁";

        // 添加研究按钮
        auto researchButton = ui::Button::create();
        researchButton->setTitleText(u8"设为当前研究");
        researchButton->setTitleFontSize(18);
        researchButton->setTitleColor(Color3B::WHITE);
        researchButton->setContentSize(Size(150, 40));
        researchButton->setPosition(Vec2(200, 20));
        researchButton->setTag(cultureId);
        researchButton->addClickEventListener([this](Ref* sender) {
            auto button = dynamic_cast<ui::Button*>(sender);
            if (button) {
                int cultureId = button->getTag();
                this->setAsCurrentResearch(cultureId);
                this->hideCultureDetail();
            }
            });
        _detailPanel->addChild(researchButton);
    }
    else {
        statusText = u8"锁定";
    }

    if (_cultureTree->getCurrentResearch() != cultureId && !_cultureTree->isActivated(cultureId) &&
        _cultureTree->isUnlockable(cultureId)) {
        // 已经添加过按钮，不需要再添加状态标签
    }
    else {
        auto statusLabel = Label::createWithSystemFont(statusText, "Arial", 18);
        statusLabel->setPosition(Vec2(200, 20));
        statusLabel->setColor(Color3B(255, 200, 100));
        _detailPanel->addChild(statusLabel);
    }
}

void CultureTreePanel::hideCultureDetail() {
    if (_detailPanel) {
        _detailPanel->removeFromParent();
        _detailPanel = nullptr;
    }
}

void CultureTreePanel::updateControlPanel() {
    if (!_cultureTree) return;

    // 更新当前政体
    GovernmentType currentGov = _cultureTree->getCurrentGovernment();
    std::string govText = u8"当前政体：";
    switch (currentGov) {
        case GovernmentType::CHIEFDOM: govText += u8"酋邦"; break;
        case GovernmentType::AUTOCRACY: govText += u8"独裁统治"; break;
        case GovernmentType::OLIGARCHY: govText += u8"寡头政体"; break;
        case GovernmentType::CLASSICAL_REPUBLIC: govText += u8"古典共和"; break;
        case GovernmentType::MONARCHY: govText += u8"君主制"; break;
        case GovernmentType::THEOCRACY: govText += u8"神权政治"; break;
        case GovernmentType::MERCHANT_REPUBLIC: govText += u8"商人共和国"; break;
        case GovernmentType::DEMOCRACY: govText += u8"民主制"; break;
        case GovernmentType::COMMUNISM: govText += u8"共产主义"; break;
        case GovernmentType::FASCISM: govText += u8"法西斯主义"; break;
        case GovernmentType::CORPORATE_LIBERTY: govText += u8"公司自由制"; break;
        case GovernmentType::DIGITAL_DEMOCRACY: govText += u8"数字民主"; break;
    }
    _currentGovernmentLabel->setString(govText);

    // 更新政策卡槽位
    const int* policySlots = _cultureTree->getActivePolicySlots();
    std::string policyText = u8"政策卡槽位：[军" + std::to_string(policySlots[0]) +
        u8"/经" + std::to_string(policySlots[1]) +
        u8"/外" + std::to_string(policySlots[2]) +
        u8"/通" + std::to_string(policySlots[3]) + "]";
    _policySlotsLabel->setString(policyText);
}

void CultureTreePanel::createInspirationEffect(int cultureId) {
    auto node = _nodeUIMap[cultureId];
    if (!node) return;

    Vec2 nodePos = node->getPosition();
    Vec2 centerPos = nodePos + Vec2(NODE_WIDTH / 2, NODE_HEIGHT / 2);

    // 创建闪光效果
    auto flash = Sprite::create();
    auto drawNode = DrawNode::create();
    drawNode->drawSolidCircle(Vec2::ZERO, NODE_WIDTH / 2 + 10,
        CC_DEGREES_TO_RADIANS(360), 30,
        Color4F(0.8f, 0.6f, 1.0f, 0.8f)); // 紫色闪光
    flash->addChild(drawNode);
    flash->setPosition(centerPos);
    flash->setScale(0.5f);

    auto scaleUp = ScaleTo::create(0.3f, 1.8f);
    auto fadeOut = FadeOut::create(0.3f);
    auto spawn = Spawn::create(scaleUp, fadeOut, nullptr);
    auto remove = CallFunc::create([flash]() { flash->removeFromParent(); });
    flash->runAction(Sequence::create(spawn, remove, nullptr));

    _contentNode->addChild(flash, 5);

    // 创建"灵感！"文字效果
    auto inspirationLabel = Label::createWithSystemFont(u8"灵感！", "Arial", 28);
    inspirationLabel->setColor(Color3B(200, 150, 255));
    inspirationLabel->enableShadow(Color4B(0, 0, 0, 128), Size(2, -2));
    inspirationLabel->setPosition(centerPos + Vec2(0, 80));
    inspirationLabel->setOpacity(0);

    auto fadeIn = FadeIn::create(0.2f);
    auto moveUp = MoveBy::create(0.8f, Vec2(0, 50));
    auto fadeOut2 = FadeOut::create(0.3f);
    auto remove2 = CallFunc::create([inspirationLabel]() { inspirationLabel->removeFromParent(); });
    inspirationLabel->runAction(Sequence::create(fadeIn, moveUp, fadeOut2, remove2, nullptr));

    _contentNode->addChild(inspirationLabel, 10);
}

void CultureTreePanel::handleCultureUnlocked(int cultureId, const std::string& cultureName, const std::string& effect) {
    updateNodeUIState(cultureId, CultureNodeState::ACTIVATED);
    updateConnectionLines();
    updateControlPanel();

    // 播放解锁特效
    auto node = _nodeUIMap[cultureId];
    if (node) {
        auto flash = LayerColor::create(Color4B(100, 200, 100, 100), NODE_WIDTH, NODE_HEIGHT);
        flash->setPosition(node->getPosition());
        flash->setOpacity(0);

        auto fadeIn = FadeTo::create(0.2f, 150);
        auto fadeOut = FadeTo::create(0.3f, 0);
        auto remove = CallFunc::create([flash]() { flash->removeFromParent(); });
        flash->runAction(Sequence::create(fadeIn, fadeOut, remove, nullptr));

        _contentNode->addChild(flash, 5);
    }
}

void CultureTreePanel::handleCultureProgress(int cultureId, int progress, int totalCost) {
    auto container = _nodeUIMap[cultureId];
    if (!container) return;

    auto progressLabel = dynamic_cast<Label*>(container->getChildByTag(102));
    auto progressBar = dynamic_cast<DrawNode*>(container->getChildByTag(107));

    if (progressLabel && totalCost > 0) {
        int percent = (progress * 100) / totalCost;
        progressLabel->setString(std::to_string(percent) + "%");
    }

    if (progressBar) {
        progressBar->clear();

        if (progress > 0 && totalCost > 0) {
            float width = (NODE_WIDTH - 10) * ((float)progress / totalCost);
            Color4F progressColor = Color4F(0.8f, 0.4f, 0.8f, 1.0f);

            if (_cultureTree && _cultureTree->isActivated(cultureId)) {
                progressColor = Color4F(0.1f, 0.8f, 0.1f, 1.0f);
            }
            else if (_cultureTree && _cultureTree->getCurrentResearch() == cultureId) {
                progressColor = Color4F(0.9f, 0.7f, 0.1f, 1.0f);
            }

            progressBar->drawSolidRect(Vec2(5, 35), Vec2(5 + width, 45), progressColor);
        }
        else {
            progressBar->clear();
        }
    }

    // 更新节点状态
    CultureNodeState state = CultureNodeState::LOCKED;
    if (_cultureTree->isActivated(cultureId)) {
        state = CultureNodeState::ACTIVATED;
    }
    else if (_cultureTree->getCurrentResearch() == cultureId) {
        state = CultureNodeState::RESEARCHING;
    }
    else if (progress > 0) {
        state = CultureNodeState::IN_PROGRESS;
    }
    else if (_cultureTree->isUnlockable(cultureId)) {
        state = CultureNodeState::UNLOCKABLE;
    }

    updateNodeUIState(cultureId, state);
}

void CultureTreePanel::handleInspirationTriggered(int cultureId, const std::string& cultureName) {
    createInspirationEffect(cultureId);

    // 更新节点状态
    if (_cultureTree) {
        CultureNodeState state = CultureNodeState::LOCKED;
        if (_cultureTree->isActivated(cultureId)) {
            state = CultureNodeState::ACTIVATED;
        }
        else if (_cultureTree->getCurrentResearch() == cultureId) {
            state = CultureNodeState::RESEARCHING;
        }
        else if (_cultureTree->getCultureProgress(cultureId) > 0) {
            state = CultureNodeState::IN_PROGRESS;
        }
        else if (_cultureTree->isUnlockable(cultureId)) {
            state = CultureNodeState::UNLOCKABLE;
        }

        updateNodeUIState(cultureId, state);
    }
}

// === 绘制列背景函数 ===
void CultureTreePanel::drawColumnBackgrounds() {
    // 找到最大列数
    int maxColumn = 0;
    for (const auto& pair : _cultureDepth) {
        maxColumn = std::max(maxColumn, pair.second);
    }

    // 绘制列背景
    const float COLUMN_WIDTH = 200.0f;
    const float PADDING = 30.0f;
    const float BACKGROUND_HEIGHT = 650.0f;
    const float BACKGROUND_BOTTOM = 80.0f;

    for (int col = 0; col <= maxColumn; col++) {
        float x = 80 + col * COLUMN_SPACING - PADDING;
        float width = COLUMN_WIDTH + PADDING * 2;

        auto columnBg = DrawNode::create();

        // 不同的时代使用不同的背景色
        Color4F bgColor;
        switch (col) {
            case 0: bgColor = Color4F(0.08f, 0.05f, 0.1f, 0.25f); break;    // 远古时代
            case 1: bgColor = Color4F(0.1f, 0.06f, 0.12f, 0.25f); break;   // 古典时代
            case 2: bgColor = Color4F(0.12f, 0.07f, 0.14f, 0.25f); break;  // 中世纪
            case 3: bgColor = Color4F(0.14f, 0.08f, 0.16f, 0.25f); break;  // 文艺复兴
            case 4: bgColor = Color4F(0.16f, 0.09f, 0.18f, 0.25f); break;  // 工业时代
            case 5: bgColor = Color4F(0.18f, 0.1f, 0.2f, 0.25f); break;    // 现代
            default: bgColor = Color4F(0.2f, 0.12f, 0.22f, 0.25f); break;  // 未来时代
        }

        // 绘制圆角矩形列背景
        Rect columnRect(x, BACKGROUND_BOTTOM, width, BACKGROUND_HEIGHT);

        // 绘制四个圆角
        float radius = 12.0f;
        float minX = x;
        float maxX = x + width;
        float minY = BACKGROUND_BOTTOM;
        float maxY = BACKGROUND_BOTTOM + BACKGROUND_HEIGHT;

        // 绘制主体矩形
        columnBg->drawSolidRect(
            Vec2(minX + radius, minY),
            Vec2(maxX - radius, maxY),
            bgColor
        );
        columnBg->drawSolidRect(
            Vec2(minX, minY + radius),
            Vec2(maxX, maxY - radius),
            bgColor
        );

        // 绘制四个圆角
        columnBg->drawSolidCircle(Vec2(minX + radius, minY + radius), radius,
            CC_DEGREES_TO_RADIANS(360), 16, bgColor);
        columnBg->drawSolidCircle(Vec2(maxX - radius, minY + radius), radius,
            CC_DEGREES_TO_RADIANS(360), 16, bgColor);
        columnBg->drawSolidCircle(Vec2(minX + radius, maxY - radius), radius,
            CC_DEGREES_TO_RADIANS(360), 16, bgColor);
        columnBg->drawSolidCircle(Vec2(maxX - radius, maxY - radius), radius,
            CC_DEGREES_TO_RADIANS(360), 16, bgColor);

        _contentNode->addChild(columnBg, -10);

        // 添加列标签（时代名称）
        std::string columnLabel;
        switch (col) {
            case 0: columnLabel = u8"远古时代"; break;
            case 1: columnLabel = u8"古典时代"; break;
            case 2: columnLabel = u8"中世纪"; break;
            case 3: columnLabel = u8"文艺复兴"; break;
            case 4: columnLabel = u8"工业时代"; break;
            case 5: columnLabel = u8"现代"; break;
            case 6: columnLabel = u8"信息时代"; break;
            default: columnLabel = u8"未来时代"; break;
        }

        auto label = Label::createWithSystemFont(columnLabel, "Arial", 20);
        label->setPosition(Vec2(x + width / 2, BACKGROUND_BOTTOM + BACKGROUND_HEIGHT + 25));
        label->setColor(Color3B(220, 200, 255));
        label->enableOutline(Color4B(0, 0, 0, 128), 2);
        _contentNode->addChild(label, -5);
    }
}

// === 调整位置优化连接线 ===
void CultureTreePanel::adjustPositionsForConnections() {
    if (!_cultureTree) return;

    // 获取内容区域的实际高度
    float contentHeight = _scrollView->getInnerContainerSize().height;

    const float MAX_ADJUSTMENT = 60.0f;
    const int MAX_ITERATIONS = 20;
    const float MIN_Y = 120.0f;
    const float MAX_Y = contentHeight - 120.0f;

    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        bool adjusted = false;

        // 收集所有连接线
        std::vector<std::pair<int, int>> connections;
        for (const auto& pair : _nodeUIMap) {
            int fromCultureId = pair.first;
            auto cultureInfo = _cultureTree->getCultureInfo(fromCultureId);
            if (cultureInfo) {
                for (int toCultureId : cultureInfo->dstCultureList) {
                    connections.push_back({ fromCultureId, toCultureId });
                }
            }
        }

        // 调整连接线交叉
        for (size_t i = 0; i < connections.size(); i++) {
            for (size_t j = i + 1; j < connections.size(); j++) {
                auto& conn1 = connections[i];
                auto& conn2 = connections[j];

                // 跳过不是同一列间连接的连接线
                int depthDiff1 = _cultureDepth[conn1.second] - _cultureDepth[conn1.first];
                int depthDiff2 = _cultureDepth[conn2.second] - _cultureDepth[conn2.first];

                if (depthDiff1 != depthDiff2) {
                    continue;
                }

                Vec2 start1 = _culturePositions[conn1.first];
                Vec2 end1 = _culturePositions[conn1.second];
                Vec2 start2 = _culturePositions[conn2.first];
                Vec2 end2 = _culturePositions[conn2.second];

                // 检查连接线是否交叉
                if (doLinesCross(start1, end1, start2, end2)) {
                    // 调整后一个连接的目标节点位置
                    int adjustCulture = conn2.second;
                    Vec2 pos = _culturePositions[adjustCulture];

                    // 根据连接线的相对位置决定调整方向
                    float y1 = (start1.y + end1.y) / 2.0f;
                    float y2 = (start2.y + end2.y) / 2.0f;

                    if (y2 > y1) {
                        pos.y += MAX_ADJUSTMENT * 0.8f;  // 向下调整
                    }
                    else {
                        pos.y -= MAX_ADJUSTMENT * 0.8f;  // 向上调整
                    }

                    // 确保调整后的位置在合理范围内
                    pos.y = std::max(MIN_Y, std::min(pos.y, MAX_Y));

                    _culturePositions[adjustCulture] = pos;
                    adjusted = true;
                }
            }
        }

        // 如果没有调整，退出循环
        if (!adjusted) break;
    }

    // 确保同一列中的节点不会重叠
    for (const auto& columnPair : _cultureByDepth) {
        int column = columnPair.first;
        const auto& culturesInColumn = columnPair.second;

        if (culturesInColumn.size() <= 1) continue;

        // 按Y坐标排序
        std::vector<int> sortedCultures = culturesInColumn;
        std::sort(sortedCultures.begin(), sortedCultures.end(),
            [this](int a, int b) {
                return _culturePositions[a].y < _culturePositions[b].y;
            });

        // 检查并调整重叠的节点
        for (size_t i = 1; i < sortedCultures.size(); i++) {
            int prevId = sortedCultures[i - 1];
            int currId = sortedCultures[i];

            Vec2 prevPos = _culturePositions[prevId];
            Vec2 currPos = _culturePositions[currId];

            // 如果两个节点太接近，调整位置
            float minSpacing = NODE_HEIGHT + 80.0f;
            float currentSpacing = currPos.y - prevPos.y;

            if (currentSpacing < minSpacing) {
                // 计算需要移动的总距离
                float neededSpace = minSpacing - currentSpacing;

                // 平均分配空间，向上移动前一个节点，向下移动后一个节点
                float moveDistance = neededSpace / 2.0f;

                float newPrevY = prevPos.y - moveDistance;
                float newCurrY = currPos.y + moveDistance;

                // 确保不超出边界
                newPrevY = std::max(MIN_Y, std::min(newPrevY, MAX_Y - minSpacing));
                newCurrY = std::max(MIN_Y + minSpacing, std::min(newCurrY, MAX_Y));

                // 更新位置
                _culturePositions[prevId].y = newPrevY;
                _culturePositions[currId].y = newCurrY;
            }
        }
    }
}

// === 更新连接线函数 ===
void CultureTreePanel::updateConnectionLines() {
    if (!_cultureTree) return;

    // 清除所有旧的连接线（非节点元素）
    std::vector<Node*> toRemove;
    for (auto child : _contentNode->getChildren()) {
        // 跳过节点UI（节点UI都有menuItem作为子节点）
        bool isNodeUI = false;
        for (auto grandChild : child->getChildren()) {
            auto menu = dynamic_cast<Menu*>(grandChild);
            if (menu) {
                isNodeUI = true;
                break;
            }
        }

        if (!isNodeUI && dynamic_cast<DrawNode*>(child)) {
            toRemove.push_back(child);
        }
    }

    for (auto node : toRemove) {
        node->removeFromParent();
    }

    // 重新创建所有连接线
    for (const auto& pair : _nodeUIMap) {
        int fromCultureId = pair.first;
        auto cultureInfo = _cultureTree->getCultureInfo(fromCultureId);
        if (cultureInfo) {
            for (int toCultureId : cultureInfo->dstCultureList) {
                createSplineConnection(fromCultureId, toCultureId);
            }
        }
    }
}

// === 节点点击事件处理函数 ===
void CultureTreePanel::onCultureNodeClicked(Ref* sender, Widget::TouchEventType type) {
    if (type != Widget::TouchEventType::ENDED) return;

    // 这个函数是旧的设计，现在使用MenuItem的回调
    // 保持函数定义以避免编译错误
    CCLOG("Old node click handler called");
}

// === 政体切换函数 ===
void CultureTreePanel::switchGovernment(GovernmentType government) {
    if (!_cultureTree) return;

    if (_cultureTree->switchGovernment(government)) {
        // 切换成功，更新UI
        updateControlPanel();

        // 播放政体切换特效
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto govLabel = Label::createWithSystemFont(u8"政体已切换！", "Arial", 32);
        govLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        govLabel->setColor(Color3B(255, 255, 0));
        govLabel->enableOutline(Color4B(0, 0, 0, 128), 3);
        govLabel->setOpacity(0);

        this->addChild(govLabel, 100);

        // 动画：淡入、上浮、淡出
        auto fadeIn = FadeIn::create(0.3f);
        auto moveUp = MoveBy::create(1.5f, Vec2(0, 100));
        auto fadeOut = FadeOut::create(0.5f);
        auto remove = CallFunc::create([govLabel]() { govLabel->removeFromParent(); });
        auto sequence = Sequence::create(fadeIn, Spawn::create(moveUp, nullptr), fadeOut, remove, nullptr);
        govLabel->runAction(sequence);
    }
    else {
        // 切换失败，显示提示
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto errorLabel = Label::createWithSystemFont(u8"政体未解锁！", "Arial", 28);
        errorLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 50));
        errorLabel->setColor(Color3B(255, 100, 100));
        errorLabel->enableOutline(Color4B(0, 0, 0, 128), 2);
        errorLabel->setOpacity(0);

        this->addChild(errorLabel, 100);

        // 动画：淡入、上浮、淡出
        auto fadeIn = FadeIn::create(0.3f);
        auto moveUp = MoveBy::create(1.0f, Vec2(0, 50));
        auto fadeOut = FadeOut::create(0.5f);
        auto remove = CallFunc::create([errorLabel]() { errorLabel->removeFromParent(); });
        auto sequence = Sequence::create(fadeIn, Spawn::create(moveUp, nullptr), fadeOut, remove, nullptr);
        errorLabel->runAction(sequence);
    }
}

// === 修改layoutCultureTree函数中的列间距计算 ===
// 在layoutCultureTree函数中，我们需要确保列宽度足够容纳列背景
void CultureTreePanel::layoutCultureTree() {
    const float BASE_X = 120.0f;  // 增加起始X坐标
    const float BASE_Y = 350.0f;
    float contentHeight = _scrollView->getInnerContainerSize().height;

    // 布局每列的市政
    for (const auto& columnPair : _cultureByDepth) {
        int column = columnPair.first;
        const auto& culturesInColumn = columnPair.second;

        if (culturesInColumn.empty()) continue;

        // 计算可用的垂直空间
        float availableHeight = contentHeight - 200.0f;
        float totalRequiredHeight = culturesInColumn.size() * (NODE_HEIGHT + 80.0f);

        // 动态调整行间距
        float dynamicRowSpacing = ROW_SPACING;
        if (totalRequiredHeight > availableHeight) {
            dynamicRowSpacing = availableHeight / culturesInColumn.size() - 50.0f;
            dynamicRowSpacing = std::max(dynamicRowSpacing, NODE_HEIGHT + 60.0f);
        }

        // 计算中心行
        float centerRow = (culturesInColumn.size() - 1) / 2.0f;

        for (size_t i = 0; i < culturesInColumn.size(); i++) {
            int cultureId = culturesInColumn[i];

            // 计算行偏移
            float rowOffset = static_cast<float>(i) - centerRow;

            // 计算基础位置
            float x = BASE_X + column * COLUMN_SPACING;
            float y = BASE_Y + rowOffset * dynamicRowSpacing;

            // 确保Y坐标在合理范围内
            float minY = 120.0f;  // 增加边距
            float maxY = contentHeight - 120.0f;
            y = std::max(minY, std::min(y, maxY));

            _culturePositions[cultureId] = Vec2(x, y);
        }
    }

    // 调整位置避免重叠
    adjustPositionsForConnections();
}

// === 修改refreshUI函数，确保在绘制列背景后创建节点 ===
void CultureTreePanel::refreshUI() {
    if (!_cultureTree) return;

    // 清除旧UI
    _contentNode->removeAllChildren();
    _nodeUIMap.clear();
    _culturePositions.clear();
    _cultureDepth.clear();
    _cultureByDepth.clear();

    // 第一步：计算拓扑排序
    std::vector<int> sortedCultures = topologicalSort();

    // 第二步：计算文化深度
    calculateCultureDepths();

    // 第三步：布局算法
    layoutCultureTree();

    // 第四步：绘制列背景（先绘制背景，再绘制节点）
    drawColumnBackgrounds();

    // 第五步：创建节点
    for (int cultureId : sortedCultures) {
        auto cultureInfo = _cultureTree->getCultureInfo(cultureId);
        if (cultureInfo) {
            Node* nodeUI = createCultureNodeUI(cultureInfo);
            if (nodeUI) {
                _nodeUIMap[cultureId] = nodeUI;
                _contentNode->addChild(nodeUI);

                // 设置计算好的位置
                Vec2 pos = _culturePositions[cultureId];
                nodeUI->setPosition(pos);
            }
        }
    }

    // 第六步：创建连接线
    for (const auto& pair : _nodeUIMap) {
        int fromCultureId = pair.first;
        auto cultureInfo = _cultureTree->getCultureInfo(fromCultureId);
        if (cultureInfo) {
            for (int toCultureId : cultureInfo->dstCultureList) {
                createSplineConnection(fromCultureId, toCultureId);
            }
        }
    }

    // 第七步：更新节点状态
    auto unlockableCultures = _cultureTree->getUnlockableCultureList();
    int currentResearch = _cultureTree->getCurrentResearch();

    for (const auto& pair : _nodeUIMap) {
        int cultureId = pair.first;

        CultureNodeState state = CultureNodeState::LOCKED;
        if (_cultureTree->isActivated(cultureId)) {
            state = CultureNodeState::ACTIVATED;
        }
        else if (currentResearch == cultureId) {
            state = CultureNodeState::RESEARCHING;
        }
        else if (_cultureTree->getCultureProgress(cultureId) > 0) {
            state = CultureNodeState::IN_PROGRESS;
        }
        else if (std::find(unlockableCultures.begin(), unlockableCultures.end(), cultureId) != unlockableCultures.end()) {
            state = CultureNodeState::UNLOCKABLE;
        }

        updateNodeUIState(cultureId, state);
    }
}

bool CultureTreePanel::doLinesCross(const Vec2& p1, const Vec2& p2, const Vec2& q1, const Vec2& q2) {
    Vec2 r = p2 - p1;
    Vec2 s = q2 - q1;
    float rxs = r.cross(s);
    float qpxr = (q1 - p1).cross(r);

    if (abs(rxs) < 0.0001f) {
        return false;
    }

    float t = (q1 - p1).cross(s) / rxs;
    float u = (q1 - p1).cross(r) / rxs;

    return (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f);
}

void CultureTreePanel::onExit() {
    hideCultureDetail();

    if (_cultureTree && _eventListener) {
        _cultureTree->removeEventListener(_eventListener);
    }

    if (_eventListener) {
        delete _eventListener;
        _eventListener = nullptr;
    }

    Layer::onExit();
}