#include "PolicyPanel.h"

// =========================================================
// 初始化与基础设置
// =========================================================

bool PolicyPanel::init() {
    if (!Layer::init()) return false;

    auto vs = Director::getInstance()->getVisibleSize();

    // 1. 背景
    auto bg = LayerColor::create(Color4B(20, 25, 30, 250), vs.width, vs.height);
    this->addChild(bg);

    // 2. 吞噬点击
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, bg);

    // 3. 关闭按钮
    auto btnClose = Button::create();
    btnClose->setTitleText(u8"关闭");
    btnClose->setTitleFontSize(24);
    btnClose->setPosition(Vec2(vs.width - 60, vs.height - 40));
    btnClose->addClickEventListener([this](Ref*) {
        // 发送关闭事件
        cocos2d::EventCustom event("policy_panel_closed");
        _eventDispatcher->dispatchEvent(&event);
        });
    this->addChild(btnClose, 10);

    initLayout();

    return true;
}

void PolicyPanel::initLayout() {
    auto vs = Director::getInstance()->getVisibleSize();
    float midX = vs.width * 0.4f;

    // --- 左侧面板 ---
    _leftPanel = Node::create();
    this->addChild(_leftPanel);

    // 左侧背景
    auto leftBg = LayerColor::create(Color4B(40, 40, 50, 100), midX, vs.height);
    _leftPanel->addChild(leftBg, -1);

    // 政体标题
    _govLabel = Label::createWithSystemFont(u8"当前政体", "Arial", 28);
    _govLabel->setPosition(midX / 2, vs.height - 50);
    _govLabel->setColor(Color3B(255, 220, 100));
    _leftPanel->addChild(_govLabel);

    // 创建专门放槽位的容器
    _leftSlotsContainer = Node::create();
    _leftSlotsContainer->setPosition(0, 0);
    _leftPanel->addChild(_leftSlotsContainer);

    // 更换政体按钮
    auto btnChange = Button::create();
    btnChange->setTitleText(u8"【更换政体】");
    btnChange->setTitleFontSize(20);
    btnChange->setTitleColor(Color3B(100, 255, 100));
    btnChange->setPosition(Vec2(midX / 2, vs.height - 90));
    btnChange->addClickEventListener([this](Ref*) {
        this->onChangeGovClicked();
        });
    _leftPanel->addChild(btnChange);

    // --- 右侧面板 ---
    _rightPanel = Node::create();
    _rightPanel->setPosition(midX, 0);
    this->addChild(_rightPanel);

    // 右侧标题
    auto listLabel = Label::createWithSystemFont(u8"可选政策库 (拖拽装备/点击详情)", "Arial", 24);
    listLabel->setPosition((vs.width - midX) / 2, vs.height - 50);
    _rightPanel->addChild(listLabel);
}

void PolicyPanel::setPolicyManager(PolicyManager* mgr) {
    _policyManager = mgr;
    refreshUI();
}

void PolicyPanel::setCultureTree(CultureTree* tree) {
    _cultureTree = tree;
}

void PolicyPanel::refreshUI() {
    if (!_policyManager) return;

    // 更新政体名称
    const auto& govConfig = _policyManager->getCurrentGovConfig();
    if (_govLabel) {
        _govLabel->setString(govConfig.name);
    }

    createLeftSlots();
    createRightList();
}

