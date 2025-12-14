#include "GameMapLayer.h"
#include "MapGenerator.h"
#include "../Units/Melee/Warrior.h"
#include "../Utils/PathFinder.h"

USING_NS_CC;

bool GameMapLayer::init() {
if (!Layer::init()) return false;

_isDragging = false;
_layout = new HexLayout(40.0f);

auto listener = EventListenerTouchOneByOne::create();
listener->setSwallowTouches(true);
listener->onTouchBegan = CC_CALLBACK_2(GameMapLayer::onTouchBegan, this);
listener->onTouchMoved = CC_CALLBACK_2(GameMapLayer::onTouchMoved, this);
listener->onTouchEnded = CC_CALLBACK_2(GameMapLayer::onTouchEnded, this);
_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

_selectionNode = DrawNode::create();
this->addChild(_selectionNode, 20);

// 创建单个 DrawNode 用于所有地块
_tilesDrawNode = DrawNode::create();
this->addChild(_tilesDrawNode, 0);

generateMap();

    Hex startHex(0, 0);
    while (getTerrainCost(startHex) < 0) {
        startHex.q++;
        startHex.r++;
    }

    _myUnit = Settler::create(startHex);
    if (_myUnit) {
        _myUnit->setPosition(_layout->hexToPixel(startHex));
        this->addChild(_myUnit, 30);
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    this->setPosition(visibleSize.width / 2, visibleSize.height / 2);

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

bool GameMapLayer::onTouchBegan(Touch* t, Event* e) {
    _isDragging = false;
    return true;
}

void GameMapLayer::onTouchMoved(Touch* t, Event* e) {
    Vec2 delta = t->getDelta();
    if (delta.getLengthSq() > 5.0f) {
        _isDragging = true;
        this->setPosition(this->getPosition() + delta);
    }
}

void GameMapLayer::onTouchEnded(Touch* t, Event* e) {
    if (_isDragging) return;

    Vec2 clickPos = this->convertToNodeSpace(t->getLocation());
    Hex clickHex = _layout->pixelToHex(clickPos);

    CCLOG("点击地块: %d, %d", clickHex.q, clickHex.r);

    updateSelection(clickHex);

    if (_myUnit) {
        std::vector<Hex> path = PathFinder::findPath(
            _myUnit->gridPos,
            clickHex,
            [this](Hex h) { return this->getTerrainCost(h); }
        );

        if (!path.empty()) {
            _myUnit->moveTo(clickHex, _layout);
        }
        else {
            CCLOG("无法到达目的地");
        }
    }

    if (_myUnit && _myUnit->gridPos == clickHex) {
        _selectedUnit = _myUnit;
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
    _selectionNode->drawPoly(vertices, 6, true, Color4F::YELLOW);
    _selectionNode->setLineWidth(3.0f);
}

void GameMapLayer::setOnUnitSelectedCallback(const std::function<void(AbstractUnit*)>& cb) {
    _onUnitSelected = cb;
}

void GameMapLayer::onBuildCityAction() {
    if (!_selectedUnit) return;

    Hex pos = _selectedUnit->gridPos;

    auto city = BaseCity::create(pos, "Rome");
    city->setPosition(_layout->hexToPixel(pos));
    this->addChild(city, 5);
    _cities.push_back(city);

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
