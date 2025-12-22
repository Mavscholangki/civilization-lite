#include "PolicyPanel.h"

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
        // 使用 removeFromParent 彻底关闭
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
    _leftPanel->addChild(_govLabel); // 只保留这就行了

    // 创建专门放槽位的容器
    _leftSlotsContainer = Node::create();
    _leftSlotsContainer->setPosition(0, 0);
    _leftPanel->addChild(_leftSlotsContainer);

    // --- 右侧面板 ---
    _rightPanel = Node::create();
    _rightPanel->setPosition(midX, 0);
    this->addChild(_rightPanel);

    // 右侧标题
    auto listLabel = Label::createWithSystemFont(u8"可选政策库 (拖拽装备)", "Arial", 24);
    listLabel->setPosition((vs.width - midX) / 2, vs.height - 50);
    _rightPanel->addChild(listLabel);

    // ▼▼▼▼▼▼ 删除下面这两行重复的代码 ▼▼▼▼▼▼
    // _govLabel->setColor(Color3B(255, 220, 100)); 
    // _leftPanel->addChild(_govLabel); 
    // ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲

    // 新增：更换政体按钮
    auto btnChange = Button::create();
    btnChange->setTitleText(u8"【更换政体】");
    btnChange->setTitleFontSize(20);
    btnChange->setTitleColor(Color3B(100, 255, 100));
    btnChange->setPosition(Vec2(midX / 2, vs.height - 90));
    btnChange->addClickEventListener([this](Ref*) {
        this->onChangeGovClicked();
        });
    _leftPanel->addChild(btnChange);
}

void PolicyPanel::setPolicyManager(PolicyManager* mgr) {
    _policyManager = mgr;
    if (_policyManager) _policyManager->updateGovernmentSlots();
    refreshUI();
}

void PolicyPanel::setCultureTree(CultureTree* tree) {
    _cultureTree = tree;
}

void PolicyPanel::refreshUI() {
    if (!_policyManager) return;

    // 更新政体名称 (不再崩溃，因为 _govLabel 没有被删除)
    const auto& govConfig = _policyManager->getCurrentGovConfig();
    if (_govLabel) {
        _govLabel->setString(govConfig.name);
    }

    createLeftSlots();
    createRightList();
}

// =========================================================
// 左侧：槽位生成
// =========================================================
void PolicyPanel::createLeftSlots() {
    // 【关键修复】只清理容器内的槽位，保留标题和背景
    if (_leftSlotsContainer) {
        _leftSlotsContainer->removeAllChildren();
    }
    _slotTargets.clear();

    const auto& govConfig = _policyManager->getCurrentGovConfig();
    auto equipped = _policyManager->getEquippedPolicies();

    auto vs = Director::getInstance()->getVisibleSize();
    float midX = vs.width * 0.4f;
    float startY = vs.height - 120;
    float gapY = 80.0f;
    float centerX = midX / 2;

    auto buildSection = [&](PolicyType type, int count, const std::string& typeName) {
        if (count <= 0) return;

        // 类型小标题
        auto label = Label::createWithSystemFont(typeName, "Arial", 20);
        label->setPosition(centerX, startY);
        label->setColor(getTypeColor(type));
        _leftSlotsContainer->addChild(label);
        startY -= 40;

        for (int i = 0; i < count; ++i) {
            int equippedId = -1;
            for (const auto& eq : equipped) {
                if (eq.slotType == type && eq.slotIndex == i) {
                    equippedId = eq.cardId;
                    break;
                }
            }

            auto slotNode = createSlotUI(type, i, equippedId);
            slotNode->setPosition(centerX, startY);
            _leftSlotsContainer->addChild(slotNode);

            // 计算碰撞包围盒 (基于世界坐标)
            // 注意：因为是在刚创建时计算，位置可能需要加上父节点的偏移
            Vec2 worldPos = _leftSlotsContainer->convertToWorldSpace(slotNode->getPosition());
            Size size = Size(220, 60);
            Rect bbox(worldPos.x - size.width / 2, worldPos.y - size.height / 2, size.width, size.height);

            SlotTarget target;
            target.type = type;
            target.index = i;
            target.worldBoundingBox = bbox;
            target.isOccupied = (equippedId != -1);
            _slotTargets.push_back(target);

            startY -= gapY;
        }
        startY -= 20;
        };

    buildSection(PolicyType::MILITARY, govConfig.militarySlots, u8"军事槽位");
    buildSection(PolicyType::ECONOMIC, govConfig.economicSlots, u8"经济槽位");
    buildSection(PolicyType::WILDCARD, govConfig.wildcardSlots, u8"通用槽位");
}

