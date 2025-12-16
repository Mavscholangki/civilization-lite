/*
* 城市基类
*/
#include "BaseCity.h"

USING_NS_CC;

BaseCity* BaseCity::create(Hex pos, std::string name) {
    BaseCity* pRet = new BaseCity();
    if (pRet && pRet->initCity(pos, name)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet; return nullptr;
}

bool BaseCity::initCity(Hex pos, std::string name) {
    if (!Node::init()) return false;

    this->gridPos = pos;
    this->cityName = name;
    this->population = 1;
    this->cityYield = { 0, 0, 2, 5, 0 };
	// 初始化领土范围 (城市所在格子及其六个邻接格子)
	this->addToTerritory(Hex(pos.q, pos.r));
    this->addToTerritory(Hex(pos.q + 1, pos.r));
    this->addToTerritory(Hex(pos.q, pos.r + 1));
    this->addToTerritory(Hex(pos.q - 1, pos.r));
    this->addToTerritory(Hex(pos.q, pos.r - 1));
    this->addToTerritory(Hex(pos.q + 1, pos.r - 1));
    this->addToTerritory(Hex(pos.q - 1, pos.r + 1));
	drawBoundary(); // 绘制城市边界
    // 绘制城市 (蓝色方块)
    auto draw = DrawNode::create();
    draw->drawSolidRect(Vec2(-15, -15), Vec2(15, 15), Color4F::BLUE);
    // 加个城墙轮廓
    draw->drawRect(Vec2(-15, -15), Vec2(15, 15), Color4F::WHITE);
    _visual = draw;
    this->addChild(_visual);

    // 城市名字
    _nameLabel = Label::createWithSystemFont(name, "Arial", 16);
    _nameLabel->setPosition(Vec2(0, 25));
    _nameLabel->enableOutline(Color4B::BLACK, 1);
    this->addChild(_nameLabel);

    return true;
}

void BaseCity::drawBoundary() // 绘制城市边界
{
	auto draw = DrawNode::create();
	for (auto tile : territory) {
		// 计算每个格子的像素位置
		HexLayout layout(40.0f); // 假设六边形大小为40
		Vec2 center = layout.hexToPixel(tile) - layout.hexToPixel(this->gridPos); // 相对于城市中心的位置
		// 画出边界
		Vec2 vertices[6];
		for (int i = 0; i < 6; i++) {
			float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
			vertices[i] = Vec2(center.x + layout.size * cos(rad), center.y + layout.size * sin(rad)); // 六边形顶点
		}
		draw->drawPoly(vertices, 6, false, Color4F::RED); // 红色边界
	}
	_boundaryVisual = draw;
    this->addChild(_boundaryVisual);
}

void BaseCity::onTurnEnd() {
    // 这里可以处理人口增长逻辑
    CCLOG("City [%s] produced %d Gold, %d Science", cityName.c_str(), cityYield.goldYield, cityYield.scienceYield);
}