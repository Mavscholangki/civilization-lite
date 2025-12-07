#include "TechTreePanel.h"
#include <sstream>

bool TechTreePanel::init() {
    if (!Layer::init()) {
        return false;
    }

    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 创建背景
    background = LayerColor::create(Color4B(20, 20, 30, 230));
    background->setContentSize(visibleSize);
    background->setPosition(Vec2::ZERO);
    this->addChild(background, -1);

    // 创建标题
    auto title = Label::createWithTTF("科技树", "fonts/arial.ttf", 36);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 60));
    title->setColor(Color3B(255, 220, 100));
    this->addChild(title);

    // 创建滚动视图
    scrollView = ScrollView::create();
    scrollView->setContentSize(Size(visibleSize.width - 40, visibleSize.height - 250));
    scrollView->setPosition(Vec2(20, 120));
    scrollView->setDirection(ScrollView::Direction::BOTH);
    scrollView->setBounceEnabled(true);
    scrollView->setSwallowTouches(true);
    this->addChild(scrollView);

    // 创建内容节点
    contentNode = Node::create();
    contentNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    scrollView->addChild(contentNode);

    // 设置滚动区域
    scrollView->setInnerContainerSize(Size(2400, 1400));
    contentNode->setPosition(0, 0);

    // 创建底部控制面板
    controlPanel = LayerColor::create(Color4B(40, 40, 50, 200), visibleSize.width - 40, 100);
    controlPanel->setPosition(Vec2(20, 20));
    this->addChild(controlPanel);

    // 当前研究标签
    currentResearchLabel = Label::createWithTTF("当前研究：无", "fonts/arial.ttf", 20);
    currentResearchLabel->setPosition(Vec2(visibleSize.width / 2, 70));
    currentResearchLabel->setColor(Color3B(255, 255, 200));
    controlPanel->addChild(currentResearchLabel);

    // 研究进度条背景
    auto progressBg = LayerColor::create(Color4B(50, 50, 50, 255), 300, 20);
    progressBg->setPosition(Vec2(visibleSize.width / 2 - 150, 40));
    controlPanel->addChild(progressBg);

    // 研究进度条
    auto progressSprite = Sprite::create();
    auto progressDrawNode = DrawNode::create();
    progressDrawNode->drawSolidRect(Vec2::ZERO, Vec2(300, 20), Color4F(0.2f, 0.6f, 0.2f, 1.0f));
    progressSprite->addChild(progressDrawNode);

    researchProgressBar = ProgressTimer::create(progressSprite);
    researchProgressBar->setType(ProgressTimer::Type::BAR);
    researchProgressBar->setMidpoint(Vec2(0, 0));
    researchProgressBar->setBarChangeRate(Vec2(1, 0));
    researchProgressBar->setPercentage(0);
    researchProgressBar->setPosition(Vec2(visibleSize.width / 2, 50));
    controlPanel->addChild(researchProgressBar);

    // 每回合科研标签
    sciencePerTurnLabel = Label::createWithTTF("每回合科研：0", "fonts/arial.ttf", 18);
    sciencePerTurnLabel->setPosition(Vec2(visibleSize.width / 2, 20));
    sciencePerTurnLabel->setColor(Color3B(150, 220, 255));
    controlPanel->addChild(sciencePerTurnLabel);

    // 添加关闭按钮
    auto closeButton = Button::create();
    closeButton->setTitleText("关闭");
    closeButton->setTitleFontSize(24);
    closeButton->setTitleColor(Color3B::WHITE);
    closeButton->setColor(Color3B(200, 100, 100));
    closeButton->setContentSize(Size(100, 40));
    closeButton->setPosition(Vec2(visibleSize.width - 70, visibleSize.height - 40));
    closeButton->addTouchEventListener([this](Ref* sender, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::ENDED) {
            this->removeFromParent();
        }
        });
    this->addChild(closeButton);

    // 初始化详情面板
    detailPanel = nullptr;

    return true;
}

void TechTreePanel::setTechTree(TechTree* tree) {
    if (techTree) {
        techTree->removeEventListener(this);
    }

    techTree = tree;

    if (techTree) {
        techTree->addEventListener(this);
        refreshUI();
        updateControlPanel();
    }
}