// PolicyPanel.cpp

Node* PolicyPanel::createSlotUI(PolicyType type, int index, int equippedCardId) {
    auto container = Node::create();

    // 背景框 (空槽显示深色)
    auto bg = Layout::create();
    bg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    bg->setContentSize(Size(220, 60));
    bg->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    bg->setBackGroundColor(Color3B(40, 40, 45)); // 默认底色
    container->addChild(bg);

    // 文字 "空闲"
    auto label = Label::createWithSystemFont(u8"空闲", "Arial", 18);
    label->setPosition(0, 0);
    container->addChild(label);

    // 如果已装备，覆盖一张“卡牌”在上面，并支持拖拽
    if (equippedCardId != -1) {
        label->setVisible(false); // 隐藏"空闲"文字

        auto card = _policyManager->getPolicyCard(equippedCardId);
        if (card) {
            // 创建一个和右侧一样的卡牌外观
            auto cardNode = Layout::create();
            cardNode->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
            cardNode->setContentSize(Size(220, 60)); // 和槽位一样大
            cardNode->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
            cardNode->setBackGroundColor(getTypeColor(card->type));
            cardNode->setPosition({ 0, 0 }); // 居中覆盖
            container->addChild(cardNode);

            auto nameLabel = Label::createWithSystemFont(card->name, "Arial", 18);
            nameLabel->setPosition(110, 30); // 相对 Layout 居中
            cardNode->addChild(nameLabel);

            // --- 添加拖拽监听 (实现拖拽卸下) ---
            auto listener = EventListenerTouchOneByOne::create();
            listener->setSwallowTouches(true);

            listener->onTouchBegan = [this, cardNode, equippedCardId, type](Touch* touch, Event* event) {
                Vec2 p = cardNode->getParent()->convertToNodeSpace(touch->getLocation());
                if (cardNode->getBoundingBox().containsPoint(p)) {
                    // 开始拖拽：传入 cardId 和 type
                    this->onCardDragBegan(cardNode, equippedCardId, type, touch);
                    return true;
                }
                return false;
                };

            listener->onTouchMoved = [this](Touch* touch, Event* event) {
                this->onCardDragMoved(touch);
                };

            listener->onTouchEnded = [this](Touch* touch, Event* event) {
                // 这是一个特殊的 End 处理，用于卸下
                this->onEquippedCardDragEnded(touch);
                };

            _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, cardNode);
        }
    }

    return container;
}
// =========================================================
// 右侧：统一卡牌列表 (不分栏)
// =========================================================
void PolicyPanel::createRightList() {
    // 移除右侧除了标题和背景之外的内容
    // 简单起见，我们移除所有名为 "cards_scroll" 的子节点，或者直接重建
    // 这里我们简单重建 ScrollView
    _rightPanel->removeChildByName("cards_scroll");

    auto vs = Director::getInstance()->getVisibleSize();
    float midX = vs.width * 0.4f;
    float panelW = vs.width - midX;
    float panelH = vs.height;

    // 创建一个大的 ScrollView
    auto scrollView = ScrollView::create();
    scrollView->setContentSize(Size(panelW - 20, panelH - 100));
    scrollView->setAnchorPoint(Vec2(0, 0));
    scrollView->setPosition(Vec2(10, 20));
    scrollView->setDirection(ScrollView::Direction::VERTICAL);
    scrollView->setScrollBarEnabled(true);
    scrollView->setName("cards_scroll");
    _rightPanel->addChild(scrollView);

    // 收集所有未装备的解锁卡牌
    std::vector<PolicyCard> allCards;

    // 按顺序添加：军事 -> 经济 -> 通用
    auto militaryCards = _policyManager->getUnlockedCards(PolicyType::MILITARY);
    auto economicCards = _policyManager->getUnlockedCards(PolicyType::ECONOMIC);
    auto wildcardCards = _policyManager->getUnlockedCards(PolicyType::WILDCARD);

    for (const auto& c : militaryCards) if (!c.isActive) allCards.push_back(c);
    for (const auto& c : economicCards) if (!c.isActive) allCards.push_back(c);
    for (const auto& c : wildcardCards) if (!c.isActive) allCards.push_back(c);

    if (allCards.empty()) {
        auto emptyLabel = Label::createWithSystemFont(u8"暂无更多可选政策", "Arial", 22);
        emptyLabel->setPosition(panelW / 2, panelH / 2);
        emptyLabel->setColor(Color3B::GRAY);
        scrollView->addChild(emptyLabel);
        return;
    }

    // 网格布局参数
    int colCount = 3; // 一行3个
    float paddingX = 10;
    float paddingY = 15;
    float startX = paddingX + CARD_W / 2;

    // 计算总高度
    int rowCount = (allCards.size() + colCount - 1) / colCount;
    float totalHeight = rowCount * (CARD_H + paddingY) + paddingY;
    float viewHeight = scrollView->getContentSize().height;

    scrollView->setInnerContainerSize(Size(scrollView->getContentSize().width, std::max(viewHeight, totalHeight)));

    // 开始填充
    float currentY = std::max(viewHeight, totalHeight) - paddingY - CARD_H / 2;

    for (int i = 0; i < allCards.size(); ++i) {
        auto cardNode = createDraggableCard(allCards[i]);

        int col = i % colCount;
        int row = i / colCount;

        float x = startX + col * (CARD_W + paddingX);
        float y = std::max(viewHeight, totalHeight) - paddingY - CARD_H / 2 - row * (CARD_H + paddingY);

        cardNode->setPosition(x, y);
        scrollView->addChild(cardNode);
    }
}

