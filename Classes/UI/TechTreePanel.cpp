#include "TechTreePanel.h"
#include <sstream>

// TechTreePanelEventListener 实现
void TechTreePanelEventListener::onTechActivated(int techId, const std::string& techName,
    const std::string& effect) {
    if (_owner) {
        _owner->handleTechActivated(techId, techName, effect);
    }
}

void TechTreePanelEventListener::onResearchProgress(int techId, int currentProgress, int totalCost) {
    if (_owner) {
        _owner->handleResearchProgress(techId, currentProgress, totalCost);
    }
}

void TechTreePanelEventListener::onEurekaTriggered(int techId, const std::string& techName) {
    if (_owner) {
        _owner->handleEurekaTriggered(techId, techName);
    }
}

bool TechTreePanel::init() {
    if (!Layer::init()) {
        return false;
    }

    // 设置触摸监听器，确保我们能处理触摸事件
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    listener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
        // 返回false，让触摸事件继续传递
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    _eventListener = new TechTreePanelEventListener(this);
    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建背景
    _background = LayerColor::create(Color4B(15, 15, 25, 240));
    _background->setContentSize(visibleSize);
    this->addChild(_background, -1);

    // 创建标题
    auto title = Label::createWithSystemFont(u8"科技树", "Arial", 40);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
    title->setColor(Color3B(255, 230, 100));
    this->addChild(title);

    // 创建滚动视图
    _scrollView = ScrollView::create();
    _scrollView->setContentSize(Size(visibleSize.width - 40, visibleSize.height - 250));
    _scrollView->setPosition(Vec2(20, 120));
    _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _scrollView->setBounceEnabled(true);
    _scrollView->setSwallowTouches(true);
    this->addChild(_scrollView);

    // 创建内容节点
    _contentNode = Node::create();
    _contentNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    _scrollView->addChild(_contentNode);

    // 设置滚动区域 - 增大宽度以容纳更多列，减少高度以限制上下滚动
    _scrollView->setInnerContainerSize(Size(ERA_SPACING * 10, visibleSize.height - 150)); // 减少内部高度
    _contentNode->setPosition(0, 0);

    // 创建底部控制面板
    _controlPanel = LayerColor::create(Color4B(30, 30, 40, 220), visibleSize.width - 40, 120); // 增加高度
    _controlPanel->setPosition(Vec2(20, 20));
    this->addChild(_controlPanel);

    // 当前研究标签 - 增大字体
    _currentResearchLabel = Label::createWithSystemFont(u8"当前研究：无", "Arial", 26);
    _currentResearchLabel->setPosition(Vec2(visibleSize.width / 2, 85));
    _currentResearchLabel->setColor(Color3B(255, 255, 200));
    _controlPanel->addChild(_currentResearchLabel);

    // 研究进度条背景 - 增大尺寸
    auto progressBg = LayerColor::create(Color4B(40, 40, 40, 255), 500, 30);
    progressBg->setPosition(Vec2(visibleSize.width / 2 - 250, 45));
    _controlPanel->addChild(progressBg);

    // 研究进度条
    auto progressSprite = Sprite::create();
    auto progressDrawNode = DrawNode::create();
    progressDrawNode->drawSolidRect(Vec2::ZERO, Vec2(500, 30), Color4F(0.1f, 0.6f, 0.1f, 1.0f));
    progressSprite->addChild(progressDrawNode);

    _researchProgressBar = ProgressTimer::create(progressSprite);
    _researchProgressBar->setType(ProgressTimer::Type::BAR);
    _researchProgressBar->setMidpoint(Vec2(0, 0));
    _researchProgressBar->setBarChangeRate(Vec2(1, 0));
    _researchProgressBar->setPercentage(0);
    _researchProgressBar->setPosition(Vec2(visibleSize.width / 2, 60));
    _controlPanel->addChild(_researchProgressBar);

    // 每回合科研标签 - 增大字体
    _sciencePerTurnLabel = Label::createWithSystemFont(u8"每回合科研：0", "Arial", 22);
    _sciencePerTurnLabel->setPosition(Vec2(visibleSize.width / 2, 25));
    _sciencePerTurnLabel->setColor(Color3B(150, 220, 255));
    _controlPanel->addChild(_sciencePerTurnLabel);

    // 创建关闭按钮 - 使用Layout作为按钮，增大按钮
    auto closeButton = Layout::create();
    closeButton->setContentSize(Size(120, 50)); // 增大按钮
    closeButton->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    closeButton->setBackGroundColor(Color3B(180, 60, 60));
    closeButton->setBackGroundColorOpacity(200);
    closeButton->setTouchEnabled(true);
    closeButton->setPosition(Vec2(visibleSize.width - 140, visibleSize.height - 50)); // 调整位置

    // 添加关闭按钮文字 - 增大字体
    auto closeLabel = Label::createWithSystemFont(u8"关闭", "Arial", 28);
    closeLabel->setPosition(Vec2(60, 25));
    closeLabel->setColor(Color3B::WHITE);
    closeButton->addChild(closeLabel);


    // 关闭按钮事件
    closeButton->addTouchEventListener([this](Ref* sender, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::ENDED) {
            // 发送关闭事件
            auto event = EventCustom("tech_tree_closed");
            this->getEventDispatcher()->dispatchEvent(&event);
        }
        });
    this->addChild(closeButton);

    return true;
}

void TechTreePanel::setTechTree(TechTree* tree) {
    if (_techTree) {
        // 移除旧的事件监听器
        _techTree->removeEventListener(_eventListener);
    }

    _techTree = tree;

    if (_techTree) {
        // 添加新的事件监听器
        _techTree->addEventListener(_eventListener);
        refreshUI();
        updateControlPanel();
    }
}

void TechTreePanel::setSciencePerTurn(int science) {
    if (_sciencePerTurnLabel) {
        std::string text = "每回合科研：" + std::to_string(science);
        _sciencePerTurnLabel->setString(text);
    }
}