void TechTreePanel::setSciencePerTurn(int science) {
    if (sciencePerTurnLabel) {
        std::string text = "每回合科研：" + std::to_string(science);
        sciencePerTurnLabel->setString(text);
    }
}

void TechTreePanel::refreshUI() {
    if (!techTree) return;

    // 清除旧UI
    contentNode->removeAllChildren();
    nodeUIMap.clear();

    // 创建所有科技节点
    auto allTechs = techTree->getActivatedTechList();
    auto researchableTechs = techTree->getResearchableTechList();
    int currentResearch = techTree->getCurrentResearch();

    // 创建节点
    for (int i = 1; i <= 24; i++) {
        auto techInfo = techTree->getTechInfo(i);
        if (techInfo) {
            Node* nodeUI = createTechNodeUI(techInfo);
            if (nodeUI) {
                nodeUIMap[i] = nodeUI;
                contentNode->addChild(nodeUI);

                // 设置位置
                Vec2 pos = calculateNodePosition(i);
                nodeUI->setPosition(pos);
            }
        }
    }

    // 创建连接线
    for (const auto& pair : nodeUIMap) {
        auto techInfo = techTree->getTechInfo(pair.first);
        if (techInfo) {
            for (int dstId : techInfo->dstTechList) {
                createConnectionLine(pair.first, dstId);
            }
        }
    }

    // 更新连接线状态
    updateConnectionLines();

    // 更新节点状态
    for (const auto& pair : nodeUIMap) {
        int techId = pair.first;

        TechNodeState state = TechNodeState::LOCKED;
        if (techTree->isActivated(techId)) {
            state = TechNodeState::ACTIVATED;
        }
        else if (currentResearch == techId) {
            state = TechNodeState::RESEARCHING;
        }
        else if (techTree->getTechProgress(techId) > 0) {
            state = TechNodeState::IN_PROGRESS;
        }
        else if (std::find(researchableTechs.begin(), researchableTechs.end(), techId) != researchableTechs.end()) {
            state = TechNodeState::RESEARCHABLE;
        }

        updateNodeUIState(techId, state);
    }
}

Node* TechTreePanel::createTechNodeUI(const TechNode* techData) {
    if (!techData) return nullptr;

    // 创建节点容器
    auto container = Node::create();
    container->setContentSize(Size(NODE_SIZE, NODE_SIZE));

    // 创建背景按钮
    auto button = Button::create();
    button->setContentSize(Size(NODE_SIZE, NODE_SIZE));
    button->setTag(techData->id);
    button->addTouchEventListener(CC_CALLBACK_2(TechTreePanel::onTechNodeClicked, this));

    // 创建圆形背景
    auto drawNode = DrawNode::create();
    drawNode->setTag(100); // 背景标签
    drawNode->drawSolidCircle(Vec2(NODE_SIZE / 2, NODE_SIZE / 2), NODE_SIZE / 2 - 5,
        CC_DEGREES_TO_RADIANS(360), 30, Color4F(0.3f, 0.3f, 0.4f, 1.0f));
    button->addChild(drawNode, -1);

    // 创建科技图标（用前两个字符表示）
    auto iconLabel = Label::createWithTTF(techData->name.substr(0, 2), "fonts/arial.ttf", 18);
    iconLabel->setPosition(Vec2(NODE_SIZE / 2, NODE_SIZE / 2 + 12));
    iconLabel->setColor(Color3B::WHITE);
    iconLabel->setTag(101); // 图标标签
    button->addChild(iconLabel);

    // 创建科技名称标签
    auto nameLabel = Label::createWithTTF(techData->name, "fonts/arial.ttf", 14);
    nameLabel->setPosition(Vec2(NODE_SIZE / 2, NODE_SIZE / 2 - 20));
    nameLabel->setColor(Color3B::YELLOW);
    nameLabel->setTag(102); // 名称标签
    button->addChild(nameLabel);

    // 创建成本标签
    auto costLabel = Label::createWithTTF(std::to_string(techData->cost), "fonts/arial.ttf", 12);
    costLabel->setPosition(Vec2(NODE_SIZE / 2, 15));
    costLabel->setColor(Color3B(200, 200, 255));
    costLabel->setTag(103); // 成本标签
    button->addChild(costLabel);

    // 创建进度条背景
    auto progressBg = DrawNode::create();
    progressBg->drawSolidRect(Vec2(5, 5), Vec2(NODE_SIZE - 5, 10), Color4F(0.2f, 0.2f, 0.2f, 1.0f));
    progressBg->setTag(104); // 进度条背景标签
    button->addChild(progressBg);

    // 创建进度条
    auto progressBar = DrawNode::create();
    progressBar->setTag(105); // 进度条标签
    button->addChild(progressBar);

    container->addChild(button);
    return container;
}

