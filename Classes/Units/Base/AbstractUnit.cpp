#include "AbstractUnit.h"
#include " ../../Map/GameMapLayer.h"
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
    , _hasActed(false) // 【修改】初始化行动标记
{
}

AbstractUnit::~AbstractUnit() {
}

// 初始化
bool AbstractUnit::initUnit(int ownerId, Hex startPos) {
    if (!Node::init()) return false;

    _ownerId = ownerId;
    _gridPos = startPos;
    _currentHp = getMaxHp();
    _currentMoves = getMaxMoves();
    _state = UnitState::IDLE;
    _hasActed = false; // 【修改】初始状态未行动

    // 1. 尝试加载图片
    std::string path = getSpritePath();
    if (!path.empty() && FileUtils::getInstance()->isFileExist(path)) {
        _unitSprite = Sprite::create(path);
    }

    // 2. 文字模式回退
    if (!_unitSprite) {
        _unitSprite = Sprite::create();
        auto bgNode = DrawNode::create();
        _unitSprite->addChild(bgNode);

        std::string nameText = getUnitName();
        auto label = Label::createWithSystemFont(nameText, "Arial", 18);
        label->enableOutline(Color4B::BLACK, 2);
        label->setPosition(Vec2::ZERO);
        _unitSprite->addChild(label);

        updateVisualColor();
    }
    else {
        updateVisualColor();
    }

    this->addChild(_unitSprite);

    // 3. 选中光圈
    auto ringDraw = DrawNode::create();
    ringDraw->drawCircle(Vec2::ZERO, 30, 0, 30, false, 1.0f, 3.0f, Color4F::GREEN);
    _selectionRing = Sprite::create();
    _selectionRing->addChild(ringDraw);
    _selectionRing->setVisible(false);
    this->addChild(_selectionRing, -1);

    // 4. 血条
    _hpBarNode = DrawNode::create();
    _hpBarNode->setPosition(Vec2(0, 35));
    this->addChild(_hpBarNode, 10);
    updateHpBar();

    // 5. 范围指示器
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

// ============================================================
// 【修改重点】回合开始逻辑：不动回血
// ============================================================
void AbstractUnit::onTurnStart() {
    if (!isAlive()) return;

    int healAmount;
    // 1. 回血逻辑：如果上回合没有行动 (_hasActed == false) 且血不满
    if (!_hasActed && _currentHp < getMaxHp()) {
        // 配置回血量，比如最大生命的 20%
        bool isInCity = false;
        if (onCheckCity) {
            isInCity = onCheckCity(_gridPos);
        }

        // 根据是否在城市决定回血量
        int healAmount = isInCity ? (getMaxHp() / 4) : (getMaxHp() / 10);
        

        // 防止溢出
        int oldHp = _currentHp;
        _currentHp += healAmount;
        if (_currentHp > getMaxHp()) {
            _currentHp = getMaxHp();
        }

        int actualHeal = _currentHp - oldHp;

        // 播放回血飘字 (绿色)
        if (actualHeal > 0) {
            auto label = Label::createWithSystemFont("+" + std::to_string(actualHeal), "Arial", 18);
            label->setColor(Color3B::GREEN);
            label->enableOutline(Color4B::BLACK, 1);
            label->setPosition(Vec2(0, 40));
            this->addChild(label, 20);
            label->runAction(Sequence::create(
                MoveBy::create(0.8f, Vec2(0, 30)),
                RemoveSelf::create(), nullptr
            ));

            updateHpBar();
        }
    }

    // 2. 重置状态
    _currentMoves = getMaxMoves();
    _state = UnitState::IDLE;
    _hasActed = false; // 【修改】重置标记，准备记录新回合的行动

    // 恢复颜色
    if (_unitSprite) {
        updateVisualColor(); // 使用这个函数可以正确处理不同阵营颜色
    }
}

// 移动逻辑
void AbstractUnit::moveTo(Hex targetPos, HexLayout* layout) {
    if (_state != UnitState::IDLE) return;
    if (!layout) return;

    _state = UnitState::MOVING;
    _gridPos = targetPos;

    // 扣除移动力
    _currentMoves = std::max(0, _currentMoves - 1);

    // 【修改】标记本回合已行动 (下回合不能回血)
    _hasActed = true;

    Vec2 pixelPos = layout->hexToPixel(targetPos);
    auto moveAction = MoveTo::create(0.3f, pixelPos);
    auto ease = EaseSineOut::create(moveAction);

    auto callback = CallFunc::create([this]() {
        _state = UnitState::IDLE;
        if (_currentMoves <= 0) {
            if (_unitSprite) _unitSprite->setColor(Color3B::GRAY);
        }
        });

    this->runAction(Sequence::create(ease, callback, nullptr));
}

// 计算战斗力
int AbstractUnit::getCombatPower() const {
    float hpRatio = (float)_currentHp / (float)getMaxHp();
    // 至少保留 1 点攻击力
    return std::max(1, (int)(getBaseAttack() * (0.5f + 0.5f * hpRatio)));
}

// ============================================================
// 【修改重点】攻击逻辑：远程/近战区分 + 反击判断
// ============================================================
void AbstractUnit::attack(AbstractUnit* target, HexLayout* layout) {
    if (_state != UnitState::IDLE) return;
    if (!target || !isAlive()) return;

    _state = UnitState::ATTACKING;
    _currentMoves = 0;
    _hasActed = true; // 【修改】标记已行动

    // --- 平民俘虏逻辑 (保持不变) ---
    if (target->getUnitType() == UnitType::CIVILIAN) {
        Vec2 offset = (target->getPosition() - this->getPosition()).getNormalized() * 20.0f;
        auto seq = Sequence::create(
            MoveBy::create(0.1f, offset),
            CallFunc::create([this, target]() { target->capture(this->_ownerId); }),
            MoveBy::create(0.2f, -offset),
            CallFunc::create([this]() {
                _state = UnitState::IDLE;
                if (_unitSprite) _unitSprite->setColor(Color3B::GRAY);
                }),
            nullptr
        );
        this->runAction(seq);
        return;
    }

    // --- 战斗数值计算 ---
    int myDamage = getCombatPower();

    // 1. 计算距离 (用于判断是否受到反击)
    int distance = this->_gridPos.Hex::distance( target->getGridPos());
    int enemyRange = target->getAttackRange();

    // 2. 判断是否会受到反击
    // 规则：如果你在敌人的攻击范围内，敌人就会反击
    bool willReceiveCounter = (distance <= enemyRange);

    // --- 动画序列 ---
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = target->getPosition();
    Vec2 direction = (targetPos - myPos).getNormalized();

    // 如果是远程攻击(距离>1)，动画幅度小一点；近战大一点
    float lungeDist = (distance > 1) ? 10.0f : 25.0f;
    Vec2 lungeOffset = direction * lungeDist;

    auto forward = MoveBy::create(0.1f, lungeOffset);

    // 伤害回调
    auto hitCallback = CallFunc::create([this, target, myDamage, willReceiveCounter]() {
        // A. 我方造成伤害
        target->takeDamage(myDamage);

        // B. 敌方反击 (如果目标还活着 且 在射程内)
        if (target->isAlive() && willReceiveCounter) {
            int enemyDamage = target->getCombatPower();

            // 可选：近战单位反击远程单位时伤害减半? (目前暂不加)
            // if (this->getAttackRange() > 1 && distance == 1) ...

            this->takeDamage(enemyDamage);
        }
        else if (target->isAlive() && !willReceiveCounter) {
            // C. 白嫖成功提示
            CCLOG("Ranged Attack! No counter-attack.");
        }
        });

    auto backward = MoveBy::create(0.2f, -lungeOffset);
    auto finishCallback = CallFunc::create([this]() {
        _state = UnitState::IDLE;
        if (_unitSprite) _unitSprite->setColor(Color3B::GRAY);
        });

    this->runAction(Sequence::create(forward, hitCallback, backward, finishCallback, nullptr));
}

// 受伤逻辑
int AbstractUnit::takeDamage(int damage) {
    int actualDamage = std::max(1, damage);
    _currentHp -= actualDamage;

    // 飘字效果
    auto label = Label::createWithSystemFont("-" + std::to_string(actualDamage), "Arial", 20);
    label->setColor(Color3B::RED);
    label->enableOutline(Color4B::BLACK, 1); // 加个描边看清楚点
    label->setPosition(Vec2(0, 40));
    this->addChild(label, 20);

    label->runAction(Sequence::create(
        Spawn::create(MoveBy::create(0.5f, Vec2(0, 40)), FadeOut::create(0.5f), nullptr),
        RemoveSelf::create(),
        nullptr
    ));

    updateHpBar();

    if (_currentHp <= 0) {
        onDeath();
    }

    return actualDamage;
}

// 刷新颜色逻辑
void AbstractUnit::updateVisualColor() {
    if (!_unitSprite) return;

    // 图片模式
    if (_unitSprite->getChildrenCount() == 0) {
        if (_ownerId == 0) _unitSprite->setColor(Color3B(100, 150, 255)); // 蓝
        else if (_ownerId == 1) _unitSprite->setColor(Color3B(255, 100, 100)); // 红
        else _unitSprite->setColor(Color3B::WHITE);
        return;
    }

    // 文字/Debug模式
    auto drawNode = dynamic_cast<DrawNode*>(_unitSprite->getChildren().at(0));
    if (drawNode) {
        drawNode->clear();
        Color4F teamColor;
        if (_ownerId == 0) teamColor = Color4F(0.2f, 0.6f, 1.0f, 0.9f);
        else if (_ownerId == 1) teamColor = Color4F(1.0f, 0.4f, 0.4f, 0.9f);
        else teamColor = Color4F::GRAY;

        drawNode->drawDot(Vec2::ZERO, 25, teamColor);
        drawNode->drawCircle(Vec2::ZERO, 25, 0, 30, false, 1.0f, 2.0f, Color4F::WHITE);
    }
}

// 俘虏逻辑
void AbstractUnit::capture(int newOwnerId) {
    if (_ownerId == newOwnerId) return;

    CCLOG("Unit Captured! %s changed owner from %d to %d", getUnitName().c_str(), _ownerId, newOwnerId);

    _ownerId = newOwnerId;
    _currentMoves = 0;
    updateVisualColor();

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

    if (_hpBarNode) _hpBarNode->setVisible(false);
    if (_selectionRing) _selectionRing->setVisible(false);
    if (_rangeNode) _rangeNode->clear();

    auto fade = FadeOut::create(0.5f);
    auto scale = ScaleTo::create(0.5f, 0.1f);
    auto remove = RemoveSelf::create();

    this->runAction(Sequence::create(Spawn::create(fade, scale, nullptr), remove, nullptr));

    CCLOG("Unit %s died at (%d, %d)", getUnitName().c_str(), _gridPos.q, _gridPos.r);
}

// 更新血条
void AbstractUnit::updateHpBar() {
    if (!_hpBarNode) return;

    _hpBarNode->clear();

    float width = 40.0f;
    float height = 5.0f;
    float x = -width / 2;

    _hpBarNode->drawSolidRect(Vec2(x, 0), Vec2(x + width, height), Color4F(0.2f, 0.2f, 0.2f, 0.8f)); // 黑底

    float hpPercent = (float)_currentHp / (float)getMaxHp();
    if (hpPercent > 0) {
        Color4F barColor = Color4F::GREEN;
        if (hpPercent < 0.3f) barColor = Color4F::RED;
        else if (hpPercent < 0.6f) barColor = Color4F(1.0f, 0.8f, 0.0f, 1.0f); // 黄色

        _hpBarNode->drawSolidRect(Vec2(x, 0), Vec2(x + width * hpPercent, height), barColor);
    }
}

// 设置选中
void AbstractUnit::setSelected(bool selected) {
    if (_selectionRing) {
        _selectionRing->setVisible(selected);
        if (selected) {
            _selectionRing->runAction(RepeatForever::create(RotateBy::create(2.0f, 90.0f)));
        }
        else {
            _selectionRing->stopAllActions();
        }
    }
}

// 显示范围
void AbstractUnit::showMoveRange(HexLayout* layout, std::function<int(Hex)> getCost) {
    if (!_rangeNode) return;
    _rangeNode->clear();

    auto reachableHexes = PathFinder::getReachableHexes(_gridPos, _currentMoves, getCost);
    Vec2 myPixelPos = layout->hexToPixel(_gridPos);

    for (const auto& hex : reachableHexes) {
        if (hex == _gridPos) continue;

        Vec2 targetPixelPos = layout->hexToPixel(hex);
        Vec2 localPos = targetPixelPos - myPixelPos;

        Vec2 vertices[6];
        float radius = layout->size * 0.9f;

        for (int i = 0; i < 6; i++) {
            float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
            vertices[i] = Vec2(localPos.x + radius * cos(rad),
                localPos.y + radius * sin(rad));
        }

        Color4F fillColor = Color4F(0.0f, 1.0f, 1.0f, 0.25f);
        Color4F borderColor = Color4F(0.0f, 1.0f, 1.0f, 0.8f);

        _rangeNode->drawPolygon(vertices, 6, fillColor, 2, borderColor);
    }
}

// 隐藏范围
void AbstractUnit::hideMoveRange() {
    if (_rangeNode) {
        _rangeNode->clear();
    }
}