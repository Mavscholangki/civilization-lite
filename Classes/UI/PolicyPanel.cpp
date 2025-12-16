#include "PolicyPanel.h"
#include <sstream>
#include <algorithm>

USING_NS_CC;

// PolicyPanelEventListener 实现
void PolicyPanelEventListener::onPolicyUnlocked(int policyId, const std::string& policyName) {
    if (_owner) {
        _owner->handlePolicyUnlocked(policyId, policyName);
    }
}

void PolicyPanelEventListener::onPolicyEquipped(int policyId, PolicyType slotType, int slotIndex) {
    if (_owner) {
        _owner->handlePolicyEquipped(policyId, slotType, slotIndex);
    }
}

void PolicyPanelEventListener::onPolicyUnequipped(int policyId, PolicyType slotType, int slotIndex) {
    if (_owner) {
        _owner->handlePolicyUnequipped(policyId, slotType, slotIndex);
    }
}

void PolicyPanelEventListener::onPolicyComboTriggered(const std::vector<int>& policyIds,
    const std::string& comboName) {
    if (_owner) {
        _owner->handlePolicyComboTriggered(policyIds, comboName);
    }
}

void PolicyPanelEventListener::onCultureUnlocked(int cultureId, const std::string& cultureName,
    const std::string& effect) {
    if (_owner) {
        _owner->handleCultureUnlocked(cultureId, cultureName, effect);
    }
}

void PolicyPanelEventListener::onCultureProgress(int cultureId, int currentProgress, int totalCost) {
    // 政策面板可能不关心文化进度
}

void PolicyPanelEventListener::onInspirationTriggered(int cultureId, const std::string& cultureName) {
    // 政策面板可能不关心灵感触发
}