// =========================================================
// 核心：通用卡牌外观绘制
// =========================================================
Node* PolicyPanel::createCardVisual(const PolicyCard& card, bool isGhost) {
    auto node = Node::create();
    node->setContentSize(Size(CARD_W, CARD_H));
    node->setAnchorPoint(Vec2::ANCHOR_MIDDLE);

    // 1. 背景底板
    auto bg = DrawNode::create();
    Color4F colorF;
    Color3B c3b = getTypeColor(card.type);
    colorF = Color4F(c3b.r / 255.f, c3b.g / 255.f, c3b.b / 255.f, isGhost ? 0.6f : 1.0f);

    // 绘制实心矩形
    bg->drawSolidRect(Vec2::ZERO, Vec2(CARD_W, CARD_H), colorF);
    // 绘制边框
    bg->drawRect(Vec2::ZERO, Vec2(CARD_W, CARD_H), Color4F(1, 1, 1, 0.5f));
    node->addChild(bg);

    // 2. 顶部类型条 (装饰)
    auto topBar = DrawNode::create();
    topBar->drawSolidRect(Vec2(5, CARD_H - 35), Vec2(CARD_W - 5, CARD_H - 5), Color4F(0, 0, 0, 0.2f));
    node->addChild(topBar);

    // 3. 类型图标/文字
    std::string typeIcon = getTypeName(card.type);
    auto typeLabel = Label::createWithSystemFont(typeIcon, "Arial", 16);
    typeLabel->setPosition(CARD_W / 2, CARD_H - 20);
    typeLabel->setColor(Color3B(255, 255, 255));
    node->addChild(typeLabel);

    // 4. 卡牌名称 (居中，自动换行)
    auto nameLabel = Label::createWithSystemFont(card.name, "Arial", 22);
    nameLabel->setPosition(CARD_W / 2, CARD_H * 0.5f); // 稍微居中一点
    nameLabel->setDimensions(CARD_W - 20, 0); // 限制宽度支持换行
    nameLabel->setHorizontalAlignment(TextHAlignment::CENTER);
    nameLabel->setColor(Color3B::WHITE);
    // 加阴影
    nameLabel->enableShadow(Color4B::BLACK, Size(2, -2), 1);
    node->addChild(nameLabel);

    // 【已删除】：底部提示文字 "点按详情\n拖拽装备"

    return node;
}

// =========================================================
// 右侧列表：创建可拖拽卡牌
// =========================================================
Node* PolicyPanel::createDraggableCard(const PolicyCard& card) {
    // 1. 创建外观
    Node* cardNode = createCardVisual(card, false);
    cardNode->setUserData((void*)(intptr_t)card.id);

    // 2. 添加复杂的触摸监听 (区分点击和拖拽)
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false); // 允许 ScrollView 滚动

    struct TouchState {
        Vec2 startPos;
        bool isDragging;
        bool validStart;
    };
    auto state = std::make_shared<TouchState>();

    listener->onTouchBegan = [this, cardNode, card, state](Touch* touch, Event* event) -> bool {
        Vec2 p = cardNode->getParent()->convertToNodeSpace(touch->getLocation());
        Rect bbox = cardNode->getBoundingBox();

        if (bbox.containsPoint(p)) {
            state->startPos = touch->getLocation(); // 记录世界坐标
            state->isDragging = false;
            state->validStart = true;
            return true; // 吞噬这次触摸，开始追踪
        }
        return false;
        };

    listener->onTouchMoved = [this, card, state](Touch* touch, Event* event) {
        if (!state->validStart) return;

        Vec2 currentPos = touch->getLocation();
        float dist = currentPos.distance(state->startPos);

        // 如果移动超过 10 像素，判定为拖拽
        if (dist > 10.0f && !state->isDragging) {
            state->isDragging = true;
            this->onCardDragBegan(card.id, card.type, currentPos);
        }

        if (state->isDragging) {
            this->onCardDragMoved(currentPos);
        }
        };

    listener->onTouchEnded = [this, card, state](Touch* touch, Event* event) {
        if (!state->validStart) return;

        if (state->isDragging) {
            // 拖拽结束
            this->onCardDragEnded(touch->getLocation());
        }
        else {
            // 点击
            this->onCardClicked(card.id, card);
        }

        state->validStart = false;
        state->isDragging = false;
        };

    // 必须设置 true 才能拦截后续 Move/End，但为了 ScrollView，通常需要处理
    // 这里设为 true 并自己在 onTouchMoved 判断是否传递可能比较复杂
    // 简单的做法是设为 true，自己处理。
    listener->setSwallowTouches(true);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, cardNode);

    return cardNode;
}

