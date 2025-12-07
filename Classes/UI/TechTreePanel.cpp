#include "TechTreePanel.h"

bool TechTreePanel::init() {
    if (!Layer::init()) {
        return false;
    }

    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 创建背景
    background = Sprite::create("tech_tree_bg.png");
    if (!background) {
        background = Sprite::create(); // 使用纯色背景
        auto drawNode = DrawNode::create();
        drawNode->drawSolidRect(Vec2::ZERO, visibleSize, Color4F(0.1f, 0.1f, 0.15f, 0.95f));
        background->addChild(drawNode);
    }
    background->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(background, -1);

    // 创建标题
    auto title = Label::createWithSystemFont("科技树", "Arial", 32);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
    title->setColor(Color3B(255, 255, 200));
    this->addChild(title);

    // 创建滚动视图
    scrollView = ScrollView::create();
    scrollView->setContentSize(Size(visibleSize.width - 40, visibleSize.height - 150));
    scrollView->setPosition(Vec2(20, 50));
    scrollView->setDirection(ScrollView::Direction::BOTH);
    scrollView->setBounceEnabled(true);
    scrollView->setSwallowTouches(true);
    this->addChild(scrollView);

    // 创建内容节点（科技树实际大小会更大）
    contentNode = Node::create();
    contentNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    scrollView->addChild(contentNode);

    // 设置滚动区域为科技树大小
    scrollView->setInnerContainerSize(Size(2000, 1200));
    contentNode->setPosition(0, 0);

    // 添加关闭按钮
    auto closeButton = Button::create("close_normal.png", "close_selected.png");
    if (!closeButton) {
        closeButton = Button::create();
        closeButton->setTitleText("X");
        closeButton->setTitleFontSize(24);
    }
    closeButton->setPosition(Vec2(visibleSize.width - 30, visibleSize.height - 30));
    closeButton->addTouchEventListener([this](Ref* sender, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::ENDED) {
            this->removeFromParent();
        }
        });
    this->addChild(closeButton);

    // 初始化选中状态
    selectedTechId = -1;

    return true;
}

void TechTreePanel::setTechTree(TechTree* tree) {
    // 如果已经有引用，先退掉原本的引用
    if (techTree) {
        techTree->removeEventListener(this);
    }

    techTree = tree;

    if (techTree) {
        techTree->addEventListener(this);
        refreshUI();
    }
}