// PolicyPanel 实现
bool PolicyPanel::init() {
    if (!Layer::init()) {
        return false;
    }

    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建事件监听器
    _eventListener = new PolicyPanelEventListener(this);

    // 创建背景
    _background = LayerColor::create(Color4B(30, 25, 35, 240), visibleSize.width, visibleSize.height);
    this->addChild(_background, -1);

    // 创建内容节点
    _contentNode = Node::create();
    _contentNode->setPosition(Vec2::ZERO);
    this->addChild(_contentNode);

    // 创建标题
    auto title = Label::createWithSystemFont(u8"政策管理系统", "Arial", 36);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 60));
    title->setColor(Color3B(255, 200, 100));
    title->enableOutline(Color4B(0, 0, 0, 128), 3);
    _contentNode->addChild(title);

    // 创建当前政体标签
    _currentGovernmentLabel = Label::createWithSystemFont(u8"当前政体：酋邦", "Arial", 24);
    _currentGovernmentLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 120));
    _currentGovernmentLabel->setColor(Color3B(200, 200, 255));
    _contentNode->addChild(_currentGovernmentLabel);

    // 创建政策槽位标签
    _policySlotsLabel = Label::createWithSystemFont(u8"政策槽位：[军0/经0/外0/通0]", "Arial", 22);
    _policySlotsLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 160));
    _policySlotsLabel->setColor(Color3B(180, 180, 255));
    _contentNode->addChild(_policySlotsLabel);

    // 创建关闭按钮
    auto closeButton = ui::Button::create();
    closeButton->setTitleText(u8"关闭");
    closeButton->setTitleFontSize(28);
    closeButton->setTitleColor(Color3B::WHITE);
    closeButton->setContentSize(Size(120, 50));
    closeButton->setPosition(Vec2(visibleSize.width - 80, visibleSize.height - 60));
    closeButton->addClickEventListener([this](Ref* sender) {
        auto event = EventCustom("policy_panel_closed");
        this->getEventDispatcher()->dispatchEvent(&event);
        });
    _contentNode->addChild(closeButton);

    // 创建政策槽位UI区域
    float slotsY = visibleSize.height - 220;
    createPolicySlotsUI();

    // 创建政策卡类型标签页
    _policyTabs = TabControl::create();
    _policyTabs->setContentSize(Size(visibleSize.width - 40, visibleSize.height - 400));
    _policyTabs->setPosition(Vec2(20, 20));

    // 创建军事政策页
    auto militaryTab = TabHeader::create();
    militaryTab->setTitleText(u8"军事");
    militaryTab->setTitleFontSize(22);

    auto militaryContainer = ScrollView::create();
    militaryContainer->setContentSize(Size(visibleSize.width - 60, visibleSize.height - 450));
    militaryContainer->setDirection(ScrollView::Direction::VERTICAL);
    militaryContainer->setInnerContainerSize(Size(visibleSize.width - 60, 1000));
    militaryContainer->setBounceEnabled(true);

    // 创建经济政策页
    auto economicTab = TabHeader::create();
    economicTab->setTitleText(u8"经济");
    economicTab->setTitleFontSize(22);

    auto economicContainer = ScrollView::create();
    economicContainer->setContentSize(Size(visibleSize.width - 60, visibleSize.height - 450));
    economicContainer->setDirection(ScrollView::Direction::VERTICAL);
    economicContainer->setInnerContainerSize(Size(visibleSize.width - 60, 1000));
    economicContainer->setBounceEnabled(true);

    // 创建外交政策页
    auto diplomaticTab = TabHeader::create();
    diplomaticTab->setTitleText(u8"外交");
    diplomaticTab->setTitleFontSize(22);

    auto diplomaticContainer = ScrollView::create();
    diplomaticContainer->setContentSize(Size(visibleSize.width - 60, visibleSize.height - 450));
    diplomaticContainer->setDirection(ScrollView::Direction::VERTICAL);
    diplomaticContainer->setInnerContainerSize(Size(visibleSize.width - 60, 1000));
    diplomaticContainer->setBounceEnabled(true);

    // 创建通用政策页
    auto wildcardTab = TabHeader::create();
    wildcardTab->setTitleText(u8"通用");
    wildcardTab->setTitleFontSize(22);

    auto wildcardContainer = ScrollView::create();
    wildcardContainer->setContentSize(Size(visibleSize.width - 60, visibleSize.height - 450));
    wildcardContainer->setDirection(ScrollView::Direction::VERTICAL);
    wildcardContainer->setInnerContainerSize(Size(visibleSize.width - 60, 1000));
    wildcardContainer->setBounceEnabled(true);

    // 添加标签页
    _policyTabs->insertTab(0, militaryTab, militaryContainer);
    _policyTabs->insertTab(1, economicTab, economicContainer);
    _policyTabs->insertTab(2, diplomaticTab, diplomaticContainer);
    _policyTabs->insertTab(3, wildcardTab, wildcardContainer);

    // 保存引用
    _policyListViews[PolicyType::MILITARY] = militaryContainer;
    _policyListViews[PolicyType::ECONOMIC] = economicContainer;
    _policyListViews[PolicyType::DIPLOMATIC] = diplomaticContainer;
    _policyListViews[PolicyType::WILDCARD] = wildcardContainer;

    _contentNode->addChild(_policyTabs);

    // 创建详情面板（初始隐藏）
    _detailPanel = LayerColor::create(Color4B(0, 0, 0, 200), 350, 250);
    _detailPanel->setPosition(Vec2(visibleSize.width - 380, visibleSize.height - 340));
    _detailPanel->setVisible(false);
    _contentNode->addChild(_detailPanel, 10);

    // 详情面板内容
    _policyNameLabel = Label::createWithSystemFont("", "Arial", 26);
    _policyNameLabel->setPosition(Vec2(175, 210));
    _policyNameLabel->setColor(Color3B::YELLOW);
    _policyNameLabel->setAlignment(TextHAlignment::CENTER);
    _detailPanel->addChild(_policyNameLabel);

    _policyDescLabel = Label::createWithSystemFont("", "Arial", 18);
    _policyDescLabel->setPosition(Vec2(175, 170));
    _policyDescLabel->setColor(Color3B::WHITE);
    _policyDescLabel->setDimensions(320, 60);
    _policyDescLabel->setAlignment(TextHAlignment::CENTER);
    _detailPanel->addChild(_policyDescLabel);

    _policyEffectLabel = Label::createWithSystemFont("", "Arial", 16);
    _policyEffectLabel->setPosition(Vec2(175, 120));
    _policyEffectLabel->setColor(Color3B(180, 220, 255));
    _policyEffectLabel->setDimensions(320, 80);
    _policyEffectLabel->setAlignment(TextHAlignment::LEFT);
    _detailPanel->addChild(_policyEffectLabel);

    _equipButton = ui::Button::create();
    _equipButton->setTitleText(u8"装备政策");
    _equipButton->setTitleFontSize(20);
    _equipButton->setTitleColor(Color3B::WHITE);
    _equipButton->setContentSize(Size(140, 40));
    _equipButton->setPosition(Vec2(100, 40));
    _equipButton->addClickEventListener([this](Ref* sender) {
        if (_selectedPolicyId != -1 && _policyManager) {
            // 尝试找到可用槽位
            auto policyCard = _policyManager->getPolicyCard(_selectedPolicyId);
            if (policyCard) {
                PolicyType slotType = policyCard->type;
                int slotIndex = findAvailableSlot(slotType);
                if (slotIndex != -1) {
                    equipPolicyToSlot(_selectedPolicyId, slotType, slotIndex);
                }
            }
        }
        });
    _detailPanel->addChild(_equipButton);

    _unequipButton = ui::Button::create();
    _unequipButton->setTitleText(u8"卸下政策");
    _unequipButton->setTitleFontSize(20);
    _unequipButton->setTitleColor(Color3B::WHITE);
    _unequipButton->setContentSize(Size(140, 40));
    _unequipButton->setPosition(Vec2(250, 40));
    _unequipButton->addClickEventListener([this](Ref* sender) {
        if (_selectedPolicyId != -1 && _policyManager) {
            unequipPolicy(_selectedPolicyId);
        }
        });
    _detailPanel->addChild(_unequipButton);

    // 创建组合效果显示区域
    auto comboArea = LayerColor::create(Color4B(20, 15, 25, 180), visibleSize.width - 400, 80);
    comboArea->setPosition(Vec2(20, 20));
    _contentNode->addChild(comboArea);

    auto comboTitle = Label::createWithSystemFont(u8"激活的组合效果：", "Arial", 20);
    comboTitle->setPosition(Vec2(100, 60));
    comboTitle->setColor(Color3B(255, 200, 100));
    comboArea->addChild(comboTitle);

    return true;
}