// =========================================================
// 左侧：槽位与布局
// =========================================================
void PolicyPanel::createLeftSlots() {
    if (_leftSlotsContainer) _leftSlotsContainer->removeAllChildren();
    _slotTargets.clear();

    const auto& govConfig = _policyManager->getCurrentGovConfig();
    auto equipped = _policyManager->getEquippedPolicies();

    auto vs = Director::getInstance()->getVisibleSize();

    float startY = vs.height - 120; // 初始高度
    float categoryGap = 30.0f;     // 类别之间的垂直间距
    float cardGapX = 15.0f;        // 槽位之间的水平间距

    // 辅助函数：构建一行槽位
    auto buildRow = [&](PolicyType type, int count, const std::string& title) {
        if (count <= 0) return;

        // 1. 标题
        auto label = Label::createWithSystemFont(title, "Arial", 20);
        label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        label->setPosition(30, startY);
        label->setColor(getTypeColor(type));
        _leftSlotsContainer->addChild(label);

        startY -= 30; // 标题高度占位

        // 2. 计算起始 X
        float currentX = 50.0f + CARD_W / 2;

        // 3. 生成槽位
        for (int i = 0; i < count; ++i) {
            int equippedId = -1;
            for (const auto& eq : equipped) {
                if (eq.slotType == type && eq.slotIndex == i) {
                    equippedId = eq.cardId;
                    break;
                }
            }

            Node* slotNode = createSlotUI(type, i, equippedId);
            slotNode->setPosition(currentX, startY - CARD_H / 2);
            _leftSlotsContainer->addChild(slotNode);

            // 记录碰撞信息
            Vec2 worldPos = _leftSlotsContainer->convertToWorldSpace(slotNode->getPosition());
            // 碰撞盒稍微收缩一点，使得必须拖到中心才算
            Rect bbox(worldPos.x - CARD_W / 2, worldPos.y - CARD_H / 2, CARD_W, CARD_H);

            SlotTarget target;
            target.type = type;
            target.index = i;
            target.worldBoundingBox = bbox;
            target.isOccupied = (equippedId != -1);
            _slotTargets.push_back(target);

            // 移动 X 坐标
            currentX += (CARD_W + cardGapX);
        }

        // 移动 Y 坐标到下一行
        startY -= (CARD_H + categoryGap);
        };

    buildRow(PolicyType::MILITARY, govConfig.militarySlots, u8"军事政策");
    buildRow(PolicyType::ECONOMIC, govConfig.economicSlots, u8"经济政策");
    buildRow(PolicyType::WILDCARD, govConfig.wildcardSlots, u8"通用政策");
}