void TechTreePanel::createConnectionLine(int fromTechId, int toTechId) {
    auto fromNode = nodeUIMap[fromTechId];
    auto toNode = nodeUIMap[toTechId];

    if (!fromNode || !toNode) return;

    // 计算起始和结束位置
    Vec2 startPos = fromNode->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2);
    Vec2 endPos = toNode->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2);

    // 创建线条
    auto drawNode = DrawNode::create();
    drawNode->drawSegment(startPos, endPos, LINE_WIDTH, LINE_COLOR);

    // 将线条添加到内容节点（在节点下面）
    contentNode->addChild(drawNode, -1);

    // 标记线条，用于后续更新
    std::string tag = "line_" + std::to_string(fromTechId) + "_" + std::to_string(toTechId);
    drawNode->setName(tag);
}

void TechTreePanel::updateNodeUIState(int techId, TechNodeState state) {
    auto nodeUI = nodeUIMap[techId];
    if (!nodeUI) return;

    auto button = dynamic_cast<Button*>(nodeUI->getChildren().at(0));
    if (!button) return;

    auto drawNode = dynamic_cast<DrawNode*>(button->getChildByTag(100));
    auto iconLabel = dynamic_cast<Label*>(button->getChildByTag(101));
    auto nameLabel = dynamic_cast<Label*>(button->getChildByTag(102));
    auto costLabel = dynamic_cast<Label*>(button->getChildByTag(103));
    auto progressBar = dynamic_cast<DrawNode*>(button->getChildByTag(105));

    if (!drawNode || !iconLabel || !nameLabel || !costLabel || !progressBar) return;

    Color4F bgColor;
    Color3B iconColor = Color3B::WHITE;
    Color3B nameColor = Color3B::YELLOW;
    Color3B costColor = Color3B(200, 200, 255);

    // 根据状态设置颜色
    switch (state) {
        case TechNodeState::LOCKED:
            bgColor = Color4F(0.3f, 0.3f, 0.4f, 1.0f);
            button->setEnabled(false);
            iconColor = Color3B(150, 150, 150);
            nameColor = Color3B(150, 150, 100);
            costColor = Color3B(150, 150, 200);
            break;
        case TechNodeState::RESEARCHABLE:
            bgColor = Color4F(0.4f, 0.4f, 0.6f, 1.0f);
            button->setEnabled(true);
            break;
        case TechNodeState::RESEARCHING:
            bgColor = Color4F(0.8f, 0.6f, 0.2f, 1.0f); // 金色表示当前研究
            button->setEnabled(true);
            nameColor = Color3B(255, 200, 100);
            break;
        case TechNodeState::IN_PROGRESS:
            bgColor = Color4F(0.5f, 0.5f, 0.3f, 1.0f); // 土黄色表示有进度但未研究
            button->setEnabled(true);
            break;
        case TechNodeState::ACTIVATED:
            bgColor = Color4F(0.3f, 0.6f, 0.3f, 1.0f);
            button->setEnabled(false);
            iconColor = Color3B(200, 255, 200);
            nameColor = Color3B(200, 255, 100);
            costColor = Color3B(180, 220, 255);
            break;
    }

    // 更新背景颜色
    drawNode->clear();
    drawNode->drawSolidCircle(Vec2(NODE_SIZE / 2, NODE_SIZE / 2), NODE_SIZE / 2 - 5,
        CC_DEGREES_TO_RADIANS(360), 30, bgColor);

    // 添加边框
    drawNode->drawCircle(Vec2(NODE_SIZE / 2, NODE_SIZE / 2), NODE_SIZE / 2 - 5,
        CC_DEGREES_TO_RADIANS(360), 30, false, Color4F(1.0f, 1.0f, 1.0f, 0.5f));

    // 更新文本颜色
    iconLabel->setColor(iconColor);
    nameLabel->setColor(nameColor);
    costLabel->setColor(costColor);

    // 更新进度条
    if (techTree) {
        int progress = techTree->getTechProgress(techId);
        int cost = techTree->getTechCost(techId);

        if (cost > 0 && progress > 0) {
            float width = (NODE_SIZE - 10) * ((float)progress / cost);
            progressBar->clear();

            // 根据状态使用不同颜色的进度条
            Color4F progressColor;
            if (state == TechNodeState::RESEARCHING) {
                progressColor = Color4F(0.9f, 0.7f, 0.1f, 1.0f); // 金色进度条
            }
            else if (state == TechNodeState::ACTIVATED) {
                progressColor = Color4F(0.2f, 0.8f, 0.2f, 1.0f); // 绿色进度条
            }
            else {
                progressColor = Color4F(0.2f, 0.6f, 0.2f, 1.0f); // 普通绿色
            }

            progressBar->drawSolidRect(Vec2(5, 5), Vec2(5 + width, 10), progressColor);
        }
        else {
            progressBar->clear();
        }

        // 更新成本显示（显示进度/成本）
        if (progress > 0) {
            std::string progressText = std::to_string(progress) + "/" + std::to_string(cost);
            costLabel->setString(progressText);
        }
    }
}