void PolicyPanel::onExit() {
    hidePolicyDetail();

    if (_policyManager && _eventListener) {
        _policyManager->removeEventListener(_eventListener);
    }

    if (_cultureTree && _eventListener) {
        _cultureTree->removeEventListener(_eventListener);
    }

    if (_eventListener) {
        delete _eventListener;
        _eventListener = nullptr;
    }

    Layer::onExit();
}

void PolicyPanel::setPolicyManager(PolicyManager* policyManager) {
    if (_policyManager && _eventListener) {
        _policyManager->removeEventListener(_eventListener);
    }

    _policyManager = policyManager;

    if (_policyManager && _eventListener) {
        _policyManager->addEventListener(_eventListener);
    }

    refreshUI();
}

void PolicyPanel::setCultureTree(CultureTree* cultureTree) {
    if (_cultureTree && _eventListener) {
        _cultureTree->removeEventListener(_eventListener);
    }

    _cultureTree = cultureTree;

    if (_cultureTree && _eventListener) {
        _cultureTree->addEventListener(_eventListener);
    }

    if (_cultureTree) {
        // 清除旧槽位（如果存在）
        if (!_policySlotsUI.empty()) {
            for (auto& pair : _policySlotsUI) {
                for (auto& slot : pair.second) {
                    if (slot.slotBg) {
                        slot.slotBg->removeFromParent();
                    }
                }
            }
            _policySlotsUI.clear();
        }

        // 创建槽位UI
        createPolicySlotsUI();
    }

    refreshUI();
}

void PolicyPanel::refreshUI() {
    updatePolicySlotsUI();
    updatePolicyCardsUI();

    // 更新顶部信息
    if (_cultureTree) {
        GovernmentType currentGov = _cultureTree->getCurrentGovernment();
        std::string govText = u8"当前政体：" + getGovernmentName(currentGov);
        _currentGovernmentLabel->setString(govText);

        const int* policySlots = _cultureTree->getActivePolicySlots();
        std::string policyText = u8"政策槽位：[军" + std::to_string(policySlots[0]) +
            u8"/经" + std::to_string(policySlots[1]) +
            u8"/外" + std::to_string(policySlots[2]) +
            u8"/通" + std::to_string(policySlots[3]) + "]";
        _policySlotsLabel->setString(policyText);
    }
}