Node* TechTreePanel::createTechNodeUI(const TechNode* techData) {
    if (!techData) return nullptr;

    // 调试输出，检查数据
    CCLOG("Creating UI for tech: %d, name: %s", techData->id, techData->name.c_str());

    // 创建容器节点
    auto container = Node::create();
    container->setContentSize(Size(NODE_WIDTH, NODE_HEIGHT));

    // 创建背景Layout（用于显示颜色）
    auto background = Layout::create();
    background->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    background->setBackGroundColor(Color3B(64, 64, 89));
    background->setBackGroundColorOpacity(255);
    background->setContentSize(Size(NODE_WIDTH, NODE_HEIGHT));
    background->setTouchEnabled(false);
    background->setTag(100);
    container->addChild(background, -1);

    // 使用MenuItem代替Button
    auto menuItem = MenuItem::create();
    menuItem->setContentSize(Size(NODE_WIDTH, NODE_HEIGHT));
    menuItem->setTag(techData->id);

    // 设置回调函数
    menuItem->setCallback([this, techId = techData->id](Ref* sender) {
        CCLOG("Tech node clicked: %d", techId);
        this->showTechDetail(techId);

        if (this->_techTree &&
            this->_techTree->isResearchable(techId) &&
            !this->_techTree->isActivated(techId)) {
            this->setAsCurrentResearch(techId);
        }
        });

    // 创建Menu（每个节点一个）
    auto menu = Menu::create(menuItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    container->addChild(menu);

    // 添加边框（使用DrawNode）
    auto border = DrawNode::create();
    border->drawRect(Vec2(0, 0), Vec2(NODE_WIDTH, NODE_HEIGHT),
        Color4F(1.0f, 1.0f, 1.0f, 0.5f));
    border->setTag(106);
    container->addChild(border);

    // 科技名称（居中显示）
    // 尝试不同的字体和设置
    auto nameLabel = Label::create();

    // 方法1：使用系统字体（确保支持中文）
    nameLabel = Label::createWithSystemFont(techData->name, "Arial", 20);

    // 如果方法1不行，尝试方法2：使用TTF字体
    // nameLabel = Label::createWithTTF(techData->name, "fonts/arial.ttf", 20);

    // 如果方法2不行，尝试方法3：创建一个简单的测试文本
    // nameLabel = Label::createWithSystemFont("测试", "Arial", 20);

    // 设置位置和颜色
    nameLabel->setPosition(Vec2(NODE_WIDTH / 2, NODE_HEIGHT / 2 + 15));
    nameLabel->setColor(Color3B::RED);  // 使用红色，更容易看到
    nameLabel->setTag(101);

    // 暂时移除自动换行设置，先确保能显示
    // nameLabel->setDimensions(NODE_WIDTH - 20, 50);
    // nameLabel->setHorizontalAlignment(TextHAlignment::CENTER);
    // nameLabel->setVerticalAlignment(TextVAlignment::CENTER);
    // nameLabel->setOverflow(Label::Overflow::SHRINK);

    container->addChild(nameLabel, 10);  // 确保标签在较高层级

    // 添加一个调试用的背景，确保标签位置正确
    auto debugBg = LayerColor::create(Color4B(255, 0, 0, 50), NODE_WIDTH - 20, 30);
    debugBg->setPosition(Vec2(10, NODE_HEIGHT / 2));
    debugBg->setTag(999);  // 使用一个独特的tag，便于识别
    container->addChild(debugBg, 5);

    // 科技成本（显示在底部）
    auto costLabel = Label::createWithSystemFont(std::to_string(techData->cost), "Arial", 16);
    costLabel->setPosition(Vec2(NODE_WIDTH / 2, 20));
    costLabel->setColor(Color3B(180, 180, 255));
    costLabel->setTag(103);
    container->addChild(costLabel, 10);

    // 进度条背景
    auto progressBg = DrawNode::create();
    progressBg->drawSolidRect(Vec2(5, NODE_HEIGHT - 12),
        Vec2(NODE_WIDTH - 5, NODE_HEIGHT - 4),
        Color4F(0.2f, 0.2f, 0.2f, 1.0f));
    progressBg->setTag(104);
    container->addChild(progressBg, 5);

    // 进度条
    auto progressBar = DrawNode::create();
    progressBar->setTag(105);
    container->addChild(progressBar, 6);

    CCLOG("Created UI for tech %d: label text = %s, label pos = (%f, %f)",
        techData->id, nameLabel->getString().c_str(),
        NODE_WIDTH / 2, NODE_HEIGHT / 2 + 15);

    return container;
}

void TechTreePanel::drawRoundedRect(DrawNode* drawNode, const Rect& rect, float radius, const Color4F& color) {
    float x = rect.getMinX();
    float y = rect.getMinY();
    float width = rect.size.width;
    float height = rect.size.height;

    // 绘制四个圆角
    drawNode->drawSolidCircle(Vec2(x + radius, y + radius), radius,
        CC_DEGREES_TO_RADIANS(360), 16, color);
    drawNode->drawSolidCircle(Vec2(x + width - radius, y + radius), radius,
        CC_DEGREES_TO_RADIANS(360), 16, color);
    drawNode->drawSolidCircle(Vec2(x + radius, y + height - radius), radius,
        CC_DEGREES_TO_RADIANS(360), 16, color);
    drawNode->drawSolidCircle(Vec2(x + width - radius, y + height - radius), radius,
        CC_DEGREES_TO_RADIANS(360), 16, color);

    // 绘制四个矩形区域
    drawNode->drawSolidRect(Vec2(x + radius, y),
        Vec2(x + width - radius, y + height), color);
    drawNode->drawSolidRect(Vec2(x, y + radius),
        Vec2(x + width, y + height - radius), color);
}

void TechTreePanel::refreshUI() {
    if (!_techTree) return;

    // 清除旧UI
    _contentNode->removeAllChildren();
    _nodeUIMap.clear();
    _techPositions.clear();
    _techDepth.clear();
    _techRow.clear();
    _techByDepth.clear();

    // 第一步：拓扑排序
    std::vector<int> sortedTechs = topologicalSort();

    // 第二步：计算科技深度
    calculateTechDepths();

    // 第三步：布局算法
    layoutTechTree();

    // 第四步：创建节点
    for (int techId : sortedTechs) {
        auto techInfo = _techTree->getTechInfo(techId);
        if (techInfo) {
            Node* nodeUI = createTechNodeUI(techInfo);
            if (nodeUI) {
                _nodeUIMap[techId] = nodeUI;
                _contentNode->addChild(nodeUI);

                // 设置计算好的位置
                Vec2 pos = _techPositions[techId];
                nodeUI->setPosition(pos);
            }
        }
    }

    // 第五步：绘制列背景
    drawColumnBackgrounds();

    // 第六步：创建连接线
    for (const auto& pair : _nodeUIMap) {
        int fromTechId = pair.first;
        auto techInfo = _techTree->getTechInfo(fromTechId);
        if (techInfo) {
            for (int toTechId : techInfo->dstTechList) {
                createSplineConnection(fromTechId, toTechId);
            }
        }
    }

    // 第七步：更新节点状态
    auto researchableTechs = _techTree->getResearchableTechList();
    int currentResearch = _techTree->getCurrentResearch();

    for (const auto& pair : _nodeUIMap) {
        int techId = pair.first;

        TechNodeState state = TechNodeState::LOCKED;
        if (_techTree->isActivated(techId)) {
            state = TechNodeState::ACTIVATED;
        }
        else if (currentResearch == techId) {
            state = TechNodeState::RESEARCHING;
        }
        else if (_techTree->getTechProgress(techId) > 0) {
            state = TechNodeState::IN_PROGRESS;
        }
        else if (std::find(researchableTechs.begin(), researchableTechs.end(), techId) != researchableTechs.end()) {
            state = TechNodeState::RESEARCHABLE;
        }

        updateNodeUIState(techId, state);
    }
}

// === 计算科技深度（列数） ===
void TechTreePanel::calculateTechDepths() {
    if (!_techTree) return;

    // 计算每个科技的深度（最大先决路径长度）
    bool changed = true;
    int maxIterations = 20;

    // 初始化：根节点深度为0，其他为-1
    for (int i = 1; i <= 24; i++) {
        auto techInfo = _techTree->getTechInfo(i);
        if (techInfo && techInfo->srcTechList.empty()) {
            _techDepth[i] = 0;
            _techByDepth[0].push_back(i);
        }
        else if (techInfo) {
            _techDepth[i] = -1; // 未计算
        }
    }

    // 迭代计算深度
    for (int iter = 0; iter < maxIterations && changed; iter++) {
        changed = false;

        for (int i = 1; i <= 24; i++) {
            auto techInfo = _techTree->getTechInfo(i);
            if (!techInfo || _techDepth[i] != -1) continue;

            // 检查所有先决科技是否都有深度
            bool allParentsHaveDepth = true;
            int maxParentDepth = -1;

            for (int parentId : techInfo->srcTechList) {
                if (_techDepth.find(parentId) == _techDepth.end() || _techDepth[parentId] == -1) {
                    allParentsHaveDepth = false;
                    break;
                }
                maxParentDepth = std::max(maxParentDepth, _techDepth[parentId]);
            }

            if (allParentsHaveDepth && maxParentDepth != -1) {
                _techDepth[i] = maxParentDepth + 1;
                _techByDepth[_techDepth[i]].push_back(i);
                changed = true;
            }
        }
    }

    // 确保所有科技都有深度
    for (int i = 1; i <= 24; i++) {
        if (_techDepth.find(i) == _techDepth.end() || _techDepth[i] == -1) {
            // 找不到先决关系的孤立节点，放在第一列
            _techDepth[i] = 0;
            _techByDepth[0].push_back(i);
        }
    }
}

// === 文明6风格的布局算法 ===
void TechTreePanel::layoutTechTree() {
    const float COLUMN_SPACING = 280.0f;   // 与ERA_SPACING保持一致
    const float ROW_SPACING = 150.0f;      // 增大行间距以避免重叠
    const float BASE_X = 100.0f;           // 增加起始X坐标偏移
    const float BASE_Y = 350.0f;

    // 获取内容区域的实际高度（滚动视图的内部高度）
    float contentHeight = _scrollView->getInnerContainerSize().height;

    // 第一步：确定每列的科技并排序
    std::unordered_map<int, std::vector<int>> columnTechs = _techByDepth;

    // 按先决关系对每列的科技进行排序
    for (auto& columnPair : columnTechs) {
        std::sort(columnPair.second.begin(), columnPair.second.end(),
            [this](int a, int b) {
                auto infoA = _techTree->getTechInfo(a);
                auto infoB = _techTree->getTechInfo(b);
                if (!infoA || !infoB) return a < b;

                // 优先按子节点数量排序（子节点多的在上）
                int childrenA = infoA->dstTechList.size();
                int childrenB = infoB->dstTechList.size();
                if (childrenA != childrenB) {
                    return childrenA > childrenB;
                }

                // 其次按父节点数量排序（父节点多的在下）
                int parentsA = infoA->srcTechList.size();
                int parentsB = infoB->srcTechList.size();
                if (parentsA != parentsB) {
                    return parentsA < parentsB;
                }

                return a < b;
            });
    }

    // 第二步：为每个科技分配行号
    for (const auto& columnPair : columnTechs) {
        int column = columnPair.first;
        const auto& techsInColumn = columnPair.second;

        if (techsInColumn.empty()) continue;

        // 使用内容区域高度计算可用的垂直空间
        float availableHeight = contentHeight - 200.0f;  // 留出上下边距
        float totalRequiredHeight = techsInColumn.size() * (NODE_HEIGHT + 60.0f);

        // 动态调整行间距，确保不会超出可用空间
        float dynamicRowSpacing = ROW_SPACING;
        if (totalRequiredHeight > availableHeight) {
            dynamicRowSpacing = availableHeight / techsInColumn.size() - 40.0f;
            dynamicRowSpacing = std::max(dynamicRowSpacing, NODE_HEIGHT + 40.0f);
        }

        // 计算中心行
        float centerRow = (techsInColumn.size() - 1) / 2.0f;

        for (size_t i = 0; i < techsInColumn.size(); i++) {
            int techId = techsInColumn[i];

            // 计算行偏移
            float rowOffset = static_cast<float>(i) - centerRow;

            // 计算基础位置
            float x = BASE_X + column * COLUMN_SPACING;
            float y = BASE_Y + rowOffset * dynamicRowSpacing;

            // 确保Y坐标在合理范围内（基于内容区域）
            float minY = 100.0f;
            float maxY = contentHeight - 100.0f;
            y = std::max(minY, std::min(y, maxY));

            _techPositions[techId] = Vec2(x, y);
        }
    }

    // 第三步：调整位置避免重叠并优化连接线
    adjustPositionsForConnections();
}

// === 调整位置优化连接线 ===
void TechTreePanel::adjustPositionsForConnections() {
    if (!_techTree) return;

    // 获取内容区域的实际高度
    float contentHeight = _scrollView->getInnerContainerSize().height;

    const float MAX_ADJUSTMENT = 50.0f;
    const int MAX_ITERATIONS = 30;
    const float MIN_Y = 120.0f;    // 最小Y坐标，增加边距
    const float MAX_Y = contentHeight - 120.0f;  // 基于内容区域的最大Y坐标

    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        bool adjusted = false;

        // 收集所有连接线
        std::vector<std::pair<int, int>> connections;
        for (const auto& pair : _nodeUIMap) {
            int fromTechId = pair.first;
            auto techInfo = _techTree->getTechInfo(fromTechId);
            if (techInfo) {
                for (int toTechId : techInfo->dstTechList) {
                    connections.push_back({ fromTechId, toTechId });
                }
            }
        }

        // 调整连接线交叉
        for (size_t i = 0; i < connections.size(); i++) {
            for (size_t j = i + 1; j < connections.size(); j++) {
                auto& conn1 = connections[i];
                auto& conn2 = connections[j];

                // 跳过不是同一列的连接线
                if (_techDepth[conn1.first] != _techDepth[conn2.first] ||
                    _techDepth[conn1.second] != _techDepth[conn2.second]) {
                    continue;
                }

                Vec2 start1 = _techPositions[conn1.first];
                Vec2 end1 = _techPositions[conn1.second];
                Vec2 start2 = _techPositions[conn2.first];
                Vec2 end2 = _techPositions[conn2.second];

                // 检查连接线是否交叉（更精确的检查）
                if (doLinesCross(start1, end1, start2, end2)) {
                    // 调整后一个连接的节点位置
                    int adjustTech = conn2.second;
                    Vec2 pos = _techPositions[adjustTech];

                    // 根据连接线的相对位置决定调整方向
                    float y1 = (start1.y + end1.y) / 2.0f;
                    float y2 = (start2.y + end2.y) / 2.0f;

                    if (y2 > y1) {
                        pos.y += MAX_ADJUSTMENT * 0.7f;  // 向下调整
                    }
                    else {
                        pos.y -= MAX_ADJUSTMENT * 0.7f;  // 向上调整
                    }

                    // 确保调整后的位置在合理范围内
                    pos.y = std::max(MIN_Y, std::min(pos.y, MAX_Y));

                    _techPositions[adjustTech] = pos;
                    adjusted = true;
                }
            }
        }

        // 如果没有调整或调整次数达到上限，退出
        if (!adjusted) break;
    }

    // 第四步：确保同一列中的节点不会重叠
    for (const auto& columnPair : _techByDepth) {
        int column = columnPair.first;
        const auto& techsInColumn = columnPair.second;

        if (techsInColumn.size() <= 1) continue;

        // 按Y坐标排序
        std::vector<int> sortedTechs = techsInColumn;
        std::sort(sortedTechs.begin(), sortedTechs.end(),
            [this](int a, int b) {
                return _techPositions[a].y < _techPositions[b].y;
            });

        // 检查并调整重叠的节点
        for (size_t i = 1; i < sortedTechs.size(); i++) {
            int prevId = sortedTechs[i - 1];
            int currId = sortedTechs[i];

            Vec2 prevPos = _techPositions[prevId];
            Vec2 currPos = _techPositions[currId];

            // 如果两个节点太接近，调整位置
            float minSpacing = NODE_HEIGHT + 60.0f; // 增加最小间距
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
                _techPositions[prevId].y = newPrevY;
                _techPositions[currId].y = newCurrY;
            }
        }
    }
}

