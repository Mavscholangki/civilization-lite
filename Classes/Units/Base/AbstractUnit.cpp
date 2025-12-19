#include "AbstractUnit.h"

USING_NS_CC;

// 构造函数
AbstractUnit::AbstractUnit()
    : _ownerId(-1)
    , _currentHp(0)
    , _currentMoves(0)
    , _state(UnitState::IDLE)
    , _unitSprite(nullptr)
    , _selectionRing(nullptr)
    , _hpBarNode(nullptr)
{
}

AbstractUnit::~AbstractUnit() {
    // Cocos2d 的 autorelease 机制通常会自动处理子节点内存
    // 这里可以处理非 Node 类型的自定义数据清理
}

// 初始化
bool AbstractUnit::initUnit(int ownerId, Hex startPos) {
    if (!Node::init()) return false;

    _ownerId = ownerId;
    _gridPos = startPos;
    _currentHp = getMaxHp();
    _currentMoves = getMaxMoves();
    _state = UnitState::IDLE;

    // ============================================================
    // 修改开始：文字 + 图形模式
    // ============================================================

    // 1. 尝试加载图片 (虽然你现在没有，但为了以后有了图片不用改代码，保留这行)
    std::string path = getSpritePath();
    if (!path.empty() && FileUtils::getInstance()->isFileExist(path)) {
        _unitSprite = Sprite::create(path);
    }

    // 2. 如果没加载到图片（或者本来就没图），创建文字图标
    if (!_unitSprite) {
        _unitSprite = Sprite::create();

        // A. 绘制背景 (先创建一个空的 DrawNode)
        auto bgNode = DrawNode::create();
        _unitSprite->addChild(bgNode); // Tag 0

        // B. 绘制文字
        std::string nameText = getUnitName();
        auto label = Label::createWithSystemFont(nameText, "Arial", 18);
        label->enableOutline(Color4B::BLACK, 2);
        label->setPosition(Vec2::ZERO);
        _unitSprite->addChild(label);

        // 【关键】调用刷新颜色，根据 _ownerId 画圆
        updateVisualColor();
    }
    else {
        // 如果是有图片的，也刷新一下颜色 tint
        updateVisualColor();
    }

    // ... init 剩余代码 ...

    this->addChild(_unitSprite);

    // ============================================================
    // 修改结束
    // ============================================================

    // 3. 创建选中光圈 (保持原样，或者改大一点适应圆圈)
    auto ringDraw = DrawNode::create();
    ringDraw->drawCircle(Vec2::ZERO, 30, 0, 30, false, 1.0f, 3.0f, Color4F::GREEN);
    _selectionRing = Sprite::create();
    _selectionRing->addChild(ringDraw);
    _selectionRing->setVisible(false);
    this->addChild(_selectionRing, -1);

    // 4. 创建血条节点
    _hpBarNode = DrawNode::create();
    _hpBarNode->setPosition(Vec2(0, 35));
    this->addChild(_hpBarNode, 10);
    updateHpBar();

	// 5. 创建攻击范围节点 (用于显示攻击范围时绘制)
    _rangeNode = DrawNode::create();
    this->addChild(_rangeNode, -10);

    return true;
}

// 瞬间传送
void AbstractUnit::teleportTo(Hex pos, HexLayout* layout) {
    _gridPos = pos;
    if (layout) {
        this->setPosition(layout->hexToPixel(pos));
    }
}

// 回合开始重置
void AbstractUnit::onTurnStart() {
    if (!isAlive()) return;

    _currentMoves = getMaxMoves();
    _state = UnitState::IDLE;

    // 恢复正常颜色 (如果之前行动完变灰了)
    if (_unitSprite) {
        _unitSprite->setColor(Color3B::WHITE);
    }
}

// 移动逻辑
void AbstractUnit::moveTo(Hex targetPos, HexLayout* layout) {
    if (_state != UnitState::IDLE) return;
    if (!layout) return;

    _state = UnitState::MOVING;
    _gridPos = targetPos; // 逻辑坐标立即更新

    // 消耗移动力 (简化逻辑：每次移动耗尽移动力，或根据距离扣除)
    // 这里假设每次移动消耗 1 点，实际项目中应由寻路算法计算 Cost
    _currentMoves = std::max(0, _currentMoves - 1);

    Vec2 pixelPos = layout->hexToPixel(targetPos);

    // 移动动画：EaseSineOut 看起来更有质感
    auto moveAction = MoveTo::create(0.3f, pixelPos);
    auto ease = EaseSineOut::create(moveAction);

    auto callback = CallFunc::create([this]() {
        _state = UnitState::IDLE;
        // 如果移动力耗尽，变灰
        if (_currentMoves <= 0) {
            _unitSprite->setColor(Color3B::GRAY);
        }
        });

    this->runAction(Sequence::create(ease, callback, nullptr));
}

