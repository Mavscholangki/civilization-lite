/*
* 单位抽象类
*/
#include "AbstractUnit.h"

USING_NS_CC;

bool AbstractUnit::initUnit(Hex startPos) {
    if (!Node::init()) return false;

    this->gridPos = startPos;

    // 根据子类提供的名字加载图片
    // 如果没有图片，先用 DrawNode 画个圆代替，防止报错
    // 这里我们假设你还没有 warrior.png，先画图
    auto draw = DrawNode::create();
    draw->drawDot(Vec2(0, 0), 15, Color4F::RED); // 红色代表兵
    this->addChild(draw);

    /* 等你有图片了，用下面这段代码
    _sprite = Sprite::create(getSpriteName());
    if (_sprite) {
        this->addChild(_sprite);
    }
    */

    return true;
}

void AbstractUnit::moveTo(Hex targetPos, HexLayout* layout) {
    this->gridPos = targetPos;

    // 计算屏幕像素位置
    Vec2 pixelPos = layout->hexToPixel(targetPos);

    // 执行移动动画 (0.5秒走到)
    auto moveAction = MoveTo::create(0.5f, pixelPos);
    this->runAction(EaseSineOut::create(moveAction)); // 加个缓动效果，丝滑

    CCLOG("%s 移动到了 (%d, %d)", getUnitName().c_str(), targetPos.q, targetPos.r);
}