// === 检查两条线段是否相交 ===
bool TechTreePanel::doLinesCross(const Vec2& p1, const Vec2& p2, const Vec2& q1, const Vec2& q2) {
    // 计算方向向量
    Vec2 r = p2 - p1;
    Vec2 s = q2 - q1;

    // 计算叉积
    float rxs = r.cross(s);
    float qpxr = (q1 - p1).cross(r);

    // 如果两线段平行
    if (abs(rxs) < 0.0001f) {
        return false;
    }

    // 计算交点参数
    float t = (q1 - p1).cross(s) / rxs;
    float u = (q1 - p1).cross(r) / rxs;

    // 检查交点是否在两线段上
    return (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f);
}

// === 创建样条曲线连接 ===
void TechTreePanel::createSplineConnection(int fromTechId, int toTechId) {
    auto fromNode = _nodeUIMap[fromTechId];
    auto toNode = _nodeUIMap[toTechId];

    if (!fromNode || !toNode) return;

    Vec2 startPos = fromNode->getPosition() + Vec2(NODE_WIDTH, NODE_HEIGHT / 2);
    Vec2 endPos = toNode->getPosition() + Vec2(0, NODE_HEIGHT / 2);

    auto drawNode = DrawNode::create();

    // 文明6风格的连线：水平-垂直-水平 折线
    float midX = (startPos.x + endPos.x) / 2;

    // 根据状态决定连线颜色
    Color4F lineColor = Color4F(0.3f, 0.3f, 0.4f, 0.6f); // 默认灰色

    bool fromActivated = _techTree->isActivated(fromTechId);
    bool toActivated = _techTree->isActivated(toTechId);
    int currentResearch = _techTree->getCurrentResearch();

    if (fromActivated && toActivated) {
        lineColor = Color4F(0.0f, 0.8f, 0.0f, 0.8f); // 绿色：都已激活
    }
    else if ((fromActivated && toTechId == currentResearch) ||
        (fromTechId == currentResearch && toActivated)) {
        lineColor = Color4F(0.9f, 0.7f, 0.1f, 0.9f); // 金色：与当前研究相关
    }
    else if (_techTree->getTechProgress(toTechId) > 0) {
        lineColor = Color4F(0.2f, 0.6f, 0.2f, 0.7f); // 浅绿：有进度
    }

    // 绘制三段线：水平-垂直-水平
    drawNode->drawSegment(startPos, Vec2(midX, startPos.y), 2.0f, lineColor);
    drawNode->drawSegment(Vec2(midX, startPos.y), Vec2(midX, endPos.y), 2.0f, lineColor);
    drawNode->drawSegment(Vec2(midX, endPos.y), endPos, 2.0f, lineColor);

    // 在拐角处添加圆点
    drawNode->drawSolidCircle(Vec2(midX, startPos.y), 4.0f,
        CC_DEGREES_TO_RADIANS(360), 8, lineColor);
    drawNode->drawSolidCircle(Vec2(midX, endPos.y), 4.0f,
        CC_DEGREES_TO_RADIANS(360), 8, lineColor);

    //// 添加箭头（指向子节点）
    //Vec2 dir = (endPos - Vec2(midX, endPos.y)).getNormalized();
    //Vec2 arrowTip = endPos - dir * 10.0f; // 稍微回退一点
    //Vec2 wing1 = arrowTip - dir * 8.0f + Vec2(-dir.y, dir.x) * 5.0f;
    //Vec2 wing2 = arrowTip - dir * 8.0f + Vec2(dir.y, -dir.x) * 5.0f;
    //drawNode->drawSolidTriangle(arrowTip, wing1, wing2, lineColor);

    _contentNode->addChild(drawNode, -5);
}