Node* PolicyPanel::createPolicyCardUI(const PolicyCard* policy) {
    if (!policy) {
        CCLOG("ERROR: createPolicyCardUI called with null policy");
        return nullptr;
    }

    CCLOG("Creating card UI for policy %d: %s", policy->id, policy->name.c_str());

    // 创建容器
    auto container = Node::create();
    container->setContentSize(Size(CARD_WIDTH, CARD_HEIGHT));

    // 1. 使用 Sprite 作为背景（而不是 DrawNode）
    auto bgSprite = Sprite::create();
    bgSprite->setTextureRect(Rect(0, 0, CARD_WIDTH, CARD_HEIGHT));

    // 根据政策类型设置颜色
    int typeIndex = static_cast<int>(policy->type);
    if (typeIndex >= 0 && typeIndex < 4) {
        Color3B bgColor = POLICY_COLORS[typeIndex];
        bgSprite->setColor(bgColor);
    }
    else {
        bgSprite->setColor(Color3B(128, 128, 128));
    }

    bgSprite->setOpacity(200);
    bgSprite->setPosition(Vec2(CARD_WIDTH / 2, CARD_HEIGHT / 2));
    container->addChild(bgSprite);

    // 2. 添加边框（使用 Sprite）
    auto borderSprite = Sprite::create();
    borderSprite->setTextureRect(Rect(0, 0, CARD_WIDTH, CARD_HEIGHT));
    borderSprite->setColor(Color3B::WHITE);
    borderSprite->setOpacity(100);
    borderSprite->setPosition(Vec2(CARD_WIDTH / 2, CARD_HEIGHT / 2));
    borderSprite->setScale(1.02f); // 稍微放大一点作为边框
    container->addChild(borderSprite, -1); // 放在背景后面

    // 3. 如果是已装备的卡，添加特殊边框
    if (policy->isActive) {
        auto equippedBorder = Sprite::create();
        equippedBorder->setTextureRect(Rect(0, 0, CARD_WIDTH, CARD_HEIGHT));
        equippedBorder->setColor(Color3B::GREEN);
        equippedBorder->setOpacity(150);
        equippedBorder->setPosition(Vec2(CARD_WIDTH / 2, CARD_HEIGHT / 2));
        equippedBorder->setScale(1.05f);
        container->addChild(equippedBorder, -2);
    }

    // 4. 政策名称
    auto nameLabel = Label::createWithTTF(policy->name, "fonts/arial.ttf", 18);
    nameLabel->setPosition(Vec2(CARD_WIDTH / 2, CARD_HEIGHT - 25));
    nameLabel->setColor(Color3B::WHITE);
    nameLabel->setDimensions(CARD_WIDTH - 20, 40);
    nameLabel->setHorizontalAlignment(TextHAlignment::CENTER);
    nameLabel->setVerticalAlignment(TextVAlignment::CENTER);
    container->addChild(nameLabel);

    // 5. 简短描述
    std::string shortDesc = policy->description;
    if (shortDesc.length() > 30) {
        shortDesc = shortDesc.substr(0, 30) + "...";
    }

    auto descLabel = Label::createWithTTF(shortDesc, "fonts/arial.ttf", 12);
    descLabel->setPosition(Vec2(CARD_WIDTH / 2, CARD_HEIGHT / 2));
    descLabel->setColor(Color3B(220, 220, 220));
    descLabel->setDimensions(CARD_WIDTH - 20, 50);
    descLabel->setHorizontalAlignment(TextHAlignment::CENTER);
    descLabel->setVerticalAlignment(TextVAlignment::CENTER);
    container->addChild(descLabel);

    // 6. 稀有度指示器（使用 Label 而不是 DrawNode）
    std::string rarityText;
    Color3B rarityColor;
    switch (policy->rarity) {
        case PolicyRarity::COMMON:
            rarityText = u8"普通";
            rarityColor = Color3B::WHITE;
            break;
        case PolicyRarity::RARE:
            rarityText = u8"稀有";
            rarityColor = Color3B(200, 100, 255);
            break;
        case PolicyRarity::EPIC:
            rarityText = u8"史诗";
            rarityColor = Color3B::YELLOW;
            break;
        case PolicyRarity::LEGENDARY:
            rarityText = u8"传奇";
            rarityColor = Color3B(255, 150, 50);
            break;
    }

    auto rarityLabel = Label::createWithTTF(rarityText, "fonts/arial.ttf", 10);
    rarityLabel->setPosition(Vec2(CARD_WIDTH - 25, 20));
    rarityLabel->setColor(rarityColor);
    container->addChild(rarityLabel);

    // 7. 状态指示器
    std::string statusText;
    Color3B statusColor;
    if (policy->isActive) {
        statusText = u8"已装备";
        statusColor = Color3B(0, 255, 0);
    }
    else if (!policy->isUnlocked) {
        statusText = u8"未解锁";
        statusColor = Color3B(255, 100, 100);
    }
    else {
        statusText = u8"可用";
        statusColor = Color3B(100, 255, 255);
    }

    auto statusLabel = Label::createWithTTF(statusText, "fonts/arial.ttf", 10);
    statusLabel->setPosition(Vec2(25, 20));
    statusLabel->setColor(statusColor);
    container->addChild(statusLabel);

    // 8. 添加触摸事件（已解锁且未装备的卡）
    if (policy->isUnlocked && !policy->isActive) {
        // 添加一个透明的触摸区域
        auto touchArea = ui::Button::create();
        touchArea->setContentSize(Size(CARD_WIDTH, CARD_HEIGHT));
        touchArea->setPosition(Vec2(CARD_WIDTH / 2, CARD_HEIGHT / 2));
        touchArea->setTitleText("");
        touchArea->setOpacity(0); // 完全透明，只用于触摸
        touchArea->addTouchEventListener([this, policyId = policy->id](Ref* sender,
            ui::Widget::TouchEventType type) {
                if (type == ui::Widget::TouchEventType::ENDED) {
                    CCLOG("Policy card %d clicked", policyId);
                    this->setSelectedPolicy(policyId);
                    this->showPolicyDetail(policyId);
                }
            });
        container->addChild(touchArea, 10);
    }

    // 9. 添加一个简单的调试边框（确保能看到）
    auto debugBorder = Sprite::create();
    debugBorder->setTextureRect(Rect(0, 0, CARD_WIDTH, CARD_HEIGHT));
    debugBorder->setColor(Color3B::WHITE);
    debugBorder->setOpacity(50);
    debugBorder->setPosition(Vec2(CARD_WIDTH / 2, CARD_HEIGHT / 2));
    debugBorder->setScale(1.01f);
    container->addChild(debugBorder, -1);

    return container;
}