Node* PolicyPanel::createSlotUI(PolicyType type, int index, int equippedCardId) {
    auto container = Node::create();

    // 1. 空槽位背景
    auto bg = DrawNode::create();
    bg->drawSolidRect(Vec2(-CARD_W / 2, -CARD_H / 2), Vec2(CARD_W / 2, CARD_H / 2), Color4F(0.15f, 0.15f, 0.18f, 0.8f));
    bg->drawRect(Vec2(-CARD_W / 2, -CARD_H / 2), Vec2(CARD_W / 2, CARD_H / 2), Color4F(0.5f, 0.5f, 0.5f, 0.3f));
    container->addChild(bg);

    auto label = Label::createWithSystemFont(u8"空槽位", "Arial", 18);
    label->setColor(Color3B::GRAY);
    container->addChild(label);

    // 2. 如果已装备，覆盖卡牌
    if (equippedCardId != -1) {
        label->setVisible(false);

        auto cardPtr = _policyManager->getPolicyCard(equippedCardId);
        if (cardPtr) {
            Node* cardNode = createCardVisual(*cardPtr, false);
            cardNode->setPosition(0, 0);
            container->addChild(cardNode);

            // --- 卸载拖拽 ---
            auto listener = EventListenerTouchOneByOne::create();
            listener->setSwallowTouches(true);

            struct DragState { bool dragging = false; Vec2 startP; };
            auto st = std::make_shared<DragState>();

            listener->onTouchBegan = [this, cardNode, equippedCardId, type, st](Touch* t, Event*) {
                Vec2 p = cardNode->getParent()->convertToNodeSpace(t->getLocation());
                Rect bbox = Rect(-CARD_W / 2, -CARD_H / 2, CARD_W, CARD_H);
                if (bbox.containsPoint(p)) {
                    st->startP = t->getLocation();
                    st->dragging = false;
                    return true;
                }
                return false;
                };

            listener->onTouchMoved = [this, equippedCardId, type, st](Touch* t, Event*) {
                if (t->getLocation().distance(st->startP) > 10 && !st->dragging) {
                    st->dragging = true;
                    this->onCardDragBegan(equippedCardId, type, t->getLocation());
                }
                if (st->dragging) {
                    this->onCardDragMoved(t->getLocation());
                }
                };

            listener->onTouchEnded = [this, cardPtr, st](Touch* t, Event*) {
                if (st->dragging) {
                    this->onEquippedCardDragEnded(t);
                }
                else {
                    this->onCardClicked(cardPtr->id, *cardPtr);
                }
                };

            _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, cardNode);
        }
    }

    return container;
}

// =========================================================
// 拖拽逻辑实现
// =========================================================

void PolicyPanel::onCardDragBegan(int cardId, PolicyType type, Vec2 touchPos) {
    if (_isDragActive) return;

    _isDragActive = true;
    _draggedCardId = cardId;
    _draggedCardType = type;

    auto cardPtr = _policyManager->getPolicyCard(cardId);
    if (!cardPtr) return;

    // 创建替身
    _draggedNode = createCardVisual(*cardPtr, true);
    _draggedNode->setOpacity(200);
    _draggedNode->setScale(1.1f);
    _draggedNode->setPosition(this->convertToNodeSpace(touchPos));
    this->addChild(_draggedNode, 999);
}

void PolicyPanel::onCardDragMoved(Vec2 touchPos) {
    if (_draggedNode) {
        _draggedNode->setPosition(this->convertToNodeSpace(touchPos));
    }
}

void PolicyPanel::onCardDragEnded(Vec2 touchPos) {
    if (!_draggedNode) {
        _isDragActive = false;
        return;
    }

    bool success = false;

    // 检查落在哪个槽位
    for (const auto& slot : _slotTargets) {
        if (slot.worldBoundingBox.containsPoint(touchPos)) {
            bool isCompatible = false;
            if (slot.type == PolicyType::WILDCARD) isCompatible = true;
            else if (slot.type == _draggedCardType) isCompatible = true;

            if (isCompatible) {
                if (_policyManager->equipPolicy(_draggedCardId, slot.type, slot.index)) {
                    success = true;
                    CCLOG("Equipped card %d to slot %d", _draggedCardId, slot.index);
                }
            }
            break;
        }
    }

    _draggedNode->removeFromParent();
    _draggedNode = nullptr;
    _draggedCardId = -1;
    _isDragActive = false;

    if (success) {
        refreshUI();
    }
}