// === 绘制列背景 ===
void TechTreePanel::drawColumnBackgrounds() {
    //// 找到最大列数
    //int maxColumn = 0;
    //for (const auto& pair : _techDepth) {
    //    maxColumn = std::max(maxColumn, pair.second);
    //}

    //// 绘制列背景
    //const float COLUMN_WIDTH = 180.0f;
    //const float PADDING = 20.0f;
    //const float BACKGROUND_HEIGHT = 700.0f; // 增加背景高度
    //const float BACKGROUND_BOTTOM = 50.0f;   // 背景底部位置

    //for (int col = 0; col <= maxColumn; col++) {
    //    float x = 60 + col * 200 - PADDING;
    //    float width = COLUMN_WIDTH + PADDING * 2;

    //    auto columnBg = DrawNode::create();
    //    Color4F bgColor = Color4F(0.05f, 0.05f, 0.08f, 0.3f);

    //    // 绘制圆角矩形列背景
    //    Rect columnRect(x, BACKGROUND_BOTTOM, width, BACKGROUND_HEIGHT);
    //    drawRoundedRect(columnBg, columnRect, 10.0f, bgColor);

    //    _contentNode->addChild(columnBg, -10);

    //    // 添加列标签（时代名称）
    //    std::string columnLabel;
    //    if (col == 0) columnLabel = u8"远古时代";
    //    else if (col == 1) columnLabel = u8"古典时代";
    //    else if (col == 2) columnLabel = u8"中世纪";
    //    else if (col == 3) columnLabel = u8"文艺复兴";
    //    else if (col == 4) columnLabel = u8"工业时代";
    //    else if (col == 5) columnLabel = u8"现代";
    //    else columnLabel = u8"未来时代";

    //    auto label = Label::createWithSystemFont(columnLabel, "Arial", 18);
    //    label->setPosition(Vec2(x + width / 2, BACKGROUND_BOTTOM + BACKGROUND_HEIGHT + 20));
    //    label->setColor(Color3B(220, 220, 255));
    //    _contentNode->addChild(label, -5);
    //}
}