void PolicyPanel::createPolicySlotsUI() {
    if (!_cultureTree) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    float startY = visibleSize.height - 220;

    // 四种政策类型
    std::vector<PolicyType> policyTypes = {
        PolicyType::MILITARY,
        PolicyType::ECONOMIC,
        PolicyType::DIPLOMATIC,
        PolicyType::WILDCARD
    };

    std::vector<std::string> typeNames = {
        u8"军事",
        u8"经济",
        u8"外交",
        u8"通用"
    };

    // 每种类型创建标题和槽位
    for (int i = 0; i < 4; i++) {
        PolicyType type = policyTypes[i];

        // 创建类型标题
        auto typeLabel = Label::createWithSystemFont(typeNames[i], "Arial", 24);
        typeLabel->setPosition(Vec2(150 + i * 220, startY - 20));
        typeLabel->setColor(POLICY_COLORS[i]);
        _contentNode->addChild(typeLabel);

        // 获取当前槽位数
        int slotCount = 0;
        if (_cultureTree) {
            const int* slots = _cultureTree->getActivePolicySlots();
            slotCount = slots[i];
        }

        // 创建槽位
        std::vector<PolicySlotUI> slots;
        for (int j = 0; j < 4; j++) { // 最多显示4个槽位
            float x = 100 + i * 220;
            float y = startY - 70 - j * 100;

            // 创建槽位背景
            auto slotBg = ui::Layout::create();
            slotBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
            slotBg->setBackGroundColor(Color3B(60, 60, 80));
            slotBg->setBackGroundColorOpacity(200);
            slotBg->setContentSize(Size(SLOT_WIDTH, SLOT_HEIGHT));
            slotBg->setPosition(Vec2(x, y));
            _contentNode->addChild(slotBg);

            // 槽位标签
            std::string slotText = typeNames[i] + " " + std::to_string(j + 1);
            if (j >= slotCount) {
                slotText += u8"（未解锁）";
            }

            auto slotLabel = Label::createWithSystemFont(slotText, "Arial", 16);
            slotLabel->setPosition(Vec2(SLOT_WIDTH / 2, SLOT_HEIGHT / 2));
            slotLabel->setColor(Color3B(180, 180, 200));
            slotBg->addChild(slotLabel);

            // 保存槽位UI
            PolicySlotUI slotUI;
            slotUI.slotBg = slotBg;
            slotUI.slotLabel = slotLabel;
            slotUI.state = (j < slotCount) ? PolicySlotState::EMPTY : PolicySlotState::INCOMPATIBLE;

            // 如果已解锁，添加点击事件
            if (j < slotCount) {
                slotBg->setTouchEnabled(true);
                slotBg->addTouchEventListener([this, type, index = j](Ref* sender,
                    ui::Widget::TouchEventType eventType) {
                        if (eventType == ui::Widget::TouchEventType::ENDED) {
                            if (_selectedPolicyId != -1 && _policyManager) {
                                // 检查槽位是否可用
                                if (getSlotState(type, index) == PolicySlotState::EMPTY) {
                                    equipPolicyToSlot(_selectedPolicyId, type, index);
                                }
                            }
                        }
                    });
            }

            slots.push_back(slotUI);
        }

        _policySlotsUI[type] = slots;
    }
}

void PolicyPanel::updatePolicySlotsUI() {
    if (!_policyManager || !_cultureTree) return;

    // 获取已装备的政策卡
    auto equippedPolicies = _policyManager->getEquippedPolicies();

    // 清除所有槽位中的政策卡节点
    for (auto& pair : _policySlotsUI) {
        for (auto& slot : pair.second) {
            if (slot.policyCardNode) {
                slot.policyCardNode->removeFromParent();
                slot.policyCardNode = nullptr;
            }
        }
    }

    // 重新填充槽位
    for (const auto& equipped : equippedPolicies) {
        auto it = _policySlotsUI.find(equipped.slotType);
        if (it != _policySlotsUI.end()) {
            if (equipped.slotIndex < it->second.size()) {
                PolicySlotUI& slot = it->second[equipped.slotIndex];
                slot.state = PolicySlotState::OCCUPIED;

                // 更新槽位标签
                auto policyCard = _policyManager->getPolicyCard(equipped.cardId);
                if (policyCard) {
                    slot.slotLabel->setString(policyCard->name);

                    // 创建小型政策卡显示
                    auto miniCard = Node::create();
                    miniCard->setContentSize(Size(SLOT_WIDTH - 10, SLOT_HEIGHT - 10));

                    // 背景
                    auto bg = ui::Layout::create();
                    bg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
                    bg->setBackGroundColor(POLICY_COLORS[static_cast<int>(equipped.slotType)]);
                    bg->setBackGroundColorOpacity(220);
                    bg->setContentSize(Size(SLOT_WIDTH - 10, SLOT_HEIGHT - 10));
                    miniCard->addChild(bg);

                    // 名称
                    auto nameLabel = Label::createWithSystemFont(policyCard->name, "Arial", 14);
                    nameLabel->setPosition(Vec2((SLOT_WIDTH - 10) / 2, (SLOT_HEIGHT - 10) / 2));
                    nameLabel->setColor(Color3B::WHITE);
                    nameLabel->setDimensions(SLOT_WIDTH - 20, 40);
                    nameLabel->setAlignment(TextHAlignment::CENTER);
                    miniCard->addChild(nameLabel);

                    // 添加点击事件（卸下政策卡）
                    bg->setTouchEnabled(true);
                    bg->addTouchEventListener([this, policyId = equipped.cardId](Ref* sender,
                        ui::Widget::TouchEventType type) {
                            if (type == ui::Widget::TouchEventType::ENDED) {
                                this->unequipPolicy(policyId);
                            }
                        });

                    slot.policyCardNode = miniCard;
                    slot.slotBg->addChild(miniCard);
                    miniCard->setPosition(Vec2(SLOT_WIDTH / 2, SLOT_HEIGHT / 2));

                    // 隐藏原始标签
                    slot.slotLabel->setVisible(false);
                }
            }
        }
    }

    // 更新未占用槽位的状态
    for (auto& pair : _policySlotsUI) {
        PolicyType type = pair.first;
        int slotCount = 0;

        if (_cultureTree) {
            const int* slots = _cultureTree->getActivePolicySlots();
            slotCount = slots[static_cast<int>(type)];
        }

        for (int i = 0; i < pair.second.size(); i++) {
            PolicySlotUI& slot = pair.second[i];

            if (i >= slotCount) {
                slot.state = PolicySlotState::INCOMPATIBLE;
                slot.slotBg->setBackGroundColor(Color3B(40, 40, 40));
            }
            else if (slot.state != PolicySlotState::OCCUPIED) {
                slot.state = PolicySlotState::EMPTY;
                slot.slotBg->setBackGroundColor(Color3B(60, 60, 80));
                slot.slotLabel->setVisible(true);

                // 恢复默认标签文本
                std::string typeName = getPolicyTypeName(type);
                std::string slotText = typeName + " " + std::to_string(i + 1);
                slot.slotLabel->setString(slotText);
            }
        }
    }
}

