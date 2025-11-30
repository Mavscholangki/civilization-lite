#include "GameMapLayer.h"
#include "MapGenerator.h"
#include "../Units/Melee/Warrior.h"
#include "../Utils/PathFinder.h" // 引用寻路器

USING_NS_CC;

bool GameMapLayer::init() {
    if (!Layer::init()) return false;

    // 1. 初始化变量
    _isDragging = false;
    _layout = new HexLayout(40.0f); // 【修改】六边形变大一点 (30 -> 40)

    // 2. 初始化触摸监听 (让地图能动！)
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(GameMapLayer::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(GameMapLayer::onTouchMoved, this); // 拖拽
    listener->onTouchEnded = CC_CALLBACK_2(GameMapLayer::onTouchEnded, this); // 点击
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 3. 初始化选中框节点
    _selectionNode = DrawNode::create();
    this->addChild(_selectionNode, 20); // Z=20

    // 4. 生成大地图
    generateMap();

    // 5. 创建单位 (勇士)
    Hex startHex(0, 0);
    // 确保出生点不是海，如果是海就往旁边挪挪
    while (getTerrainCost(startHex) < 0) {
        startHex.q++;
        startHex.r++;
    }

    _myUnit = Settler::create(startHex);
    if (_myUnit) {
        _myUnit->setPosition(_layout->hexToPixel(startHex));
        this->addChild(_myUnit, 30); // Z=30 单位在最上层
    }

    // 6. 把镜头(Layer)移到屏幕中心
    auto visibleSize = Director::getInstance()->getVisibleSize();
    this->setPosition(visibleSize.width / 2, visibleSize.height / 2);

    return true;
}

void GameMapLayer::generateMap() {

    int mapWidth = 120;
    int mapHeight = 50;

    // 获取数据并存入成员变量 _mapData
    _mapData = MapGenerator::generate(mapWidth, mapHeight);

    // 渲染
    for (auto const& item : _mapData) {
        Hex hex = item.first;
        TileData data = item.second;
        Color4F color;

        switch (data.type) {
        case TerrainType::OCEAN:    color = Color4F(0.1f, 0.1f, 0.4f, 1); break; // 深蓝
        case TerrainType::COAST:    color = Color4F(0.2f, 0.5f, 0.7f, 1); break; // 浅蓝

            // 寒带
        case TerrainType::SNOW:     color = Color4F(0.95f, 0.95f, 1.0f, 1); break; // 纯白
        case TerrainType::TUNDRA:   color = Color4F(0.6f, 0.6f, 0.65f, 1); break; // 灰白

            // 热带/温带
        case TerrainType::DESERT:   color = Color4F(0.9f, 0.8f, 0.5f, 1); break; // 沙黄
        case TerrainType::PLAINS:   color = Color4F(0.6f, 0.7f, 0.3f, 1); break; // 枯黄绿
        case TerrainType::GRASSLAND:color = Color4F(0.2f, 0.7f, 0.2f, 1); break; // 鲜绿
        case TerrainType::JUNGLE:   color = Color4F(0.0f, 0.4f, 0.0f, 1); break; // 深绿(雨林)

        case TerrainType::MOUNTAIN: color = Color4F(0.4f, 0.3f, 0.3f, 1); break; // 褐色山脉

        default: color = Color4F::WHITE;
        }
        drawHex(_layout->hexToPixel(hex), _layout->size, color);
    }
}

void GameMapLayer::drawHex(Vec2 pos, float size, Color4F color) {
    auto drawNode = DrawNode::create();
    Vec2 vertices[6];
    for (int i = 0; i < 6; i++) {
        float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
        vertices[i] = Vec2(pos.x + size * cos(rad), pos.y + size * sin(rad));
    }
    drawNode->drawPolygon(vertices, 6, color, 1, Color4F(0.0f, 0.0f, 0.0f, 0.1f));
    this->addChild(drawNode, 0); // Z=0 地形在最底层
}

int GameMapLayer::getTerrainCost(Hex h) {
    if (_mapData.find(h) == _mapData.end()) return -1;
    TileData d = _mapData[h];

    // 不可通行
    if (d.type == TerrainType::OCEAN || d.type == TerrainType::COAST || d.type == TerrainType::MOUNTAIN) return -1;

    // 难走
    if (d.type == TerrainType::JUNGLE || d.type == TerrainType::DESERT || d.type == TerrainType::SNOW) return 2;

    return 1;
}

// --- 交互逻辑 ---

bool GameMapLayer::onTouchBegan(Touch* t, Event* e) {
    _isDragging = false;
    return true;
}

void GameMapLayer::onTouchMoved(Touch* t, Event* e) {
    Vec2 delta = t->getDelta();
    // 只有移动距离够大才算拖拽，防止抖动
    if (delta.getLengthSq() > 5.0f) {
        _isDragging = true;
        // 移动整个地图层 (实现镜头移动)
        this->setPosition(this->getPosition() + delta);
    }
}

void GameMapLayer::onTouchEnded(Touch* t, Event* e) {
    if (_isDragging) return; // 如果是拖拽，就不处理点击

    // 1. 计算点击了哪个格子
    // 必须用 convertToNodeSpace，因为地图层被拖动过了，坐标系变了
    Vec2 clickPos = this->convertToNodeSpace(t->getLocation());
    Hex clickHex = _layout->pixelToHex(clickPos);

    CCLOG("点击坐标: %d, %d", clickHex.q, clickHex.r);

    // 2. 画出选中框
    updateSelection(clickHex);

    // 3. 简单的单位移动逻辑
    if (_myUnit) {
        // 使用寻路器找路
        std::vector<Hex> path = PathFinder::findPath(
            _myUnit->gridPos,
            clickHex,
            [this](Hex h) { return this->getTerrainCost(h); }
        );

        if (!path.empty()) {
            _myUnit->moveTo(clickHex, _layout); // 这里为了演示，暂时瞬移
            // 如果你实现了 moveAlongPath，可以用 _myUnit->moveAlongPath(path, _layout);
        }
        else {
            CCLOG("无法到达或目标是海！");
        }
    }

    if (_myUnit && _myUnit->gridPos == clickHex) {
        _selectedUnit = _myUnit; // 记录下来
        if (_onUnitSelected) _onUnitSelected(_myUnit);
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
    _selectionNode->drawPoly(vertices, 6, true, Color4F::YELLOW); // 黄色选中框
    _selectionNode->setLineWidth(3.0f);
}

void GameMapLayer::setOnUnitSelectedCallback(const std::function<void(AbstractUnit*)>& cb) {
    _onUnitSelected = cb;
}

void GameMapLayer::onBuildCityAction() {
    if (!_selectedUnit) return;

    // 1. 获取位置
    Hex pos = _selectedUnit->gridPos;

    // 2. 创建城市
    auto city = BaseCity::create(pos, "Rome"); // 第一座城叫罗马
    city->setPosition(_layout->hexToPixel(pos));
    this->addChild(city, 5); // Z=5，在地块上，单位下
    _cities.push_back(city);

    // 3. 移除单位 (消耗掉开拓者)
    if (_selectedUnit == _myUnit) {
        _myUnit = nullptr; // 指针置空防止野指针
    }
    _selectedUnit->removeFromParent();
    _selectedUnit = nullptr;

    // 4. 清除选中状态
    if (_onUnitSelected) _onUnitSelected(nullptr);
    _selectionNode->clear();

    CCLOG("城市建立成功！");
}

void GameMapLayer::onNextTurnAction() {
    // 让所有城市产出资源
    for (auto city : _cities) {
        city->onTurnEnd();
    }
    // 这里应该通知 GameManager 增加玩家的金币
}