// === 修改拓扑排序函数（优化版） ===
std::vector<int> TechTreePanel::topologicalSort() {
    std::vector<int> result;
    if (!_techTree) return result;

    // 收集所有科技ID
    std::vector<int> allTechs;
    for (int i = 1; i <= 24; i++) {
        if (_techTree->getTechInfo(i)) {
            allTechs.push_back(i);
        }
    }

    // 计算入度
    std::unordered_map<int, int> inDegree;
    std::queue<int> zeroInDegree;

    for (int techId : allTechs) {
        auto techInfo = _techTree->getTechInfo(techId);
        if (techInfo) {
            inDegree[techId] = techInfo->srcTechList.size();
            if (inDegree[techId] == 0) {
                zeroInDegree.push(techId);
            }
        }
    }

    // 拓扑排序（Kahn算法）
    while (!zeroInDegree.empty()) {
        int current = zeroInDegree.front();
        zeroInDegree.pop();
        result.push_back(current);

        auto techInfo = _techTree->getTechInfo(current);
        if (techInfo) {
            for (int childId : techInfo->dstTechList) {
                if (--inDegree[childId] == 0) {
                    zeroInDegree.push(childId);
                }
            }
        }
    }

    return result;
}

void TechTreePanel::updateNodeUIState(int techId, TechNodeState state) {
    auto container = _nodeUIMap[techId];
    if (!container) {
        CCLOG("ERROR: No container found for tech %d", techId);
        return;
    }

    // 移除调试背景
    auto debugBg = container->getChildByTag(999);
    if (debugBg) {
        debugBg->removeFromParent();
    }

    // 获取正确的菜单项
    Menu* menu = nullptr;
    MenuItem* menuItem = nullptr;

    // 查找 Menu
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
            case TechNodeState::LOCKED:
                menuItem->setEnabled(false);
                break;
            case TechNodeState::RESEARCHABLE:
            case TechNodeState::RESEARCHING:
            case TechNodeState::IN_PROGRESS:
                menuItem->setEnabled(true);
                break;
            case TechNodeState::ACTIVATED:
                menuItem->setEnabled(false); // 已激活的科技不可点击
                break;
        }
    }

    // 获取背景（Layout）
    auto background = dynamic_cast<Layout*>(container->getChildByTag(100));
    auto nameLabel = dynamic_cast<Label*>(container->getChildByTag(101));
    auto costLabel = dynamic_cast<Label*>(container->getChildByTag(103));
    auto border = dynamic_cast<DrawNode*>(container->getChildByTag(106));
    auto progressBar = dynamic_cast<DrawNode*>(container->getChildByTag(105));

    if (!background || !nameLabel || !costLabel || !border) {
        CCLOG("ERROR: Failed to get UI elements for tech %d", techId);
        if (!background) CCLOG("  - Background not found");
        if (!nameLabel) CCLOG("  - Name label not found");
        if (!costLabel) CCLOG("  - Cost label not found");
        if (!border) CCLOG("  - Border not found");
        return;
    }

    Color3B bgColor;
    Color3B nameColor = Color3B::YELLOW;
    Color3B costColor = Color3B(180, 180, 255);
    Color4F borderColor = Color4F(1.0f, 1.0f, 1.0f, 0.3f);

    // 根据状态设置颜色
    switch (state) {
        case TechNodeState::LOCKED:
            bgColor = Color3B(64, 64, 89);
            nameColor = Color3B(150, 150, 100);
            costColor = Color3B(120, 120, 180);
            borderColor = Color4F(0.5f, 0.5f, 0.5f, 0.3f);
            break;
        case TechNodeState::RESEARCHABLE:
            bgColor = Color3B(89, 89, 127);
            borderColor = Color4F(0.8f, 0.8f, 0.2f, 0.8f);
            break;
        case TechNodeState::RESEARCHING:
            bgColor = Color3B(204, 153, 25);
            nameColor = Color3B(255, 230, 100);
            borderColor = Color4F(1.0f, 0.9f, 0.2f, 1.0f);
            break;
        case TechNodeState::IN_PROGRESS:
            bgColor = Color3B(127, 127, 76);
            break;
        case TechNodeState::ACTIVATED:
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
    costLabel->setColor(costColor);

    CCLOG("Updated UI state for tech %d: name color = (%d, %d, %d)",
        techId, nameColor.r, nameColor.g, nameColor.b);

    // 更新进度条（如果有）
    if (progressBar && _techTree) {
        int progress = _techTree->getTechProgress(techId);
        int cost = _techTree->getTechCost(techId);

        if (cost > 0 && progress > 0) {
            float width = (NODE_WIDTH - 10) * ((float)progress / cost);
            progressBar->clear();

            Color4F progressColor;
            if (state == TechNodeState::RESEARCHING) {
                progressColor = Color4F(0.9f, 0.7f, 0.1f, 1.0f);
            }
            else if (state == TechNodeState::ACTIVATED) {
                progressColor = Color4F(0.1f, 0.8f, 0.1f, 1.0f);
            }
            else {
                progressColor = Color4F(0.2f, 0.6f, 0.2f, 1.0f);
            }

            progressBar->drawSolidRect(Vec2(5, NODE_HEIGHT - 12),
                Vec2(5 + width, NODE_HEIGHT - 4), progressColor);

            // 更新成本显示为进度
            std::string progressText = std::to_string(progress) + "/" + std::to_string(cost);
            costLabel->setString(progressText);
        }
        else {
            progressBar->clear();
            // 如果没有进度，显示原始成本
            auto techInfo = _techTree->getTechInfo(techId);
            if (techInfo) {
                costLabel->setString(std::to_string(techInfo->cost));
            }
        }
    }
}