Vec2 TechTreePanel::calculateNodePosition(int techId) {
    // 根据科技ID和时代计算位置
    int era = getTechEra(techId);
    int indexInEra = 0;

    // 计算同一时代中的索引
    std::vector<int> eraTechs;
    switch (era) {
        case 1: eraTechs = { 1, 2 }; break;
        case 2: eraTechs = { 3, 4 }; break;
        case 3: eraTechs = { 5, 6, 7 }; break;
        case 4: eraTechs = { 8, 9, 10, 11 }; break;
        case 5: eraTechs = { 12, 13 }; break;
        case 6: eraTechs = { 14, 15 }; break;
        case 7: eraTechs = { 16, 17, 18, 19 }; break;
        case 8: eraTechs = { 20, 21 }; break;
        case 9: eraTechs = { 22, 23, 24 }; break;
    }

    // 找到在时代中的位置
    for (size_t i = 0; i < eraTechs.size(); i++) {
        if (eraTechs[i] == techId) {
            indexInEra = static_cast<int>(i);
            break;
        }
    }

    // 计算位置
    float x = 200 + era * 200;
    float y = 400 + (indexInEra - eraTechs.size() / 2.0f + 0.5f) * 180;

    return Vec2(x, y);
}

int TechTreePanel::getTechEra(int techId) const {
    // 根据之前的分层确定时代
    if (techId <= 2) return 1;
    else if (techId <= 4) return 2;
    else if (techId <= 7) return 3;
    else if (techId <= 11) return 4;
    else if (techId <= 13) return 5;
    else if (techId <= 15) return 6;
    else if (techId <= 19) return 7;
    else if (techId <= 21) return 8;
    else return 9;
}

void TechTreePanel::layoutTechTree() {
    // 使用calculateNodePosition自动布局
    // 这个函数现在可以保持为空，或者用于额外的布局调整
}

void TechTreePanel::onTechNodeClicked(Ref* sender, Widget::TouchEventType type) {
    if (type != Widget::TouchEventType::ENDED) return;

    auto button = dynamic_cast<Button*>(sender);
    if (!button || !techTree) return;

    int techId = button->getTag();

    // 显示科技详情
    showTechDetail(techId);

    // 如果可研究且未激活，提供设置为当前研究的选项
    if (techTree->isResearchable(techId) && !techTree->isActivated(techId)) {
        setAsCurrentResearch(techId);
    }
}

