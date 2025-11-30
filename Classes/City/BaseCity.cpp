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
    this->goldYield = 5;    // 初始产出
    this->scienceYield = 2;

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

void BaseCity::onTurnEnd() {
    // 这里可以处理人口增长逻辑
    CCLOG("City [%s] produced %d Gold, %d Science", cityName.c_str(), goldYield, scienceYield);
}