void TechTreePanel::onTechNodeClicked(Ref* sender, Widget::TouchEventType type) {
    if (type != Widget::TouchEventType::ENDED) return;

    auto button = dynamic_cast<Button*>(sender);
    if (!button || !_techTree) return;

    int techId = button->getTag();
    showTechDetail(techId);

    //CCLOG("==========================================");
    //CCLOG("TECH NODE CLICK EVENT");
    //CCLOG("Button clicked for tech ID: %d", techId);
    //CCLOG("Calling showTechDetail...");

    //// 添加一个直接的测试，确保函数被调用
    //auto testLabel = Label::createWithTTF("TEST DIRECT - Before showDetail",
    //    "fonts/arial.ttf", 20);
    //testLabel->setPosition(Vec2(200, 300));
    //testLabel->setColor(Color3B::RED);
    //this->addChild(testLabel, 1000);
    //testLabel->runAction(Sequence::create(
    //    DelayTime::create(1.0f),
    //    FadeOut::create(0.5f),
    //    RemoveSelf::create(),
    //    nullptr
    //));

    //// 然后调用 showDetail
    //this->showTechDetail(techId);

    //// 再添加一个测试标签
    //auto testLabel2 = Label::createWithTTF("TEST DIRECT - After showDetail",
    //    "fonts/arial.ttf", 20);
    //testLabel2->setPosition(Vec2(200, 250));
    //testLabel2->setColor(Color3B::GREEN);
    //this->addChild(testLabel2, 1000);
    //testLabel2->runAction(Sequence::create(
    //    DelayTime::create(1.0f),
    //    FadeOut::create(0.5f),
    //    RemoveSelf::create(),
    //    nullptr
    //));

    //CCLOG("showTechDetail called");

    if (_techTree->isResearchable(techId) && !_techTree->isActivated(techId)) {
        setAsCurrentResearch(techId);
    }
}