void TechTreePanel::refreshUI() {
    if (!techTree) return;

    // 清除旧UI
    contentNode->removeAllChildren();
    nodeUIMap.clear();

    // 创建所有科技节点
    auto allTechs = techTree->getActivatedTechList();
    auto researchableTechs = techTree->getUnlockableTechList();

    // 获取所有科技信息
    for (int i = 1; i <= 20; i++) { // 假设最大20个科技
        auto techInfo = techTree->getTechInfo(i);
        if (techInfo) {
            Node* nodeUI = createTechNodeUI(techInfo);
            if (nodeUI) {
                nodeUIMap[i] = nodeUI;
                contentNode->addChild(nodeUI);
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

    // 布局科技树
    layoutTechTree();

    // 更新节点状态
    for (const auto& pair : nodeUIMap) {
        int techId = pair.first;

        TechNodeState state = TechNodeState::LOCKED;
        if (techTree->isActivated(techId)) {
            state = TechNodeState::ACTIVATED;
        }
        else if (std::find(researchableTechs.begin(), researchableTechs.end(), techId) != researchableTechs.end()) {
            state = TechNodeState::RESEARCHABLE;
        }
        else {
            // 检查是否正在研究（可以通过进度判断）
            int progress = techTree->getTechProgress(techId);
            if (progress > 0 && progress < 100) {
                state = TechNodeState::RESEARCHING;
            }
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
    drawNode->drawSolidCircle(Vec2(NODE_SIZE / 2, NODE_SIZE / 2), NODE_SIZE / 2 - 5,
        CC_DEGREES_TO_RADIANS(360), 30, Color4F(0.3f, 0.3f, 0.4f, 1.0f));
    button->addChild(drawNode, -1);

    // 创建科技图标（这里用Label代替）
    auto iconLabel = Label::createWithSystemFont(techData->name.substr(0, 2), "Arial", 16);
    iconLabel->setPosition(Vec2(NODE_SIZE / 2, NODE_SIZE / 2 + 10));
    iconLabel->setColor(Color3B::WHITE);
    button->addChild(iconLabel);

    // 创建科技名称标签
    auto nameLabel = Label::createWithSystemFont(techData->name, "Arial", 12);
    nameLabel->setPosition(Vec2(NODE_SIZE / 2, 15));
    nameLabel->setColor(Color3B::YELLOW);
    button->addChild(nameLabel);

    // 创建进度条背景
    auto progressBg = DrawNode::create();
    progressBg->drawSolidRect(Vec2(5, 5), Vec2(NODE_SIZE - 5, 10), Color4F(0.2f, 0.2f, 0.2f, 1.0f));
    button->addChild(progressBg);

    // 创建进度条（初始为0）
    auto progressBar = DrawNode::create();
    progressBar->setTag(100); // 进度条标签
    progressBar->drawSolidRect(Vec2(5, 5), Vec2(5, 10), Color4F(0.2f, 0.6f, 0.2f, 1.0f));
    button->addChild(progressBar);

    container->addChild(button);

    // 设置位置（稍后布局会调整）
    container->setPosition(Vec2(0, 0));

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

    auto drawNode = dynamic_cast<DrawNode*>(button->getChildren().at(0));
    if (!drawNode) return;

    Color4F bgColor;
    Color3B textColor = Color3B::WHITE;

    switch (state) {
        case TechNodeState::LOCKED:
            bgColor = Color4F(0.3f, 0.3f, 0.4f, 1.0f);
            button->setEnabled(false);
            break;
        case TechNodeState::RESEARCHABLE:
            bgColor = Color4F(0.4f, 0.4f, 0.6f, 1.0f);
            button->setEnabled(true);
            break;
        case TechNodeState::RESEARCHING:
            bgColor = Color4F(0.5f, 0.5f, 0.3f, 1.0f);
            button->setEnabled(true);
            break;
        case TechNodeState::ACTIVATED:
            bgColor = Color4F(0.3f, 0.6f, 0.3f, 1.0f);
            button->setEnabled(false);
            textColor = Color3B(200, 255, 200);
            break;
    }

    // 更新背景颜色
    drawNode->clear();
    drawNode->drawSolidCircle(Vec2(NODE_SIZE / 2, NODE_SIZE / 2), NODE_SIZE / 2 - 5,
        CC_DEGREES_TO_RADIANS(360), 30, bgColor);

    // 更新文本颜色
    auto iconLabel = dynamic_cast<Label*>(button->getChildren().at(1));
    auto nameLabel = dynamic_cast<Label*>(button->getChildren().at(2));
    if (iconLabel) iconLabel->setColor(textColor);
    if (nameLabel) nameLabel->setColor(textColor);

    // 更新进度条
    auto progressBar = dynamic_cast<DrawNode*>(button->getChildByTag(100));
    if (progressBar && techTree) {
        int progress = techTree->getTechProgress(techId);
        float width = (NODE_SIZE - 10) * (progress / 100.0f);
        progressBar->clear();
        if (width > 0) {
            progressBar->drawSolidRect(Vec2(5, 5), Vec2(5 + width, 10),
                Color4F(0.2f, 0.6f, 0.2f, 1.0f));
        }
    }
}

void TechTreePanel::layoutTechTree() {
    // 简化的层级布局
    // 第1层：农业
    setNodePosition(1, Vec2(400, 800));

    // 第2层：畜牧、采矿、制陶
    setNodePosition(2, Vec2(300, 600));
    setNodePosition(3, Vec2(400, 600));
    setNodePosition(4, Vec2(500, 600));

    // 第3层：弓箭
    setNodePosition(5, Vec2(250, 400));

    // 第4层：文字、青铜术、骑术
    setNodePosition(6, Vec2(500, 400));
    setNodePosition(7, Vec2(350, 400));
    setNodePosition(8, Vec2(150, 200));

    // 第5层：数学
    setNodePosition(9, Vec2(600, 200));
}

void TechTreePanel::setNodePosition(int techId, const Vec2& pos) {
    auto node = nodeUIMap[techId];
    if (node) {
        node->setPosition(pos);
    }
}

void TechTreePanel::onTechNodeClicked(Ref* sender, Widget::TouchEventType type) {
    if (type != Widget::TouchEventType::ENDED) return;

    auto button = dynamic_cast<Button*>(sender);
    if (!button || !techTree) return;

    int techId = button->getTag();

    // 显示科技详情
    showTechDetail(techId);

    // 如果可研究，则提供研究按钮
    if (techTree->isUnlockable(techId) && !techTree->isActivated(techId)) {
        selectedTechId = techId;

        // 在实际游戏中，这里会弹出确认对话框或直接开始研究
        // 为了演示，我们直接调用研究函数
        sendResearchCommand(techId);
    }
}

void TechTreePanel::showTechDetail(int techId) {
    auto techInfo = techTree->getTechInfo(techId);
    if (!techInfo) return;

    // 移除旧的详情面板
    this->removeChildByName("detail_panel");

    // 创建详情面板
    auto detailPanel = LayerColor::create(Color4B(0, 0, 0, 200), 300, 200);
    detailPanel->setName("detail_panel");
    detailPanel->setPosition(Vec2(50, 50));

    // 科技名称
    auto nameLabel = Label::createWithSystemFont(techInfo->name, "Arial", 24);
    nameLabel->setPosition(Vec2(150, 150));
    nameLabel->setColor(Color3B::YELLOW);
    detailPanel->addChild(nameLabel);

    // 效果描述
    auto effectLabel = Label::createWithSystemFont(techInfo->effectDescription, "Arial", 16);
    effectLabel->setPosition(Vec2(150, 100));
    effectLabel->setDimensions(280, 0);
    effectLabel->setAlignment(TextHAlignment::LEFT);
    effectLabel->setColor(Color3B::WHITE);
    detailPanel->addChild(effectLabel);

    // 进度信息
    int progress = techTree->getTechProgress(techId);
    std::string progressText = "进度: " + std::to_string(progress) + "%";
    auto progressLabel = Label::createWithSystemFont(progressText, "Arial", 18);
    progressLabel->setPosition(Vec2(150, 50));
    progressLabel->setColor(Color3B(150, 255, 150));
    detailPanel->addChild(progressLabel);

    // 关闭按钮
    auto closeBtn = Button::create();
    closeBtn->setTitleText("关闭");
    closeBtn->setTitleFontSize(18);
    closeBtn->setPosition(Vec2(280, 180));
    closeBtn->addTouchEventListener([detailPanel](Ref* sender, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::ENDED) {
            detailPanel->removeFromParent();
        }
        });
    detailPanel->addChild(closeBtn);

    this->addChild(detailPanel, 10);
}

void TechTreePanel::sendResearchCommand(int techId) {
    if (!techTree) return;

    // 在实际游戏中，这里应该由游戏主循环调用updateProgress
    // 目前这里只是占位符
    techTree->updateProgress(techId, 30);

    // 刷新UI
    refreshUI();
}

// TechEventListener接口实现
void TechTreePanel::onTechActivated(int techId, const std::string& techName, const std::string& effect) {
    // 更新节点状态
    updateNodeUIState(techId, TechNodeState::ACTIVATED);

    // 播放激活特效
    auto node = nodeUIMap[techId];
    if (node) {
        auto particle = ParticleSystemQuad::create("particle/activate.plist");
        if (particle) {
            particle->setPosition(node->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2));
            particle->setAutoRemoveOnFinish(true);
            contentNode->addChild(particle, 5);
        }
    }

    // 更新连接线颜色（激活的线变为绿色）
    auto techInfo = techTree->getTechInfo(techId);
    if (techInfo) {
        for (int dstId : techInfo->dstTechList) {
            std::string tag = "line_" + std::to_string(techId) + "_" + std::to_string(dstId);
            auto line = contentNode->getChildByName(tag);
            if (line) {
                auto drawNode = dynamic_cast<DrawNode*>(line);
                if (drawNode) {
                    drawNode->clear();

                    auto fromNode = nodeUIMap[techId];
                    auto toNode = nodeUIMap[dstId];
                    if (fromNode && toNode) {
                        Vec2 startPos = fromNode->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2);
                        Vec2 endPos = toNode->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2);
                        drawNode->drawSegment(startPos, endPos, LINE_WIDTH, LINE_ACTIVE_COLOR);
                    }
                }
            }
        }
    }
}

void TechTreePanel::onResearchProgress(int techId, int currentProgress) {
    // 更新进度条
    auto nodeUI = nodeUIMap[techId];
    if (nodeUI) {
        auto button = dynamic_cast<Button*>(nodeUI->getChildren().at(0));
        if (button) {
            auto progressBar = dynamic_cast<DrawNode*>(button->getChildByTag(100));
            if (progressBar) {
                float width = (NODE_SIZE - 10) * (currentProgress / 100.0f);
                progressBar->clear();
                if (width > 0) {
                    progressBar->drawSolidRect(Vec2(5, 5), Vec2(5 + width, 10),
                        Color4F(0.2f, 0.6f, 0.2f, 1.0f));
                }
            }
        }
    }
}

void TechTreePanel::onEurekaTriggered(int techId, const std::string& techName) {
    // 播放尤里卡特效
    createEurekaEffect(techId);

    // 更新UI状态
    if (techTree->isUnlockable(techId)) {
        updateNodeUIState(techId, TechNodeState::RESEARCHABLE);
    }
    else {
        updateNodeUIState(techId, TechNodeState::LOCKED);
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
    auto eurekaLabel = Label::createWithSystemFont("尤里卡!", "Arial", 24);
    eurekaLabel->setColor(Color3B::YELLOW);
    eurekaLabel->setPosition(node->getPosition() + Vec2(NODE_SIZE / 2, NODE_SIZE / 2 + 50));
    eurekaLabel->setOpacity(0);

    auto fadeIn = FadeIn::create(0.2f);
    auto moveUp = MoveBy::create(0.8f, Vec2(0, 50));
    auto fadeOut2 = FadeOut::create(0.2f);
    auto remove2 = CallFunc::create([eurekaLabel]() { eurekaLabel->removeFromParent(); });
    eurekaLabel->runAction(Sequence::create(fadeIn, moveUp, fadeOut2, remove2, nullptr));

    contentNode->addChild(eurekaLabel, 10);
}

void TechTreePanel::onExit() {
    if (techTree) {
        techTree->removeEventListener(this);
    }
    Layer::onExit();
}