void PolicyPanel::updatePolicyCardsUI() {
    CCLOG("=== updatePolicyCardsUI START ===");

    if (!_policyManager) {
        CCLOG("ERROR: PolicyManager is null!");
        return;
    }

    // 清空所有滚动视图
    for (auto& pair : _policyListViews) {
        if (pair.second) {
            pair.second->removeAllChildren();
        }
    }

    // 政策类型数组
    const PolicyType policyTypes[] = {
        PolicyType::MILITARY,
        PolicyType::ECONOMIC,
        PolicyType::DIPLOMATIC,
        PolicyType::WILDCARD
    };

    // 检查所有政策卡
    auto allPolicies = _policyManager->getAvailablePolicies(PolicyType::WILDCARD);
    CCLOG("Total unlocked policies from manager: %zu", allPolicies.size());

    for (int i = 0; i < allPolicies.size(); i++) {
        CCLOG("  Policy %d: %s (type=%d, active=%s)",
            allPolicies[i].id, allPolicies[i].name.c_str(),
            static_cast<int>(allPolicies[i].type),
            allPolicies[i].isActive ? "YES" : "NO");
    }

    for (PolicyType type : policyTypes) {
        auto scrollView = _policyListViews[type];
        if (!scrollView) {
            CCLOG("ERROR: ScrollView for type %d is null", static_cast<int>(type));
            continue;
        }

        // 获取该类型的政策卡
        auto policies = _policyManager->getAvailablePolicies(type);
        CCLOG("Type %d has %zu available policies", static_cast<int>(type), policies.size());

        if (policies.empty()) {
            // 添加空状态提示
            auto emptyLabel = Label::createWithSystemFont(u8"暂无政策卡", "Arial", 24);
            emptyLabel->setColor(Color3B::GRAY);
            emptyLabel->setPosition(Vec2(scrollView->getContentSize().width / 2,
                scrollView->getContentSize().height / 2));
            scrollView->addChild(emptyLabel);
            continue;
        }

        // 完全重置内层容器
        scrollView->setInnerContainerSize(scrollView->getContentSize());

        // 简单的网格布局
        float padding = 15.0f;
        float startX = padding + CARD_WIDTH / 2;
        float startY = scrollView->getContentSize().height - padding - CARD_HEIGHT / 2;
        float currentX = startX;
        float currentY = startY;

        int cardsPerRow = (int)((scrollView->getContentSize().width - padding * 2) / (CARD_WIDTH + padding));
        if (cardsPerRow < 1) cardsPerRow = 1;

        int cardIndex = 0;

        for (const auto& policy : policies) {
            // 创建政策卡UI
            auto cardNode = createPolicyCardUI(&policy);
            if (!cardNode) {
                CCLOG("  Failed to create card for policy %d", policy.id);
                continue;
            }

            // 计算位置
            int row = cardIndex / cardsPerRow;
            int col = cardIndex % cardsPerRow;

            float x = startX + col * (CARD_WIDTH + padding);
            float y = startY - row * (CARD_HEIGHT + padding);

            // 确保y在容器内
            if (y < CARD_HEIGHT / 2 + padding) {
                y = CARD_HEIGHT / 2 + padding;
            }

            cardNode->setPosition(Vec2(x, y));
            scrollView->addChild(cardNode);

            CCLOG("  Added policy card at (%.1f, %.1f): %s", x, y, policy.name.c_str());

            cardIndex++;
        }

        // 计算所需容器高度
        int totalRows = (int)ceil((float)policies.size() / cardsPerRow);
        float requiredHeight = totalRows * (CARD_HEIGHT + padding) + padding * 2;

        // 如果所需高度大于当前容器高度，调整容器大小
        if (requiredHeight > scrollView->getContentSize().height) {
            scrollView->setInnerContainerSize(Size(
                scrollView->getContentSize().width,
                requiredHeight
            ));

            // 重新定位所有卡片
            cardIndex = 0;
            for (auto& child : scrollView->getChildren()) {
                int row = cardIndex / cardsPerRow;
                int col = cardIndex % cardsPerRow;

                float x = startX + col * (CARD_WIDTH + padding);
                float y = requiredHeight - padding - CARD_HEIGHT / 2 - row * (CARD_HEIGHT + padding);

                child->setPosition(Vec2(x, y));
                cardIndex++;
            }
        }

        // 滚动到顶部
        scrollView->scrollToTop(0.1f, false);
    }

    CCLOG("=== updatePolicyCardsUI END ===");
}