void TechTreePanel::showTechDetail(int techId) {

    CCLOG("showTechDetail called for tech: %d", techId);

    if (!_techTree) {
        CCLOG("ERROR: _techTree is null!");
        return;
    }

    auto techInfo = _techTree->getTechInfo(techId);
    if (!techInfo) {
        CCLOG("ERROR: No tech info for id: %d", techId);
        return;
    }

    CCLOG("Tech name: %s", techInfo->name.c_str());

    // 隐藏旧的详情面板
    hideTechDetail();

    // 确保在主线程创建UI
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建详情面板 - 确保位置在可见区域内
    _detailPanel = LayerColor::create(Color4B(0, 0, 0, 220), 350, 250);

    // 计算位置，确保不超出屏幕
    float panelX = visibleSize.width - 370;
    float panelY = 150;

    // 如果超出右边界，调整位置
    if (panelX < 20) {
        panelX = 20;
    }

    // 如果超出上边界，调整位置
    if (panelY + 250 > visibleSize.height - 100) {
        panelY = visibleSize.height - 350;
    }

    _detailPanel->setPosition(Vec2(panelX, panelY));
    _detailPanel->setName("detail_panel");

    // 添加详情面板到TechTreePanel，而不是场景
    this->addChild(_detailPanel, 10);

    // 科技名称
    auto nameLabel = Label::createWithSystemFont(techInfo->name, "Arial", 28);
    nameLabel->setPosition(Vec2(175, 200));
    nameLabel->setColor(Color3B::YELLOW);
    _detailPanel->addChild(nameLabel);

    // 科技效果
    auto effectLabel = Label::createWithSystemFont(techInfo->effectDescription, "Arial", 18);
    effectLabel->setPosition(Vec2(175, 120));
    effectLabel->setDimensions(320, 120);
    effectLabel->setAlignment(TextHAlignment::LEFT);
    effectLabel->setVerticalAlignment(TextVAlignment::TOP);
    effectLabel->setColor(Color3B::WHITE);
    _detailPanel->addChild(effectLabel);

    // 研究状态
    std::string statusText;
    if (_techTree->isActivated(techId)) {
        statusText = "Already researched";
    }
    else if (_techTree->getCurrentResearch() == techId) {
        statusText = "Researching...";
    }
    else if (_techTree->getTechProgress(techId) > 0) {
        statusText = "Has progress";
    }
    else if (_techTree->isResearchable(techId)) {
        statusText = "Researchable";

        // 添加研究按钮
        auto researchButton = Button::create();
        researchButton->setTitleText("Set as current research");
        researchButton->setTitleFontSize(18);
        researchButton->setTitleColor(Color3B::WHITE);
        researchButton->setContentSize(Size(200, 40));
        researchButton->setPosition(Vec2(175, 60));
        researchButton->setTag(techId);
        researchButton->addClickEventListener([this](Ref* sender) {
            auto button = dynamic_cast<Button*>(sender);
            if (button) {
                int techId = button->getTag();
                CCLOG("Set research button clicked for tech: %d", techId);
                this->setAsCurrentResearch(techId);
                this->hideTechDetail();
            }
            });
        _detailPanel->addChild(researchButton);
    }
    else {
        statusText = "Locked";
    }

    auto statusLabel = Label::createWithSystemFont(statusText, "Arial", 18);
    statusLabel->setPosition(Vec2(175, 30));
    statusLabel->setColor(Color3B(255, 200, 100));
    _detailPanel->addChild(statusLabel);

    CCLOG("Detail panel created and shown");
}

void TechTreePanel::hideTechDetail() {
    if (_detailPanel) {
        _detailPanel->removeFromParent();
        _detailPanel = nullptr;
    }
}

void TechTreePanel::setAsCurrentResearch(int techId) {
    if (!_techTree) return;

    if (_techTree->setCurrentResearch(techId)) {
        refreshUI();
        updateControlPanel();
    }
}

void TechTreePanel::updateControlPanel() {
    if (!_techTree) return;

    int currentResearch = _techTree->getCurrentResearch();

    if (currentResearch > 0) {
        auto techInfo = _techTree->getTechInfo(currentResearch);
        if (techInfo) {
            std::string researchText = u8"当前研究：" + techInfo->name;
            _currentResearchLabel->setString(researchText);

            int progress = _techTree->getTechProgress(currentResearch);
            int cost = _techTree->getTechCost(currentResearch);
            int percent = (cost > 0) ? (progress * 100 / cost) : 0;
            _researchProgressBar->setPercentage(percent);
        }
    }
    else {
        _currentResearchLabel->setString(u8"当前研究：无");
        _researchProgressBar->setPercentage(0);
    }
}

// 更新连接线状态
void TechTreePanel::updateConnectionLines() {
    if (!_techTree) return;

    // 清除所有旧的连接线
    std::vector<Node*> toRemove;
    for (auto child : _contentNode->getChildren()) {
        if (child->getName().find("line_") == 0) {
            toRemove.push_back(child);
        }
    }

    for (auto node : toRemove) {
        node->removeFromParent();
    }

    // 重新创建所有连接线
    for (const auto& pair : _nodeUIMap) {
        int fromTechId = pair.first;
        auto techInfo = _techTree->getTechInfo(fromTechId);
        if (techInfo) {
            for (int toTechId : techInfo->dstTechList) {
                createSplineConnection(fromTechId, toTechId);
            }
        }
    }
}