void TechTreePanel::showTechDetail(int techId) {
    auto techInfo = techTree->getTechInfo(techId);
    if (!techInfo) return;

    // 隐藏旧的详情面板
    hideTechDetail();

    // 创建新的详情面板
    auto visibleSize = Director::getInstance()->getVisibleSize();

    detailPanel = LayerColor::create(Color4B(0, 0, 0, 220), 350, 250);
    detailPanel->setPosition(Vec2(visibleSize.width - 370, 150));
    detailPanel->setName("detail_panel");

    // 科技名称
    auto nameLabel = Label::createWithTTF(techInfo->name, "fonts/arial.ttf", 28);
    nameLabel->setPosition(Vec2(175, 200));
    nameLabel->setColor(Color3B::YELLOW);
    detailPanel->addChild(nameLabel);

    // 成本信息
    int progress = techTree->getTechProgress(techId);
    int cost = techTree->getTechCost(techId);
    int percent = (cost > 0) ? (progress * 100 / cost) : 0;

    std::string costText = "科技值: " + std::to_string(progress) + "/" + std::to_string(cost);
    costText += " (" + std::to_string(percent) + "%)";

    auto costLabel = Label::createWithTTF(costText, "fonts/arial.ttf", 20);
    costLabel->setPosition(Vec2(175, 160));
    costLabel->setColor(Color3B(150, 220, 255));
    detailPanel->addChild(costLabel);

    // 效果描述
    auto effectLabel = Label::createWithTTF(techInfo->effectDescription, "fonts/arial.ttf", 18);
    effectLabel->setPosition(Vec2(175, 100));
    effectLabel->setDimensions(320, 120);
    effectLabel->setAlignment(TextHAlignment::LEFT);
    effectLabel->setVerticalAlignment(TextVAlignment::TOP);
    effectLabel->setColor(Color3B::WHITE);
    detailPanel->addChild(effectLabel);

    // 研究状态
    std::string statusText;
    if (techTree->isActivated(techId)) {
        statusText = "已研发";
    }
    else if (techTree->getCurrentResearch() == techId) {
        statusText = "研究中...";
    }
    else if (progress > 0) {
        statusText = "有进度";
    }
    else if (techTree->isResearchable(techId)) {
        statusText = "可研究";
    }
    else {
        // 检查缺少的前置科技
        std::string missingPrereqs;
        auto techData = techTree->getTechInfo(techId);
        if (techData) {
            for (int prereqId : techData->srcTechList) {
                if (!techTree->isActivated(prereqId)) {
                    auto prereqInfo = techTree->getTechInfo(prereqId);
                    if (prereqInfo) {
                        if (!missingPrereqs.empty()) missingPrereqs += ", ";
                        missingPrereqs += prereqInfo->name;
                    }
                }
            }
        }
        statusText = "需要: " + missingPrereqs;
    }

    auto statusLabel = Label::createWithTTF(statusText, "fonts/arial.ttf", 18);
    statusLabel->setPosition(Vec2(175, 30));
    statusLabel->setColor(Color3B(255, 200, 100));
    detailPanel->addChild(statusLabel);

    // 如果可研究且未激活，添加研究按钮
    if (techTree->isResearchable(techId) && !techTree->isActivated(techId)) {
        auto researchButton = Button::create();
        researchButton->setTitleText("设为当前研究");
        researchButton->setTitleFontSize(18);
        researchButton->setTitleColor(Color3B::WHITE);
        researchButton->setColor(Color3B(100, 180, 100));
        researchButton->setContentSize(Size(150, 40));
        researchButton->setPosition(Vec2(175, 70));
        researchButton->setTag(techId);
        researchButton->addTouchEventListener([this](Ref* sender, Widget::TouchEventType type) {
            if (type == Widget::TouchEventType::ENDED) {
                auto button = dynamic_cast<Button*>(sender);
                if (button) {
                    setAsCurrentResearch(button->getTag());
                    hideTechDetail();
                }
            }
            });
        detailPanel->addChild(researchButton);
    }

    this->addChild(detailPanel, 10);
}

void TechTreePanel::hideTechDetail() {
    if (detailPanel) {
        detailPanel->removeFromParent();
        detailPanel = nullptr;
    }
}