void PolicyPanel::showPolicyDetail(int policyId) {
    if (!_policyManager) return;

    auto policyCard = _policyManager->getPolicyCard(policyId);
    if (!policyCard) return;

    _detailPanel->setVisible(true);

    // 更新详情面板内容
    _policyNameLabel->setString(policyCard->name);
    _policyDescLabel->setString(policyCard->description);

    // 效果描述
    std::string effectText = u8"效果：\n" + policyCard->getEffectDescription();
    _policyEffectLabel->setString(effectText);

    // 更新按钮状态
    bool isUnlocked = policyCard->isUnlocked;
    bool isEquipped = policyCard->isActive;
    bool isCompatible = isPolicyCompatible(policyId);

    _equipButton->setVisible(isUnlocked && !isEquipped && isCompatible);
    _unequipButton->setVisible(isEquipped);

    if (!isCompatible) {
        auto incompatibleLabel = Label::createWithSystemFont(u8"（与当前政体不兼容）", "Arial", 18);
        incompatibleLabel->setPosition(Vec2(175, 40));
        incompatibleLabel->setColor(Color3B(255, 100, 100));
        incompatibleLabel->setTag(999);
        _detailPanel->addChild(incompatibleLabel);
    }
    else {
        // 移除不兼容标签
        auto oldLabel = _detailPanel->getChildByTag(999);
        if (oldLabel) oldLabel->removeFromParent();
    }
}

void PolicyPanel::hidePolicyDetail() {
    _detailPanel->setVisible(false);
    _selectedPolicyId = -1;
}

void PolicyPanel::setSelectedPolicy(int policyId) {
    _selectedPolicyId = policyId;
}

void PolicyPanel::equipPolicyToSlot(int policyId, PolicyType slotType, int slotIndex) {
    if (!_policyManager) return;

    if (_policyManager->equipPolicy(policyId, slotType, slotIndex)) {
        // 装备成功，更新UI
        updatePolicySlotsUI();
        updatePolicyCardsUI();

        // 播放成功特效
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto successLabel = Label::createWithSystemFont(u8"政策装备成功！", "Arial", 28);
        successLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        successLabel->setColor(Color3B(100, 255, 100));
        successLabel->enableOutline(Color4B(0, 0, 0, 128), 3);
        successLabel->setOpacity(0);

        this->addChild(successLabel, 100);

        auto fadeIn = FadeIn::create(0.3f);
        auto delay = DelayTime::create(0.5f);
        auto fadeOut = FadeOut::create(0.3f);
        auto remove = CallFunc::create([successLabel]() { successLabel->removeFromParent(); });
        auto sequence = Sequence::create(fadeIn, delay, fadeOut, remove, nullptr);
        successLabel->runAction(sequence);
    }
}

void PolicyPanel::unequipPolicy(int policyId) {
    if (!_policyManager) return;

    if (_policyManager->unequipPolicy(policyId)) {
        // 卸下成功，更新UI
        updatePolicySlotsUI();
        updatePolicyCardsUI();
    }
}

PolicySlotState PolicyPanel::getSlotState(PolicyType slotType, int slotIndex) const {
    auto it = _policySlotsUI.find(slotType);
    if (it != _policySlotsUI.end()) {
        if (slotIndex < it->second.size()) {
            return it->second[slotIndex].state;
        }
    }
    return PolicySlotState::INCOMPATIBLE;
}

std::string PolicyPanel::getPolicyTypeName(PolicyType type) const {
    switch (type) {
        case PolicyType::MILITARY: return u8"军事";
        case PolicyType::ECONOMIC: return u8"经济";
        case PolicyType::DIPLOMATIC: return u8"外交";
        case PolicyType::WILDCARD: return u8"通用";
        default: return u8"未知";
    }
}