Node* PolicyPanel::createDraggableCard(const PolicyCard& card) {
    auto node = Layout::create();
    node->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    node->setContentSize(Size(CARD_W, CARD_H));
    node->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    node->setBackGroundColor(getTypeColor(card.type)); // 颜色区分类型

    // 卡名
    auto label = Label::createWithSystemFont(card.name, "Arial", 16);
    label->setPosition(CARD_W / 2, CARD_H / 2);
    node->addChild(label);

    // 拖拽监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [this, node, card](Touch* touch, Event* event) {
        Vec2 p = node->getParent()->convertToNodeSpace(touch->getLocation());
        if (node->getBoundingBox().containsPoint(p)) {
            this->onCardDragBegan(node, card.id, card.type, touch);
            return true;
        }
        return false;
        };

    listener->onTouchMoved = [this](Touch* touch, Event* event) {
        this->onCardDragMoved(touch);
        };

    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        this->onCardDragEnded(touch);
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, node);

    return node;
}

// =========================================================
// 拖拽逻辑 (保持不变)
// =========================================================

void PolicyPanel::onCardDragBegan(Node* cardNode, int cardId, PolicyType type, Touch* touch) {
    _draggedCardId = cardId;
    _draggedCardType = type;

    // 创建替身
    _draggedNode = Layout::create();
    static_cast<Layout*>(_draggedNode)->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    static_cast<Layout*>(_draggedNode)->setBackGroundColor(getTypeColor(type));
    static_cast<Layout*>(_draggedNode)->setOpacity(200);
    _draggedNode->setContentSize(Size(CARD_W, CARD_H));
    _draggedNode->setAnchorPoint(Vec2::ANCHOR_MIDDLE);

    // 复制文字
    if (auto layout = dynamic_cast<Layout*>(cardNode)) {
        if (!layout->getChildren().empty()) {
            if (auto lbl = dynamic_cast<Label*>(layout->getChildren().at(0))) {
                auto newLbl = Label::createWithSystemFont(lbl->getString(), "Arial", 16);
                newLbl->setPosition(CARD_W / 2, CARD_H / 2);
                _draggedNode->addChild(newLbl);
            }
        }
    }

    Vec2 nodePos = this->convertToNodeSpace(touch->getLocation());
    _draggedNode->setPosition(nodePos);
    _draggedNode->setLocalZOrder(999);
    this->addChild(_draggedNode);
}

void PolicyPanel::onCardDragMoved(Touch* touch) {
    if (_draggedNode) {
        Vec2 nodePos = this->convertToNodeSpace(touch->getLocation());
        _draggedNode->setPosition(nodePos);
    }
}

void PolicyPanel::onCardDragEnded(Touch* touch) {
    if (!_draggedNode) return;

    bool equipped = false;
    Vec2 touchLocation = touch->getLocation();

    for (const auto& slot : _slotTargets) {
        if (slot.worldBoundingBox.containsPoint(touchLocation)) {
            // 类型检查
            bool isCompatible = false;
            if (slot.type == PolicyType::WILDCARD) isCompatible = true;
            else if (slot.type == _draggedCardType) isCompatible = true;

            if (isCompatible) {
                if (_policyManager->equipPolicy(_draggedCardId, slot.type, slot.index)) {
                    equipped = true;
                }
            }
            else {
                CCLOG("Type mismatch!");
            }
            break;
        }
    }

    _draggedNode->removeFromParent();
    _draggedNode = nullptr;
    _draggedCardId = -1;

    if (equipped) {
        refreshUI();
    }
}

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