void TechTreePanel::setAsCurrentResearch(int techId) {
    if (!techTree) return;

    if (techTree->setCurrentResearch(techId)) {
        // 更新UI
        refreshUI();
        updateControlPanel();

        // 显示提示
        auto techInfo = techTree->getTechInfo(techId);
        if (techInfo) {
            auto notification = Label::createWithTTF("已开始研究: " + techInfo->name,
                "fonts/arial.ttf", 20);
            notification->setColor(Color3B(100, 255, 100));
            notification->setPosition(Vec2(getContentSize().width / 2, 180));
            notification->setOpacity(0);

            auto fadeIn = FadeIn::create(0.5f);
            auto delay = DelayTime::create(1.5f);
            auto fadeOut = FadeOut::create(0.5f);
            auto remove = CallFunc::create([notification]() {
                notification->removeFromParent();
                });
            notification->runAction(Sequence::create(fadeIn, delay, fadeOut, remove, nullptr));

            this->addChild(notification, 20);
        }
    }
}

void TechTreePanel::updateControlPanel() {
    if (!techTree) return;

    int currentResearch = techTree->getCurrentResearch();

    if (currentResearch > 0) {
        auto techInfo = techTree->getTechInfo(currentResearch);
        if (techInfo) {
            // 更新当前研究标签
            std::string researchText = "当前研究: " + techInfo->name;
            currentResearchLabel->setString(researchText);

            // 更新进度条
            int progress = techTree->getTechProgress(currentResearch);
            int cost = techTree->getTechCost(currentResearch);
            int percent = (cost > 0) ? (progress * 100 / cost) : 0;
            researchProgressBar->setPercentage(percent);

            // 更新进度条上的文本
            auto progressText = Label::createWithTTF(std::to_string(percent) + "%",
                "fonts/arial.ttf", 14);
            progressText->setPosition(Vec2(researchProgressBar->getContentSize().width / 2,
                researchProgressBar->getContentSize().height / 2));
            progressText->setColor(Color3B::WHITE);

            // 移除旧的文本
            researchProgressBar->removeChildByName("progress_text");
            progressText->setName("progress_text");
            researchProgressBar->addChild(progressText);
        }
    }
    else {
        currentResearchLabel->setString("当前研究：无");
        researchProgressBar->setPercentage(0);
        researchProgressBar->removeChildByName("progress_text");
    }
}

void TechTreePanel::updateConnectionLines() {
    if (!techTree) return;

    // 更新所有连接线的状态
    for (const auto& pair : nodeUIMap) {
        auto techInfo = techTree->getTechInfo(pair.first);
        if (techInfo) {
            for (int dstId : techInfo->dstTechList) {
                std::string tag = "line_" + std::to_string(pair.first) + "_" + std::to_string(dstId);
                auto line = contentNode->getChildByName(tag);
                if (line) {
                    auto drawNode = dynamic_cast<DrawNode*>(line);
                    if (drawNode) {
                        drawNode->clear();

                        auto fromNode = nodeUIMap[pair.first];
                        auto toNode = nodeUIMap[dstId];
                        if (fromNode && toNode) {
                            Vec2 startPos = fromNode->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2);
                            Vec2 endPos = toNode->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2);

                            // 判断线条颜色
                            Color4F lineColor = LINE_COLOR;
                            if (techTree->isActivated(pair.first) && techTree->isActivated(dstId)) {
                                lineColor = LINE_ACTIVE_COLOR; // 已激活
                            }
                            else if (techTree->getCurrentResearch() == pair.first ||
                                techTree->getCurrentResearch() == dstId) {
                                lineColor = LINE_CURRENT_COLOR; // 当前研究相关
                            }

                            drawNode->drawSegment(startPos, endPos, LINE_WIDTH, lineColor);
                        }
                    }
                }
            }
        }
    }
}

// TechEventListener接口实现
void TechTreePanel::onTechActivated(int techId, const std::string& techName,
    const std::string& effect) {
    // 更新节点状态
    updateNodeUIState(techId, TechNodeState::ACTIVATED);

    // 更新连接线
    updateConnectionLines();

    // 更新控制面板
    updateControlPanel();

    // 播放激活特效
    auto node = nodeUIMap[techId];
    if (node) {
        // 简单闪光效果
        auto flash = LayerColor::create(Color4B(100, 255, 100, 100), NODE_SIZE, NODE_SIZE);
        flash->setPosition(node->getPosition());
        flash->setOpacity(0);

        auto fadeIn = FadeTo::create(0.2f, 150);
        auto fadeOut = FadeTo::create(0.3f, 0);
        auto remove = CallFunc::create([flash]() { flash->removeFromParent(); });
        flash->runAction(Sequence::create(fadeIn, fadeOut, remove, nullptr));

        contentNode->addChild(flash, 5);
    }
}