std::string PolicyPanel::getGovernmentName(GovernmentType gov) const {
    switch (gov) {
        case GovernmentType::CHIEFDOM: return u8"酋邦";
        case GovernmentType::AUTOCRACY: return u8"独裁统治";
        case GovernmentType::OLIGARCHY: return u8"寡头政体";
        case GovernmentType::CLASSICAL_REPUBLIC: return u8"古典共和";
        case GovernmentType::MONARCHY: return u8"君主制";
        case GovernmentType::THEOCRACY: return u8"神权政治";
        case GovernmentType::MERCHANT_REPUBLIC: return u8"商人共和国";
        case GovernmentType::DEMOCRACY: return u8"民主制";
        case GovernmentType::COMMUNISM: return u8"共产主义";
        case GovernmentType::FASCISM: return u8"法西斯主义";
        case GovernmentType::CORPORATE_LIBERTY: return u8"公司自由制";
        case GovernmentType::DIGITAL_DEMOCRACY: return u8"数字民主";
        default: return u8"未知政体";
    }
}

bool PolicyPanel::isPolicyCompatible(int policyId) const {
    if (!_policyManager || !_cultureTree) return true;

    // 使用PolicyManager的兼容性检查
    return _policyManager->isPolicyCompatible(policyId);
}

int PolicyPanel::findAvailableSlot(PolicyType type) const {
    if (!_cultureTree) return -1;

    // 获取该类型的槽位数
    const int* slots = _cultureTree->getActivePolicySlots();
    int slotCount = slots[static_cast<int>(type)];

    // 查找第一个空槽位
    auto it = _policySlotsUI.find(type);
    if (it != _policySlotsUI.end()) {
        for (int i = 0; i < slotCount && i < it->second.size(); i++) {
            if (it->second[i].state == PolicySlotState::EMPTY) {
                return i;
            }
        }
    }

    return -1;
}

void PolicyPanel::handlePolicyUnlocked(int policyId, const std::string& policyName) {
    updatePolicyCardsUI();

    // 显示解锁提示
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto unlockLabel = Label::createWithSystemFont(u8"新政策解锁：" + policyName, "Arial", 24);
    unlockLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 100));
    unlockLabel->setColor(Color3B(100, 255, 255));
    unlockLabel->enableOutline(Color4B(0, 0, 0, 128), 2);
    unlockLabel->setOpacity(0);

    this->addChild(unlockLabel, 100);

    auto fadeIn = FadeIn::create(0.3f);
    auto moveUp = MoveBy::create(1.5f, Vec2(0, 50));
    auto fadeOut = FadeOut::create(0.5f);
    auto remove = CallFunc::create([unlockLabel]() { unlockLabel->removeFromParent(); });
    auto sequence = Sequence::create(fadeIn, Spawn::create(moveUp, nullptr), fadeOut, remove, nullptr);
    unlockLabel->runAction(sequence);
}

void PolicyPanel::handlePolicyEquipped(int policyId, PolicyType slotType, int slotIndex) {
    updatePolicySlotsUI();
}

void PolicyPanel::handlePolicyUnequipped(int policyId, PolicyType slotType, int slotIndex) {
    updatePolicySlotsUI();
}

void PolicyPanel::handlePolicyComboTriggered(const std::vector<int>& policyIds,
    const std::string& comboName) {
    // 显示组合效果提示
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto comboLabel = Label::createWithSystemFont(u8"组合效果触发：" + comboName, "Arial", 28);
    comboLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    comboLabel->setColor(Color3B(255, 200, 50));
    comboLabel->enableOutline(Color4B(0, 0, 0, 128), 3);
    comboLabel->setOpacity(0);

    this->addChild(comboLabel, 100);

    auto fadeIn = FadeIn::create(0.3f);
    auto scaleUp = ScaleTo::create(0.3f, 1.2f);
    auto scaleDown = ScaleTo::create(0.3f, 1.0f);
    auto fadeOut = FadeOut::create(0.5f);
    auto remove = CallFunc::create([comboLabel]() { comboLabel->removeFromParent(); });

    auto sequence = Sequence::create(
        fadeIn,
        Spawn::create(scaleUp, nullptr),
        scaleDown,
        DelayTime::create(1.0f),
        fadeOut,
        remove,
        nullptr
    );

    comboLabel->runAction(sequence);
}

void PolicyPanel::handleCultureUnlocked(int cultureId, const std::string& cultureName,
    const std::string& effect) {
    // 文化解锁可能带来新的政策槽位，刷新UI
    refreshUI();

    // 新增：解锁对应的政策卡
    if (_policyManager && _cultureTree) {
        // 获取该文化解锁的政策ID
        std::vector<int> policyIds = _cultureTree->getPoliciesUnlockedByCulture(cultureId);
        for (int policyId : policyIds) {
            if (!_policyManager->isPolicyUnlocked(policyId)) {
                _policyManager->unlockPolicy(policyId);
                CCLOG("Unlocked policy %d from culture %d", policyId, cultureId);
            }
        }
    }
}