void PolicyPanel::onEquippedCardDragEnded(Touch* touch) {
    if (!_draggedNode) {
        _isDragActive = false;
        return;
    }

    Vec2 pos = touch->getLocation();
    auto vs = Director::getInstance()->getVisibleSize();
    float leftPanelWidth = vs.width * 0.4f;

    bool hitSlot = false;
    for (const auto& slot : _slotTargets) {
        if (slot.worldBoundingBox.containsPoint(pos)) {
            hitSlot = true;
            // 支持重新装备（即交换或放回原位）
            if (_policyManager->equipPolicy(_draggedCardId, slot.type, slot.index)) {
                refreshUI();
            }
            break;
        }
    }

    // 拖到右侧或空白处卸载
    if (!hitSlot) {
        if (pos.x > leftPanelWidth) {
            _policyManager->unequipPolicy(_draggedCardId);
            refreshUI();
        }
    }

    _draggedNode->removeFromParent();
    _draggedNode = nullptr;
    _draggedCardId = -1;
    _isDragActive = false;
}

// =========================================================
// 右侧卡牌列表
// =========================================================

void PolicyPanel::createRightList() {
    _rightPanel->removeChildByName("cards_scroll");

    auto vs = Director::getInstance()->getVisibleSize();
    float midX = vs.width * 0.4f;
    float panelW = vs.width - midX;
    float panelH = vs.height;

    auto scrollView = ScrollView::create();
    scrollView->setContentSize(Size(panelW - 20, panelH - 80));
    scrollView->setAnchorPoint(Vec2(0, 0));
    scrollView->setPosition(Vec2(10, 20));
    scrollView->setDirection(ScrollView::Direction::VERTICAL);
    scrollView->setName("cards_scroll");
    _rightPanel->addChild(scrollView);

    std::vector<PolicyCard> allCards;
    auto militaryCards = _policyManager->getUnlockedCards(PolicyType::MILITARY);
    auto economicCards = _policyManager->getUnlockedCards(PolicyType::ECONOMIC);
    auto wildcardCards = _policyManager->getUnlockedCards(PolicyType::WILDCARD);

    for (const auto& c : militaryCards) if (!c.isActive) allCards.push_back(c);
    for (const auto& c : economicCards) if (!c.isActive) allCards.push_back(c);
    for (const auto& c : wildcardCards) if (!c.isActive) allCards.push_back(c);

    // 网格计算
    int colCount = floor((panelW - 20) / (CARD_W + 10));
    if (colCount < 1) colCount = 1;

    float paddingX = 15;
    float paddingY = 20;

    int totalCount = allCards.size();
    int rowCount = (totalCount + colCount - 1) / colCount;

    float totalH = rowCount * (CARD_H + paddingY) + paddingY;
    float viewH = scrollView->getContentSize().height;

    scrollView->setInnerContainerSize(Size(scrollView->getContentSize().width, std::max(viewH, totalH)));

    float startX = paddingX + CARD_W / 2;
    float startY = std::max(viewH, totalH) - paddingY - CARD_H / 2;

    for (int i = 0; i < totalCount; ++i) {
        int r = i / colCount;
        int c = i % colCount;

        auto cardNode = createDraggableCard(allCards[i]);
        cardNode->setPosition(startX + c * (CARD_W + paddingX), startY - r * (CARD_H + paddingY));
        scrollView->addChild(cardNode);
    }
}

// =========================================================
// 辅助函数与政体界面
// =========================================================

Color3B PolicyPanel::getTypeColor(PolicyType type) {
    switch (type) {
    case PolicyType::MILITARY: return COLOR_MIL;
    case PolicyType::ECONOMIC: return COLOR_ECO;
    case PolicyType::WILDCARD: return COLOR_WILD;
    }
    return Color3B::WHITE;
}

std::string PolicyPanel::getTypeName(PolicyType type) {
    switch (type) {
    case PolicyType::MILITARY: return u8"军事";
    case PolicyType::ECONOMIC: return u8"经济";
    case PolicyType::WILDCARD: return u8"通用";
    }
    return "";
}