// 创建尤里卡特效
void TechTreePanel::createEurekaEffect(int techId) {
    auto node = _nodeUIMap[techId];
    if (!node) return;

    // 获取节点位置
    Vec2 nodePos = node->getPosition();
    Vec2 centerPos = nodePos + Vec2(NODE_WIDTH / 2, NODE_HEIGHT / 2);

    // 创建闪光效果
    auto flash = Sprite::create();

    // 使用DrawNode创建圆形闪光
    auto drawNode = DrawNode::create();
    drawNode->drawSolidCircle(Vec2::ZERO, NODE_WIDTH / 2 + 10,
        CC_DEGREES_TO_RADIANS(360), 30,
        Color4F(1.0f, 1.0f, 0.5f, 0.8f)); // 金色闪光
    flash->addChild(drawNode);
    flash->setPosition(centerPos);
    flash->setScale(0.5f);

    // 动画：放大并淡出
    auto scaleUp = ScaleTo::create(0.3f, 1.8f);
    auto fadeOut = FadeOut::create(0.3f);
    auto spawn = Spawn::create(scaleUp, fadeOut, nullptr);
    auto remove = CallFunc::create([flash]() { flash->removeFromParent(); });
    flash->runAction(Sequence::create(spawn, remove, nullptr));

    _contentNode->addChild(flash, 5);

    // 创建"尤里卡!"文字效果
    auto eurekaLabel = Label::createWithTTF("尤里卡!", "fonts/arial.ttf", 28);
    eurekaLabel->setColor(Color3B(255, 255, 0)); // 金色文字
    eurekaLabel->enableShadow(Color4B(0, 0, 0, 128), Size(2, -2));
    eurekaLabel->setPosition(centerPos + Vec2(0, 80));
    eurekaLabel->setOpacity(0);

    // 文字动画：淡入、上浮、淡出
    auto fadeIn = FadeIn::create(0.2f);
    auto moveUp = MoveBy::create(0.8f, Vec2(0, 50));
    auto fadeOut2 = FadeOut::create(0.3f);
    auto remove2 = CallFunc::create([eurekaLabel]() { eurekaLabel->removeFromParent(); });
    eurekaLabel->runAction(Sequence::create(fadeIn, moveUp, fadeOut2, remove2, nullptr));

    _contentNode->addChild(eurekaLabel, 10);

    // 创建粒子效果（如果支持）
    /*
    if (ParticleSystemQuad::create("particles/eureka.plist")) {
        auto particles = ParticleSystemQuad::create("particles/eureka.plist");
        particles->setPosition(centerPos);
        particles->setAutoRemoveOnFinish(true);
        _contentNode->addChild(particles, 3);
    }
    */

    // 添加音效提示（如果有音频系统）
    // SimpleAudioEngine::getInstance()->playEffect("sounds/eureka.mp3");
}

void TechTreePanel::handleTechActivated(int techId, const std::string& techName, const std::string& effect) {
    // 更新节点状态
    updateNodeUIState(techId, TechNodeState::ACTIVATED);

    // 更新连接线
    updateConnectionLines();

    // 更新控制面板
    updateControlPanel();

    // 播放激活特效（比尤里卡特效更简单）
    auto node = _nodeUIMap[techId];
    if (node) {
        // 简单闪光效果
        auto flash = LayerColor::create(Color4B(100, 255, 100, 100), NODE_WIDTH, NODE_HEIGHT);
        flash->setPosition(node->getPosition());
        flash->setOpacity(0);

        auto fadeIn = FadeTo::create(0.2f, 150);
        auto fadeOut = FadeTo::create(0.3f, 0);
        auto remove = CallFunc::create([flash]() { flash->removeFromParent(); });
        flash->runAction(Sequence::create(fadeIn, fadeOut, remove, nullptr));

        _contentNode->addChild(flash, 5);
    }
}

// 处理研究进度事件
void TechTreePanel::handleResearchProgress(int techId, int currentProgress, int totalCost) {
    // 更新节点进度条
    auto container = _nodeUIMap[techId];
    if (!container) return;

    // 直接获取进度条，不需要通过Button
    auto progressBar = dynamic_cast<DrawNode*>(container->getChildByTag(105));
    auto costLabel = dynamic_cast<Label*>(container->getChildByTag(103));

    if (progressBar && totalCost > 0) {
        float width = (NODE_WIDTH - 10) * ((float)currentProgress / totalCost);
        progressBar->clear();

        Color4F progressColor;
        if (_techTree && _techTree->getCurrentResearch() == techId) {
            progressColor = Color4F(0.9f, 0.7f, 0.1f, 1.0f); // 金色进度条
        }
        else {
            progressColor = Color4F(0.2f, 0.6f, 0.2f, 1.0f); // 绿色进度条
        }

        progressBar->drawSolidRect(Vec2(5, NODE_HEIGHT - 8),
            Vec2(5 + width, NODE_HEIGHT - 4), progressColor);

        // 更新成本标签显示进度
        if (costLabel) {
            std::string progressText = std::to_string(currentProgress) + "/" + std::to_string(totalCost);
            costLabel->setString(progressText);
        }
    }

    // 如果是当前研究，更新控制面板
    if (_techTree && _techTree->getCurrentResearch() == techId) {
        updateControlPanel();
    }
}

// 处理尤里卡事件
void TechTreePanel::handleEurekaTriggered(int techId, const std::string& techName) {
    // 播放尤里卡特效
    createEurekaEffect(techId);

    // 更新节点状态（可能有进度变化）
    if (_techTree) {
        TechNodeState state = TechNodeState::LOCKED;
        if (_techTree->isActivated(techId)) {
            state = TechNodeState::ACTIVATED;
        }
        else if (_techTree->getCurrentResearch() == techId) {
            state = TechNodeState::RESEARCHING;
        }
        else if (_techTree->getTechProgress(techId) > 0) {
            state = TechNodeState::IN_PROGRESS;
        }
        else if (_techTree->isResearchable(techId)) {
            state = TechNodeState::RESEARCHABLE;
        }

        updateNodeUIState(techId, state);
    }
}

void TechTreePanel::onExit() {
    hideTechDetail();

    if (_techTree && _eventListener) {
        _techTree->removeEventListener(_eventListener);
    }

    // 清理事件监听器
    if (_eventListener) {
        delete _eventListener;
        _eventListener = nullptr;
    }

    Layer::onExit();
}