void TechTreePanel::onResearchProgress(int techId, int currentProgress, int totalCost) {
    // 更新节点进度条
    auto nodeUI = nodeUIMap[techId];
    if (nodeUI) {
        auto button = dynamic_cast<Button*>(nodeUI->getChildren().at(0));
        if (button) {
            auto progressBar = dynamic_cast<DrawNode*>(button->getChildByTag(105));
            if (progressBar && totalCost > 0) {
                float width = (NODE_SIZE - 10) * ((float)currentProgress / totalCost);
                progressBar->clear();

                // 判断是否为当前研究
                Color4F progressColor;
                if (techTree->getCurrentResearch() == techId) {
                    progressColor = Color4F(0.9f, 0.7f, 0.1f, 1.0f);
                }
                else {
                    progressColor = Color4F(0.2f, 0.6f, 0.2f, 1.0f);
                }

                progressBar->drawSolidRect(Vec2(5, 5), Vec2(5 + width, 10), progressColor);

                // 更新成本标签显示进度
                auto costLabel = dynamic_cast<Label*>(button->getChildByTag(103));
                if (costLabel) {
                    std::string progressText = std::to_string(currentProgress) + "/" + std::to_string(totalCost);
                    costLabel->setString(progressText);
                }
            }
        }
    }

    // 如果这是当前研究的科技，更新控制面板
    if (techTree->getCurrentResearch() == techId) {
        updateControlPanel();
    }
}

void TechTreePanel::onEurekaTriggered(int techId, const std::string& techName) {
    // 播放尤里卡特效
    createEurekaEffect(techId);

    // 更新节点状态（可能会有进度变化）
    if (techTree) {
        TechNodeState state = TechNodeState::LOCKED;
        if (techTree->isActivated(techId)) {
            state = TechNodeState::ACTIVATED;
        }
        else if (techTree->getCurrentResearch() == techId) {
            state = TechNodeState::RESEARCHING;
        }
        else if (techTree->getTechProgress(techId) > 0) {
            state = TechNodeState::IN_PROGRESS;
        }
        else if (techTree->isResearchable(techId)) {
            state = TechNodeState::RESEARCHABLE;
        }

        updateNodeUIState(techId, state);
    }
}

void TechTreePanel::createEurekaEffect(int techId) {
    auto node = nodeUIMap[techId];
    if (!node) return;

    // 创建闪光特效
    auto flash = Sprite::create();
    auto drawNode = DrawNode::create();
    drawNode->drawSolidCircle(Vec2::ZERO, NODE_SIZE / 2, CC_DEGREES_TO_RADIANS(360), 30,
        Color4F(1.0f, 1.0f, 0.5f, 0.8f));
    flash->addChild(drawNode);
    flash->setPosition(node->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2));
    flash->setScale(0.5f);

    auto scaleUp = ScaleTo::create(0.3f, 1.5f);
    auto fadeOut = FadeOut::create(0.3f);
    auto spawn = Spawn::create(scaleUp, fadeOut, nullptr);
    auto remove = CallFunc::create([flash]() { flash->removeFromParent(); });
    flash->runAction(Sequence::create(spawn, remove, nullptr));

    contentNode->addChild(flash, 10);

    // 显示"尤里卡!"文字
    auto eurekaLabel = Label::createWithTTF("尤里卡!", "fonts/arial.ttf", 28);
    eurekaLabel->setColor(Color3B::YELLOW);
    eurekaLabel->setPosition(node->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2 + 60));
    eurekaLabel->setOpacity(0);

    auto fadeIn = FadeIn::create(0.2f);
    auto moveUp = MoveBy::create(1.0f, Vec2(0, 60));
    auto fadeOut2 = FadeOut::create(0.3f);
    auto remove2 = CallFunc::create([eurekaLabel]() { eurekaLabel->removeFromParent(); });
    eurekaLabel->runAction(Sequence::create(fadeIn, moveUp, fadeOut2, remove2, nullptr));

    contentNode->addChild(eurekaLabel, 10);
}

void TechTreePanel::onExit() {
    hideTechDetail();

    if (techTree) {
        techTree->removeEventListener(this);
    }
    Layer::onExit();
}