// 简介面板相关
void PolicyPanel::createDescriptionPanel() {
    _descriptionPanel = Node::create();

    // 背景底板
    auto background = DrawNode::create();
    // 使用更深的黑色背景 (0,0,0, 0.95) 提高可读性
    background->drawSolidRect(Vec2(0, 0), Vec2(300, 200), Color4F(0.1f, 0.1f, 0.1f, 0.95f));
    // 加一个白色边框
    background->drawRect(Vec2(0, 0), Vec2(300, 200), Color4F::WHITE);
    _descriptionPanel->addChild(background);

    // ... 其余代码不变 (Title, Text, CloseBtn) ...

    _descriptionTitle = Label::createWithSystemFont("", "Arial", 18);
    _descriptionTitle->setAnchorPoint(Vec2(0, 1));
    _descriptionTitle->setPosition(10, 190);
    _descriptionTitle->setColor(Color3B(255, 220, 100)); // 标题改成金色更好看
    _descriptionPanel->addChild(_descriptionTitle);

    _descriptionText = Label::createWithSystemFont("", "Arial", 16); // 字体稍微大一点
    _descriptionText->setAnchorPoint(Vec2(0, 1));
    _descriptionText->setPosition(10, 160);
    _descriptionText->setColor(Color3B(220, 220, 220));
    _descriptionText->setDimensions(280, 150);
    _descriptionText->setHorizontalAlignment(TextHAlignment::LEFT);
    _descriptionText->setVerticalAlignment(TextVAlignment::TOP);
    _descriptionPanel->addChild(_descriptionText);

    auto closeBtn = ui::Button::create();
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(30, 30));
    closeBtn->setPosition(Vec2(285, 185));
    closeBtn->setTitleFontSize(20);
    closeBtn->setTitleText(u8"×");
    closeBtn->setTitleColor(Color3B::WHITE);
    closeBtn->addClickEventListener([this](Ref*) {
        this->hidePolicyDescription();
        });
    _descriptionPanel->addChild(closeBtn);

    _descriptionPanel->setVisible(false);
    this->addChild(_descriptionPanel, 100); // 确保在最上层
}

void PolicyPanel::showPolicyDescription(const PolicyCard& card) {
    if (!_descriptionPanel) {
        createDescriptionPanel();
    }
    _descriptionTitle->setString(card.name);
    std::string descText = u8"类型: " + getTypeName(card.type) + "\n\n" + card.desc;
    _descriptionText->setString(descText);

    // 获取屏幕尺寸
    auto vs = Director::getInstance()->getVisibleSize();

    // 面板固定大小是 300x200 (在 createDescriptionPanel 中定义的)
    float panelW = 300.0f;
    float panelH = 200.0f;
    float margin = 20.0f; // 边距

    // 设置位置为右下角
    // 坐标原点在左下角，所以 x = 屏幕宽 - 面板宽 - 边距, y = 边距
    _descriptionPanel->setPosition(Vec2(vs.width - panelW - margin, margin));

    _descriptionPanel->setVisible(true);
}

void PolicyPanel::hidePolicyDescription() {
    if (_descriptionPanel) {
        _descriptionPanel->setVisible(false);
    }
}

void PolicyPanel::onCardClicked(int cardId, const PolicyCard& card) {
    showPolicyDescription(card);
}

// 政体选择界面相关
void PolicyPanel::onChangeGovClicked() {
    showGovSelectLayer();
}