// 计算战斗力
int AbstractUnit::getCombatPower() const {
    // 简单公式：基础攻击力 * (当前血量 / 最大血量)
    // 比如 10攻，剩50%血 -> 战斗力 = 5
    float hpRatio = (float)_currentHp / (float)getMaxHp();
    return (int)(getBaseAttack() * (0.5f + 0.5f * hpRatio));
}

// 攻击逻辑
void AbstractUnit::attack(AbstractUnit* target, HexLayout* layout) {
    if (_state != UnitState::IDLE) return;
    if (!target || !isAlive()) return;

    _state = UnitState::ATTACKING;
    _currentMoves = 0; // 攻击通常消耗所有行动力

    // 【新增逻辑】平民被俘虏判断
    if (target->getUnitType() == UnitType::CIVILIAN) {

        // 1. 播放冲刺动画 (视觉上是冲过去占领)
        Vec2 myPos = this->getPosition();
        Vec2 targetPos = target->getPosition();
        Vec2 offset = (targetPos - myPos).getNormalized() * 20.0f;

        auto seq = Sequence::create(
            MoveBy::create(0.1f, offset),
            CallFunc::create([this, target]() {
                // 2. 核心：不扣血，直接改阵营
                target->capture(this->_ownerId);
                }),
            MoveBy::create(0.2f, -offset),
            CallFunc::create([this]() {
                _state = UnitState::IDLE;
                if (_unitSprite) _unitSprite->setColor(Color3B::GRAY);
                }),
            nullptr
        );
        this->runAction(seq);

        return; // 【重要】直接返回，不执行后面的伤害计算
    }


    // 1. 计算伤害
    int myPower = getCombatPower();
    // 这里可以加入防御减伤逻辑，比如 damage = myPower - target->getDefense()
    int damage = myPower;

    // 2. 播放攻击动画 (向前冲撞)
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = target->getPosition();
    Vec2 direction = targetPos - myPos;
    direction.normalize();

    Vec2 lungeOffset = direction * 20.0f; // 冲20像素

    auto forward = MoveBy::create(0.1f, lungeOffset);
    auto backward = MoveBy::create(0.2f, -lungeOffset);
    auto hitCallback = CallFunc::create([target, damage]() {
        // 在冲撞动作最远点时扣血
        target->takeDamage(damage);
        });

    auto finishCallback = CallFunc::create([this]() {
        _state = UnitState::IDLE;
        _unitSprite->setColor(Color3B::GRAY); // 攻击完变灰
        });

    this->runAction(Sequence::create(forward, hitCallback, backward, finishCallback, nullptr));
}

// 受伤逻辑
int AbstractUnit::takeDamage(int damage) {
    int actualDamage = std::max(1, damage); // 至少扣1点
    _currentHp -= actualDamage;

    // 飘字效果
    auto label = Label::createWithSystemFont("-" + std::to_string(actualDamage), "Arial", 16);
    label->setColor(Color3B::RED);
    label->setPosition(Vec2(0, 40));
    this->addChild(label, 20);
    label->runAction(Sequence::create(
        MoveBy::create(0.5f, Vec2(0, 30)),
        RemoveSelf::create(),
        nullptr
    ));

    updateHpBar();

    if (_currentHp <= 0) {
        onDeath();
    }

    return actualDamage;
}

// 刷新颜色逻辑 (从 init 中提取出来的)
void AbstractUnit::updateVisualColor() {
    if (!_unitSprite) return;

    // 1. 如果是加载了图片的 Sprite (非文字模式)
    // 我们可以直接用 setColor 染色
    if (_unitSprite->getChildrenCount() == 0) {
        if (_ownerId == 0) _unitSprite->setColor(Color3B(50, 150, 255)); // 蓝
        else if (_ownerId == 1) _unitSprite->setColor(Color3B(255, 100, 100)); // 红
        else _unitSprite->setColor(Color3B::WHITE);
        return;
    }

    // 2. 如果是文字圆圈模式 (Debug模式)，我们需要找到那个 DrawNode 并重画
    // 注意：这里假设 init 里添加的第一个子节点是 DrawNode 背景
    auto drawNode = dynamic_cast<DrawNode*>(_unitSprite->getChildren().at(0));
    if (drawNode) {
        drawNode->clear(); // 清除旧的圆点

        Color4F teamColor;
        if (_ownerId == 0) teamColor = Color4F(0.2f, 0.6f, 1.0f, 0.9f); // 蓝
        else if (_ownerId == 1) teamColor = Color4F(1.0f, 0.4f, 0.4f, 0.9f); // 红
        else teamColor = Color4F::GRAY;

        // 重画圆和边框
        drawNode->drawDot(Vec2::ZERO, 25, teamColor);
        drawNode->drawCircle(Vec2::ZERO, 25, 0, 30, false, 1.0f, 2.0f, Color4F::WHITE);
    }
}

