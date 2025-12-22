#include "GameMapLayer.h"
#include "MapGenerator.h"
#include "../Units/Melee/Warrior.h"
#include "../Utils/PathFinder.h"
#include "../Core/GameManager.h"
#include "cocos2d.h"
#define RADIUS 50.0f
USING_NS_CC;

bool GameMapLayer::init() {
    if (!Layer::init()) return false;

    _isDragging = false;
    _layout = new HexLayout(RADIUS); // 尖顶六边形布局

    // 1. 初始化双击变量
    _lastClickHex = Hex(-999, -999);
    _lastClickTime = std::chrono::steady_clock::now();

    // 2. 注册 Touch 监听器 (拖拽 + 单击 + 双击)
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = CC_CALLBACK_2(GameMapLayer::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(GameMapLayer::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(GameMapLayer::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    _selectionNode = DrawNode::create();
    this->addChild(_selectionNode, 20);

    _tilesDrawNode = DrawNode::create();
    this->addChild(_tilesDrawNode, 0);

    _allUnits.clear();
    _cities.clear();
    _selectedUnit = nullptr;
    _myUnit = nullptr;

    generateMap(); // 地图大小是 120 x 50

    // ============================================================
    // 【修改点】：设置出生地在地图中心
    // ============================================================
    // 假设 generateMap 里的宽是120，高是50
    // 轴向坐标的中心大概在 q=60, r=10 左右 (取决于具体的地图生成算法)
    // 这里我们取一个大概的中间值
    Hex startHex(55, 15);

    // 寻找最近的陆地 (防止生在海里)
    // 如果当前点是海 (Cost < 0)，就向右寻找
    int safeGuard = 0;
    while (getTerrainCost(startHex) < 0 && safeGuard < 500) {
        startHex.q++; // 向右移动

        // 如果一行找完了还没找到，换一行继续找
        if (safeGuard % 20 == 0) {
            startHex.q -= 20; // 回退
            startHex.r++;     // 下一行
        }
        safeGuard++;
    }
    // ============================================================

    auto unit = Settler::create();
    unit->initUnit(0, startHex);
    this->addChild(unit, 10);

    unit->onCheckCity = [this](Hex h) {
        return this->getCityAt(h) != nullptr;
        };
    _myUnit = unit;
    _allUnits.push_back(unit);

    if (_myUnit) {
        _myUnit->setPosition(_layout->hexToPixel(startHex));
    }

    // ============================================================
    // 【修改点】：镜头跟随出生点
    // ============================================================
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 unitPos = _layout->hexToPixel(startHex);

    // 计算 Layer 的偏移量，使单位处于屏幕中心
    // LayerPos + UnitPos = ScreenCenter
    // LayerPos = ScreenCenter - UnitPos
    Vec2 centerOffset = Vec2(visibleSize.width / 2, visibleSize.height / 2) - unitPos;
    this->setPosition(centerOffset);

    return true;
}


void GameMapLayer::generateMap() {
    int mapWidth = 120;
    int mapHeight = 50;

    _mapData = MapGenerator::generate(mapWidth, mapHeight);

    for (auto const& item : _mapData) {
        Hex hex = item.first;
        TileData data = item.second;
        Color4F color;

        switch (data.type) {
        case TerrainType::OCEAN:    color = Color4F(0.1f, 0.1f, 0.4f, 1); break;
        case TerrainType::COAST:    color = Color4F(0.2f, 0.5f, 0.7f, 1); break;
        case TerrainType::SNOW:     color = Color4F(0.95f, 0.95f, 1.0f, 1); break;
        case TerrainType::TUNDRA:   color = Color4F(0.6f, 0.6f, 0.65f, 1); break;
        case TerrainType::DESERT:   color = Color4F(0.9f, 0.8f, 0.5f, 1); break;
        case TerrainType::PLAINS:   color = Color4F(0.6f, 0.7f, 0.3f, 1); break;
        case TerrainType::GRASSLAND:color = Color4F(0.2f, 0.7f, 0.2f, 1); break;
        case TerrainType::JUNGLE:   color = Color4F(0.0f, 0.4f, 0.0f, 1); break;
        case TerrainType::MOUNTAIN: color = Color4F(0.4f, 0.3f, 0.3f, 1); break;
        default: color = Color4F::WHITE;
        }
        
        drawHexOnNode(_tilesDrawNode, _layout->hexToPixel(hex), _layout->size, color);
        drawTileResources(hex, data);
    }
}

void GameMapLayer::drawHexOnNode(DrawNode* node, Vec2 pos, float size, Color4F color) {
    Vec2 vertices[6];
    for (int i = 0; i < 6; i++) {
        float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
        vertices[i] = Vec2(pos.x + size * cos(rad), pos.y + size * sin(rad));
    }
    node->drawPolygon(vertices, 6, color, 1, Color4F(0.0f, 0.0f, 0.0f, 0.1f));
}

// 绘制地皮资源属性（仅显示高产出的地块）
void GameMapLayer::drawTileResources(Hex hex, const TileData& data) {
    // 只显示总产出 >= 3 的地块，减少 Label 数量
    int totalOutput = data.food + data.production + data.gold + data.science + data.culture;
    if (totalOutput < 3) {
        return;
    }

    Vec2 centerPos = _layout->hexToPixel(hex);
    
    // 创建资源文本（使用中文字代替）
    std::string resourceStr = "";
    
    // 粮=粮食, 产=生产力, 金=金币, 科=科技, 文=文化
    if (data.food > 0) {
        resourceStr += u8"粮" + std::to_string(data.food);
    }
    if (data.production > 0) {
        if (!resourceStr.empty()) resourceStr += " ";
        resourceStr += u8"产" + std::to_string(data.production);
    }
    if (data.gold > 0) {
        if (!resourceStr.empty()) resourceStr += " ";
        resourceStr += u8"金" + std::to_string(data.gold);
    }
    if (data.science > 0) {
        if (!resourceStr.empty()) resourceStr += " ";
        resourceStr += u8"科" + std::to_string(data.science);
    }
    if (data.culture > 0) {
        if (!resourceStr.empty()) resourceStr += " ";
        resourceStr += u8"文" + std::to_string(data.culture);
    }

    // 创建 Label（UTF-8 编码）- 使用更小的字号和缓存
    auto label = Label::createWithSystemFont(resourceStr, "Arial", 9);
    label->setPosition(centerPos);
    label->setColor(Color3B::WHITE);
    this->addChild(label, 10);
}

int GameMapLayer::getTerrainCost(Hex h) {
    if (_mapData.find(h) == _mapData.end()) return -1;
    TileData d = _mapData[h];

    if (d.type == TerrainType::OCEAN || d.type == TerrainType::COAST || d.type == TerrainType::MOUNTAIN) return -1;

    if (d.type == TerrainType::JUNGLE || d.type == TerrainType::DESERT || d.type == TerrainType::SNOW) return 2;

    return 1;
}

bool GameMapLayer::onTouchBegan(Touch* touch, Event* event) {
    _isDragging = false;
    return true; // 返回 true 才能接收后续的 Moved 和 Ended
}

void GameMapLayer::onTouchMoved(Touch* touch, Event* event) {
    Vec2 delta = touch->getDelta();

    // 防抖动阈值
    if (delta.getLengthSq() > 5.0f) {
        _isDragging = true;
        // 直接叠加 delta，实现跟随手指拖动
        this->setPosition(this->getPosition() + delta);
    }
}

void GameMapLayer::onTouchEnded(Touch* touch, Event* event) {
    // 首先判断是否是拖拽，如果是就不处理单元逻辑
    if (_isDragging) return;

    // 1. 将触摸转换 (Touch -> NodeSpace -> Hex)
    Vec2 clickPos = this->convertToNodeSpace(touch->getLocation());
    Hex clickHex = _layout->pixelToHex(clickPos);

    CCLOG("Touch Hex: %d, %d", clickHex.q, clickHex.r);

    // 【修复：添加调试日志】
    CCLOG("SelectedUnit: %s, moves: %d", 
          _selectedUnit ? _selectedUnit->getUnitName().c_str() : "NULL",
          _selectedUnit ? _selectedUnit->getCurrentMoves() : -1);

    // ============================================================
    // 双击逻辑处理
    // ============================================================
    auto now = std::chrono::steady_clock::now();
    long long diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastClickTime).count();

    bool isDoubleTap = false;
    // 检查是否同一个六边形，且间隔小于 300ms
    if (clickHex == _lastClickHex && diff < 300) {
        isDoubleTap = true;
    }

    // 更新记录
    _lastClickTime = now;
    _lastClickHex = clickHex;

    // ============================================================
    // 分支 A: 双击事件（执行移动/攻击）
    // ============================================================
    if (isDoubleTap) {
        // 只有当前选中的我方单位才能执行
        if (_selectedUnit && _selectedUnit->getOwnerId() == 0) {

            // 排除攻击自己目标
            if (clickHex != _selectedUnit->getGridPos()) {

                // 1. 检查攻击
                AbstractUnit* enemy = getUnitAt(clickHex);
                if (enemy && enemy->getOwnerId() != 0) {
                    // handleAttack(enemy);
                    CCLOG("Double Tap -> Attack Enemy!");

                    // 重置双击状态以防止再次触发
                    _lastClickHex = Hex(-999, -999);
                    return;
                }

                // 2. 执行移动 【核心修复】
                auto costFunc = [this](Hex h) { return this->getTerrainCost(h); };
                std::vector<Hex> path = PathFinder::findPath(_selectedUnit->getGridPos(), clickHex, costFunc);

                if (!path.empty()) {
                    // 【修复：计算实际需要的移动力消耗】
                    int pathCost = 0;
                    for (size_t i = 1; i < path.size(); i++) {
                        int tileCost = getTerrainCost(path[i]);
                        if (tileCost < 0) {
                            // 路径无法通过
                            pathCost = INT_MAX;
                            break;
                        }
                        pathCost += tileCost;
                    }

                    // 【修复：检查移动力是否足够】
                    if (pathCost <= _selectedUnit->getCurrentMoves()) {
                        // 执行移动
                        _selectedUnit->moveTo(clickHex, _layout);

                        // 移动成功更新视觉状态
                        updateSelection(clickHex);
                        _selectedUnit->hideMoveRange();

                        // 重置双击状态
                        _lastClickHex = Hex(-999, -999);
                        CCLOG("Double Tap Move -> Cost: %d, Remaining: %d", 
                              pathCost, _selectedUnit->getCurrentMoves());
                    }
                    else {
                        CCLOG("移动力不足！需要：%d，当前：%d", pathCost, _selectedUnit->getCurrentMoves());
                    }
                }
                else {
                    CCLOG("无法找到路径到目标");
                }
            }
        }
        return; // 双击事件处理完毕，直接返回
    }

    // ============================================================
    // 分支 B: 单击事件（执行选择/切换）
    // ============================================================

    // 更新浅蓝色背景选中框框提示用户
    updateSelection(clickHex);

    AbstractUnit* clickedUnit = getUnitAt(clickHex);
    BaseCity* clickedCity = getCityAt(clickHex);

    if (clickedUnit) {
        // --- 情况1: 点击单位 ---
        if (_selectedUnit == clickedUnit) {
            _selectedUnit->hideMoveRange();
            _selectedUnit = nullptr;
            _selectionNode->clear(); // 清除框
            if (_onUnitSelected) _onUnitSelected(nullptr);
            return; // 退出
        }

        if (_selectedUnit != clickedUnit) {
            // 切换选中目标
            if (_selectedUnit) _selectedUnit->hideMoveRange();
            _selectedUnit = clickedUnit;

            // 通知 UI
            if (_onUnitSelected) _onUnitSelected(_selectedUnit);

            // 显示移动范围
            auto costFunc = [this](Hex h) { return this->getTerrainCost(h); };
            _selectedUnit->showMoveRange(_layout, costFunc);
        }
        // 单击单位时关闭城市面板
        if (_onCitySelected) _onCitySelected(nullptr);
    }
    else if (clickedCity) {
        // --- 情况2: 点击城市 ---
        if (_selectedUnit) {
            _selectedUnit->hideMoveRange();
            _selectedUnit = nullptr;
            if (_onUnitSelected) _onUnitSelected(nullptr);
        }
        // 打开城市面板
        if (_onCitySelected) _onCitySelected(clickedCity);
    }
    else {
        // --- 情况3: 点击空地 ---
        // 空地位置可以通过点击移动，或需要再次点击(双击)来移动。
        // 只关闭城市面板
        if (_onCitySelected) _onCitySelected(nullptr);
    }
}

void GameMapLayer::updateSelection(Hex h) {
    _selectionNode->clear();
    Vec2 center = _layout->hexToPixel(h);
    Vec2 vertices[6];
    for (int i = 0; i < 6; i++) {
        float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
        vertices[i] = Vec2(center.x + _layout->size * cos(rad), center.y + _layout->size * sin(rad));
    }
    _selectionNode->drawPoly(vertices, 6, true, Color4F::YELLOW);
    _selectionNode->setLineWidth(3.0f);
}

void GameMapLayer::setOnUnitSelectedCallback(const std::function<void(AbstractUnit*)>& cb) {
    _onUnitSelected = cb;
}

void GameMapLayer::onBuildCityAction() {
    if (!_selectedUnit) return;

    Hex pos = _selectedUnit->getGridPos();

    auto city = BaseCity::create(0, pos, "Rome");
    city->setPosition(_layout->hexToPixel(pos));
    this->addChild(city, 5);
    _cities.push_back(city);

    GameManager* gameManager = GameManager::getInstance();
    if (gameManager) {
        Player* currentPlayer = gameManager->getCurrentPlayer();
        if (currentPlayer) {
            // 添加到玩家的城市列表
            currentPlayer->addCity(city);

            CCLOG("City added to Player %d, total cities: %d",
                currentPlayer->getPlayerId(), currentPlayer->getCityCount());
        }
        else {
            CCLOG("Warning: No current player found");
        }
    }

    if (_selectedUnit == _myUnit) {
        _myUnit = nullptr;
    }
    _selectedUnit->removeFromParent();
    _selectedUnit = nullptr;

    if (_onUnitSelected) _onUnitSelected(nullptr);
    _selectionNode->clear();

    CCLOG("城市建立成功！");
}

void GameMapLayer::onNextTurnAction() {
    for (auto city : _cities) {
        city->onTurnEnd();
    }
}

// 获取指定地块的数据
TileData GameMapLayer::getTileData(Hex h)
{
	if (_mapData.find(h) != _mapData.end()) {
		return _mapData[h];
	}
	return TileData();
}
// 1. 实现设置回调
void GameMapLayer::setOnCitySelectedCallback(const std::function<void(BaseCity*)>& cb) {
    _onCitySelected = cb;
}

// 2. 实现查找城市
BaseCity* GameMapLayer::getCityAt(Hex hex) {
    for (auto city : _cities) {
        if (city->gridPos == hex) {
            return city;
        }
    }
    return nullptr;
}

AbstractUnit* GameMapLayer::getUnitAt(Hex hex) {
    for (auto unit : _allUnits) { // 使用统一的变量名
        if (unit->isAlive() && unit->getGridPos() == hex) {
            return unit;
        }
    }
    return nullptr;
}