void PolicyPanel::showGovSelectLayer() {
    auto vs = Director::getInstance()->getVisibleSize();

    auto layer = LayerColor::create(Color4B(0, 0, 0, 220), vs.width, vs.height);
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, layer);
    this->addChild(layer, 999);

    auto title = Label::createWithSystemFont(u8"选择政体", "Arial", 36);
    title->setPosition(vs.width / 2, vs.height - 50);
    title->setColor(Color3B::YELLOW);
    layer->addChild(title);

    auto btnClose = Button::create();
    btnClose->setTitleText(u8"返回");
    btnClose->setTitleFontSize(28);
    btnClose->setPosition(Vec2(vs.width - 80, vs.height - 50));
    btnClose->addClickEventListener([layer](Ref*) {
        layer->removeFromParent();
        });
    layer->addChild(btnClose);

    auto scrollView = ScrollView::create();
    scrollView->setContentSize(Size(vs.width - 100, vs.height - 120));
    scrollView->setAnchorPoint(Vec2(0.5, 0));
    scrollView->setPosition(Vec2(vs.width / 2, 20));
    scrollView->setDirection(ScrollView::Direction::VERTICAL);
    layer->addChild(scrollView);

    auto allTypes = getAllGovTypes();
    float itemH = 120.0f;
    float gap = 20.0f;
    float contentH = (itemH + gap) * allTypes.size();
    float innerH = std::max(scrollView->getContentSize().height, contentH);
    scrollView->setInnerContainerSize(Size(scrollView->getContentSize().width, innerH));

    float currentY = innerH - itemH / 2;

    for (auto type : allTypes) {
        Node* itemNode = createGovOptionUI(type);
        itemNode->setPosition(scrollView->getContentSize().width / 2, currentY);

        bool isUnlocked = false;
        bool isCurrent = false;
        if (_cultureTree) {
            isUnlocked = _cultureTree->isGovernmentUnlocked(type);
            isCurrent = (_cultureTree->getCurrentGovernment() == type);
        }

        auto bg = dynamic_cast<Layout*>(itemNode->getChildByName("bg"));
        if (bg) {
            if (isUnlocked) {
                bg->setTouchEnabled(true);
                bg->addClickEventListener([this, type, layer, isCurrent](Ref*) {
                    if (isCurrent) {
                        layer->removeFromParent();
                        return;
                    }
                    if (_cultureTree->switchGovernment(type)) {
                        _policyManager->updateGovernmentSlots();
                        this->refreshUI();
                        layer->removeFromParent();
                    }
                    });
            }
            else {
                itemNode->setOpacity(100);
                if (auto lbl = dynamic_cast<Label*>(itemNode->getChildByName("status"))) {
                    lbl->setString(u8"未解锁");
                    lbl->setColor(Color3B::RED);
                }
            }
        }
        scrollView->addChild(itemNode);
        currentY -= (itemH + gap);
    }
}

Node* PolicyPanel::createGovOptionUI(GovernmentType type) {
    const auto& config = _policyManager->getGovConfig(type);
    auto container = Node::create();

    auto bg = Layout::create();
    bg->setName("bg");
    bg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    bg->setContentSize(Size(600, 100));
    bg->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    bg->setBackGroundColor(Color3B(60, 60, 70));
    container->addChild(bg);

    auto nameLabel = Label::createWithSystemFont(config.name, "Arial", 28);
    nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    nameLabel->setPosition(-280, 20);
    nameLabel->setColor(Color3B::WHITE);
    container->addChild(nameLabel);

    // 简易显示槽位
    std::string info = u8"军事:" + std::to_string(config.militarySlots) +
        u8" 经济:" + std::to_string(config.economicSlots) +
        u8" 通用:" + std::to_string(config.wildcardSlots);
    auto slotInfo = Label::createWithSystemFont(info, "Arial", 20);
    slotInfo->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    slotInfo->setPosition(-280, -20);
    slotInfo->setColor(Color3B(200, 200, 200));
    container->addChild(slotInfo);

    auto statusLabel = Label::createWithSystemFont("", "Arial", 20);
    statusLabel->setName("status");
    statusLabel->setPosition(250, -30);
    container->addChild(statusLabel);

    return container;
}

std::vector<GovernmentType> PolicyPanel::getAllGovTypes() {
    return {
        GovernmentType::CHIEFDOM,
        GovernmentType::AUTOCRACY,
        GovernmentType::OLIGARCHY,
        GovernmentType::CLASSICAL_REPUBLIC,
        GovernmentType::MONARCHY,
        GovernmentType::DEMOCRACY
    };
}