// 俘虏逻辑
void AbstractUnit::capture(int newOwnerId) {
    if (_ownerId == newOwnerId) return; // 已经是自己的了

    CCLOG("Unit Captured! %s changed owner from %d to %d", getUnitName().c_str(), _ownerId, newOwnerId);

    _ownerId = newOwnerId;
    _currentMoves = 0; // 被抓后通常本回合不能动

    // 刷新颜色
    updateVisualColor();

    // 可以在这里播放一个被抓的音效或动画
    auto scaleAnim = Sequence::create(
        ScaleTo::create(0.1f, 1.2f),
        ScaleTo::create(0.1f, 1.0f),
        nullptr
    );
    this->runAction(scaleAnim);
}

// 死亡处理
void AbstractUnit::onDeath() {
    _currentHp = 0;
    _state = UnitState::DEAD;

    // 移除血条和光圈
    if (_hpBarNode) _hpBarNode->setVisible(false);
    if (_selectionRing) _selectionRing->setVisible(false);

    // 死亡动画：淡出并缩小
    auto fade = FadeOut::create(0.5f);
    auto scale = ScaleTo::create(0.5f, 0.1f);
    auto remove = RemoveSelf::create(); // 从父节点移除

    this->runAction(Sequence::create(Spawn::create(fade, scale, nullptr), remove, nullptr));

    CCLOG("Unit %s died at (%d, %d)", getUnitName().c_str(), _gridPos.q, _gridPos.r);
}

// 更新血条
void AbstractUnit::updateHpBar() {
    if (!_hpBarNode) return;

    _hpBarNode->clear();

    // 血条尺寸
    float width = 40.0f;
    float height = 5.0f;
    float x = -width / 2;

    // 绘制背景 (红)
    _hpBarNode->drawSolidRect(Vec2(x, 0), Vec2(x + width, height), Color4F(1, 0, 0, 0.5f));

    // 绘制当前血量 (绿)
    float hpPercent = (float)_currentHp / (float)getMaxHp();
    if (hpPercent > 0) {
        _hpBarNode->drawSolidRect(Vec2(x, 0), Vec2(x + width * hpPercent, height), Color4F::GREEN);
    }
}

// 设置选中
void AbstractUnit::setSelected(bool selected) {
    if (_selectionRing) {
        _selectionRing->setVisible(selected);

        // 如果选中，可以让光圈转起来增加动态感
        if (selected) {
            _selectionRing->runAction(RepeatForever::create(RotateBy::create(1.0f, 90.0f)));
        }
        else {
            _selectionRing->stopAllActions();
        }
    }
}

// 实现显示范围
void AbstractUnit::showMoveRange(HexLayout* layout, std::function<int(Hex)> getCost) {
    if (!_rangeNode) return;
    _rangeNode->clear(); // 清除旧的

    // 1. 计算可达格子
    auto reachableHexes = PathFinder::getReachableHexes(_gridPos, _currentMoves, getCost);

    // 2. 遍历并绘制
    Vec2 myPixelPos = layout->hexToPixel(_gridPos); // 单位当前的屏幕位置

    for (const auto& hex : reachableHexes) {
        if (hex == _gridPos) continue; // 不画自己脚下

        // 计算目标格子的相对位置
        Vec2 targetPixelPos = layout->hexToPixel(hex);
        Vec2 localPos = targetPixelPos - myPixelPos;

        // 【关键修复】手动计算6个顶点，确保角度是 -30 度 (尖顶)
        // 这样就和 GameMapLayer 里的 drawHexOnNode 完全重合了
        Vec2 vertices[6];
        float radius = layout->size * 0.9f; // 稍微小一点，留出缝隙

        for (int i = 0; i < 6; i++) {
            // 这里必须是 60 * i - 30，与地图绘制保持一致！
            float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
            vertices[i] = Vec2(localPos.x + radius * cos(rad),
                localPos.y + radius * sin(rad));
        }

        // 颜色配置
        Color4F fillColor = Color4F(0.0f, 1.0f, 1.0f, 0.25f); // 青色半透明填充
        Color4F borderColor = Color4F(0.0f, 1.0f, 1.0f, 0.8f); // 青色边框

        // 绘制：参数分别为 顶点数组, 顶点数, 填充色, 边框粗细, 边框色
        _rangeNode->drawPolygon(vertices, 6, fillColor, 2, borderColor);
    }
}

// 实现隐藏范围
void AbstractUnit::hideMoveRange() {
    if (_rangeNode) {
        _rangeNode->clear();
    }
}