// PolicyPanel.cpp

void PolicyPanel::onEquippedCardDragEnded(Touch* touch) {
    if (!_draggedNode) return;

    Vec2 touchLocation = touch->getLocation();

    // 1. 移除拖拽替身
    _draggedNode->removeFromParent();
    _draggedNode = nullptr;

    // 2. 判断是否还在原来的槽位附近？
    // 简单逻辑：如果拖到了右侧区域 (x > 屏幕宽度的40%)，或者是空白区域，则卸下
    auto vs = Director::getInstance()->getVisibleSize();
    float splitLineX = vs.width * 0.4f;

    bool shouldUnequip = false;

    // 条件A: 拖到了右侧列表区
    if (touchLocation.x > splitLineX) {
        shouldUnequip = true;
    }
    // 条件B: 虽然在左侧，但没有落在任何有效槽位上 (稍微复杂点，暂用条件A即可满足大部分需求)
    // 如果你想做的更精细，可以遍历 _slotTargets，如果都没命中，也算卸下。
    else {
        bool hitAnySlot = false;
        for (const auto& slot : _slotTargets) {
            if (slot.worldBoundingBox.containsPoint(touchLocation)) {
                hitAnySlot = true;
                // 这里其实可以做“交换卡牌”的逻辑，目前先不做，只要落在槽位里就不卸下
                break;
            }
        }
        if (!hitAnySlot) {
            shouldUnequip = true; // 拖到左侧空白处也卸下
        }
    }

    if (shouldUnequip) {
        if (_policyManager->unequipPolicy(_draggedCardId)) {
            refreshUI(); // 刷新界面，卡牌回到右侧
            CCLOG("Card %d unequipped by drag.", _draggedCardId);
        }
    }

    _draggedCardId = -1;
}

// PolicyPanel.cpp 底部

void PolicyPanel::onChangeGovClicked() {
    showGovSelectLayer();
}

void PolicyPanel::showGovSelectLayer() {
    auto vs = Director::getInstance()->getVisibleSize();

    // 1. 创建遮罩层 (半透明黑底)
    auto layer = LayerColor::create(Color4B(0, 0, 0, 220), vs.width, vs.height);

    // 吞噬点击，防止点穿到底层
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, layer);

    this->addChild(layer, 999); // 最上层

    // 2. 标题
    auto title = Label::createWithSystemFont(u8"选择政体", "Arial", 36);
    title->setPosition(vs.width / 2, vs.height - 50);
    title->setColor(Color3B::YELLOW);
    layer->addChild(title);

    // 3. 关闭按钮 (点空白处关闭也可以，这里加个显式按钮)
    auto btnClose = Button::create();
    btnClose->setTitleText(u8"返回");
    btnClose->setTitleFontSize(28);
    btnClose->setPosition(Vec2(vs.width - 80, vs.height - 50));
    btnClose->addClickEventListener([layer](Ref*) {
        layer->removeFromParent();
        });
    layer->addChild(btnClose);

    // 4. 创建滚动列表
    auto scrollView = ScrollView::create();
    scrollView->setContentSize(Size(vs.width - 100, vs.height - 120));
    scrollView->setAnchorPoint(Vec2(0.5, 0));
    scrollView->setPosition(Vec2(vs.width / 2, 20));
    scrollView->setDirection(ScrollView::Direction::VERTICAL);
    layer->addChild(scrollView);

    // 5. 填充政体列表
    auto allTypes = getAllGovTypes();
    float itemH = 120.0f; // 每个条目的高度
    float gap = 20.0f;
    float contentH = (itemH + gap) * allTypes.size();
    float innerH = std::max(scrollView->getContentSize().height, contentH);

    scrollView->setInnerContainerSize(Size(scrollView->getContentSize().width, innerH));

    float currentY = innerH - itemH / 2;

    for (auto type : allTypes) {
        // 创建单个条目 UI
        Node* itemNode = createGovOptionUI(type);
        itemNode->setPosition(scrollView->getContentSize().width / 2, currentY);

        // 只有解锁的政体才能点击选择
        // 注意：CultureTree 负责解锁状态判断
        bool isUnlocked = false;
        bool isCurrent = false;
        if (_cultureTree) {
            // 注意：你需要确保 CultureTree 有 isGovernmentUnlocked 接口
            // 之前的代码里实现了 isGovernmentUnlocked(type)
            isUnlocked = _cultureTree->isGovernmentUnlocked(type);
            isCurrent = (_cultureTree->getCurrentGovernment() == type);
        }

        // 添加按钮逻辑
        auto bg = dynamic_cast<Layout*>(itemNode->getChildByName("bg"));
        if (bg) {
            if (isUnlocked) {
                bg->setTouchEnabled(true);
                bg->addClickEventListener([this, type, layer, isCurrent](Ref*) {
                    if (isCurrent) {
                        layer->removeFromParent(); // 已经是当前政体，直接关闭
                        return;
                    }

                    // 执行切换
                    if (_cultureTree->switchGovernment(type)) {
                        // 切换成功，刷新数据和UI
                        _policyManager->updateGovernmentSlots();
                        this->refreshUI();
                        layer->removeFromParent();
                        CCLOG("Switched government!");
                    }
                    });
            }
            else {
                // 未解锁变灰
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

    // 背景
    auto bg = Layout::create();
    bg->setName("bg"); // 方便获取
    bg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    bg->setContentSize(Size(600, 100));
    bg->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    bg->setBackGroundColor(Color3B(60, 60, 70));
    container->addChild(bg);

    // 政体名称
    auto nameLabel = Label::createWithSystemFont(config.name, "Arial", 28);
    nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    nameLabel->setPosition(-280, 20);
    nameLabel->setColor(Color3B::WHITE);
    container->addChild(nameLabel);

    // 槽位信息 (用彩色文字拼接)
    std::string slotText = u8"槽位: ";
    auto slotInfo = Label::createWithSystemFont(slotText, "Arial", 20);
    slotInfo->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    slotInfo->setPosition(-280, -20);
    container->addChild(slotInfo);

    // 动态生成槽位图标（简单用文字代替：[红2] [黄1]）
    float startX = -220;

    auto addSlotIcon = [&](const std::string& txt, Color3B col) {
        auto lbl = Label::createWithSystemFont(txt, "Arial", 20);
        lbl->setColor(col);
        lbl->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        lbl->setPosition(startX, -20);
        container->addChild(lbl);
        startX += 60;
        };

    if (config.militarySlots > 0) addSlotIcon(u8"军" + std::to_string(config.militarySlots), COLOR_MIL);
    if (config.economicSlots > 0) addSlotIcon(u8"经" + std::to_string(config.economicSlots), COLOR_ECO);
    if (config.wildcardSlots > 0) addSlotIcon(u8"通" + std::to_string(config.wildcardSlots), COLOR_WILD);

    // 固有加成描述
    std::string bonusText = u8"加成: 无";
    if (!config.inherentBonuses.empty()) {
        // 简单展示第一个效果
        float val = config.inherentBonuses[0].value;
        switch (config.inherentBonuses[0].type) {
        case EffectType::COMBAT_STRENGTH: bonusText = u8"所有单位战斗力 +" + std::to_string((int)val); break;
        case EffectType::MODIFIER_CULTURE: bonusText = u8"文化值 +" + std::to_string((int)val) + "%"; break;
        case EffectType::MODIFIER_SCIENCE: bonusText = u8"科技值 +" + std::to_string((int)val) + "%"; break;
        case EffectType::MODIFIER_PRODUCTION: bonusText = u8"生产力 +" + std::to_string((int)val) + "%"; break;
        default: bonusText = u8"特殊加成"; break;
        }
    }
    auto bonusLabel = Label::createWithSystemFont(bonusText, "Arial", 18);
    bonusLabel->setColor(Color3B(200, 200, 255));
    bonusLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
    bonusLabel->setPosition(280, 0);
    container->addChild(bonusLabel);

    // 状态标签 (默认空白，在外部控制)
    auto statusLabel = Label::createWithSystemFont("", "Arial", 20);
    statusLabel->setName("status");
    statusLabel->setPosition(250, -30);
    container->addChild(statusLabel);

    return container;
}

std::vector<GovernmentType> PolicyPanel::getAllGovTypes() {
    // 按 Tier 顺序排列
    return {
        GovernmentType::CHIEFDOM,
        GovernmentType::AUTOCRACY,
        GovernmentType::OLIGARCHY,
        GovernmentType::CLASSICAL_REPUBLIC,
        GovernmentType::MONARCHY,
        GovernmentType::DEMOCRACY
        // ... 其他政体 ...
    };
}