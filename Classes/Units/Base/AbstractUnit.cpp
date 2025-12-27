#include "AbstractUnit.h"
#include " ../../Map/GameMapLayer.h"
#include "Core/GameManager.h"
#include "../../Core/Player.h"

USING_NS_CC;

// 构造函数
AbstractUnit::AbstractUnit()
    : ProductionProgram(ProductionProgram::ProductionType::UNIT, "", Hex(), true, 0)
    , _ownerId(-1)
    , _currentHp(0)
    , _currentMoves(0)
    , _state(UnitState::IDLE)
    , _unitSprite(nullptr)
    , _selectionRing(nullptr)
    , _hpBarNode(nullptr)
    , _hasActed(false) // 【修改】初始化行动标记
    , prereqTechID(-1)
{
    switch (_type)
    {
    case UnitName::SETTLER:
        ProductionProgram::_name = "Settler";
        cost = 80;
        purchaseCost = 400;
        break;
    case UnitName::BUILDER:
        ProductionProgram::_name = "Builder";
        cost = 50;
        purchaseCost = 200;
        break;
    case UnitName::WARRIOR:
        ProductionProgram::_name = "Warrior";
        cost = 30;
        purchaseCost = 200;
        break;
    case UnitName::SWORDSMAN:
        ProductionProgram::_name = "Swordsman";
        cost = 40;
        purchaseCost = 500;
        break;
    case UnitName::LINE_INFANTRY:
        ProductionProgram::_name = "LineInfantry";
        cost = 60;
        purchaseCost = 600;
        break;
    case UnitName::ARCHER:
        ProductionProgram::_name = "Archer";
        cost = 30;
        purchaseCost = 400;
        break;
    case UnitName::CROSSBOWMAN:
        ProductionProgram::_name = "CrossBowman";
        cost = 50;
        purchaseCost = 500;
        break;
    case UnitName::MUSKETEERS:
        ProductionProgram::_name = "Settler";
        cost = 80;
        purchaseCost = 600;
        break;
    case UnitName::CATAPULT:
        ProductionProgram::_name = "Settler";
        cost = 30;
        purchaseCost = 200;
        break;
    case UnitName::CANNON:
        ProductionProgram::_name = "Settler";
        cost = 50;
        purchaseCost = 400;
        break;
    case UnitName::BIPLANE:
        ProductionProgram::_name = "Settler";
        cost = 60;
        purchaseCost = 1000;
        break;
    case UnitName::JET_FIGHTER:
        ProductionProgram::_name = "Settler";
        cost = 80;
        purchaseCost = 10600;
        break;
    default:
        break;
    }
}

AbstractUnit::AbstractUnit(std::string unitName)
    : ProductionProgram(ProductionProgram::ProductionType::UNIT, unitName, Hex(), true, 0)
    , _ownerId(-1)
    , _currentHp(0)
    , _currentMoves(0)
    , _state(UnitState::IDLE)
    , _unitSprite(nullptr)
    , _selectionRing(nullptr)
    , _hpBarNode(nullptr)
    , _hasActed(false) // 【修改】初始化行动标记
    , prereqTechID(-1)
{
    if (ProductionProgram::_name == "Settler")
    {
        _type = UnitName::SETTLER;
        cost = 80;
        purchaseCost = 400;
    }
    else if (ProductionProgram::_name == "Builder")
    {
        _type = UnitName::BUILDER;
        cost = 50;
        purchaseCost = 200;
    }
    else if (ProductionProgram::_name == "Warrior")
    {
        _type = UnitName::WARRIOR;
        cost = 30;
        purchaseCost = 200;
    }
    else if (ProductionProgram::_name == "Swordsman")
    {
        _type = UnitName::SWORDSMAN;
        cost = 40;
        purchaseCost = 500;
    }
    else if (ProductionProgram::_name == "LineInfantry")
    {
        _type = UnitName::LINE_INFANTRY;
        cost = 60;
        purchaseCost = 600;
    }
    else if (ProductionProgram::_name == "Archer")
    {
        _type = UnitName::ARCHER;
        cost = 30;
        purchaseCost = 400;
    }
    else if (ProductionProgram::_name == "CrossBowman")
    {
        _type = UnitName::CROSSBOWMAN;
        cost = 50;
        purchaseCost = 500;
    }
    else if (ProductionProgram::_name == "Musketeers")
    {
        _type = UnitName::MUSKETEERS;
        cost = 80;
        purchaseCost = 600;
    }
    else if (ProductionProgram::_name == "Catapult")
    {
        _type = UnitName::CATAPULT;
        cost = 30;
        purchaseCost = 200;
    }
    else if (ProductionProgram::_name == "Cannon")
    {
        _type = UnitName::CANNON;
        cost = 50;
        purchaseCost = 400;
    }
    else if (ProductionProgram::_name == "Biplane")
    {
        _type = UnitName::BIPLANE;
        cost = 60;
        purchaseCost = 1000;
    }
    else if (ProductionProgram::_name == "JetFighter")
    {
        _type = UnitName::JET_FIGHTER;
        cost = 80;
        purchaseCost = 10600;
    }
    else
    {
        // 默认处理，保持_type的当前值或设置为默认值
        // 如果需要，可以添加_type = UnitName::DEFAULT; 或处理错误情况
    }
}

AbstractUnit::~AbstractUnit() {
}

// 初始化
bool AbstractUnit::initUnit(int ownerId, Hex startPos) {
    if (!Node::init()) return false;

	assert(ownerId >= 0 && "Invalid ownerId for AbstractUnit");
    _ownerId = ownerId;
    _gridPos = startPos;
    _currentHp = getMaxHp();
    _currentMoves = getMaxMoves();
    _state = UnitState::IDLE;
    _hasActed = false;

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

    this->addChild(_unitSprite, 50);

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

    // Player 注册
    if (GameManager::getInstance()) {
        auto player = GameManager::getInstance()->getPlayer(_ownerId);
        if (player) {
            player->addUnit(this);
            CCLOG("Unit %s registered to Player %d", getUnitName().c_str(), _ownerId);
        } else {
            CCLOG("Warning: AbstractUnit initialized for non-existent player %d", _ownerId);
        }
    }
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
// 回合开始逻辑：恢复移动力、回血
// ============================================================
void AbstractUnit::onTurnStart() {
    if (!isAlive()) return;

    CCLOG("Unit %s onTurnStart: hasActed=%d, currentMoves=%d->%d", 
          getUnitName().c_str(), _hasActed, _currentMoves, getMaxMoves());

    // 1. 回血逻辑：如果上回合没有行动且血不满
    if (!_hasActed && _currentHp < getMaxHp()) {
        bool isInCity = false;
        if (onCheckCity) {
            isInCity = onCheckCity(_gridPos);
        }

        int healAmount = isInCity ? (getMaxHp() / 4) : (getMaxHp() / 10);
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

    // 2. 恢复移动力（关键修复点）
    _currentMoves = getMaxMoves();
    _state = UnitState::IDLE;
    _hasActed = false;

    // 恢复颜色
    if (_unitSprite) {
        updateVisualColor();
    }
}

// 移动逻辑（修复：接受路径成本参数）
void AbstractUnit::moveTo(Hex targetPos, HexLayout* layout, int pathCost) {
    if (_state != UnitState::IDLE) return;
    if (!layout) return;
    if (_currentMoves <= 0) return;

    // 如果没有传入路径成本，使用直线距离作为后备
    int actualCost = (pathCost > 0) ? pathCost : _gridPos.distance(targetPos);
    
    // 检查移动力是否足够
    if (_currentMoves < actualCost) {
        CCLOG("Unit %s: Insufficient movement points. Need %d, have %d", 
              getUnitName().c_str(), actualCost, _currentMoves);
        return;
    }

    _state = UnitState::MOVING;
    _gridPos = targetPos;

    // 扣除实际路径成本
    _currentMoves = std::max(0, _currentMoves - actualCost);
    _hasActed = true;

    CCLOG("Unit %s moved. Cost: %d, Remaining moves: %d", 
          getUnitName().c_str(), actualCost, _currentMoves);

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
    return std::max(1, (int)(getBaseAttack() * (0.5f + 0.5f * hpRatio)));
}

// ============================================================
// 攻击逻辑
// ============================================================
void AbstractUnit::attack(AbstractUnit* target, HexLayout* layout) {
    if (_state != UnitState::IDLE) return;
    if (!target || !isAlive()) return;

    _state = UnitState::ATTACKING;
    
    if (!canMoveAfterAttack()) {
        _currentMoves = 0;
    }
    
    _hasActed = true;

    // --- 平民俘虏逻辑 ---
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
    int distance = this->_gridPos.distance(target->getGridPos());
    int enemyRange = target->getAttackRange();
    bool willReceiveCounter = (distance <= enemyRange);

    // --- 动画序列 ---
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = target->getPosition();
    Vec2 direction = (targetPos - myPos).getNormalized();

    float lungeDist = (distance > 1) ? 10.0f : 25.0f;
    Vec2 lungeOffset = direction * lungeDist;

    auto forward = MoveBy::create(0.1f, lungeOffset);

    auto hitCallback = CallFunc::create([this, target, myDamage, willReceiveCounter]() {
        target->takeDamage(myDamage);

        if (target->isAlive() && willReceiveCounter) {
            int enemyDamage = target->getCombatPower();
            this->takeDamage(enemyDamage);
        }
        else if (target->isAlive() && !willReceiveCounter) {
            CCLOG("Ranged Attack! No counter-attack.");
        }
        });

    auto backward = MoveBy::create(0.2f, -lungeOffset);
    auto finishCallback = CallFunc::create([this]() {
        _state = UnitState::IDLE;
        if (_currentMoves <= 0) {
            if (_unitSprite) _unitSprite->setColor(Color3B::GRAY);
        }
        });

    this->runAction(Sequence::create(forward, hitCallback, backward, finishCallback, nullptr));
}

// 受伤逻辑
int AbstractUnit::takeDamage(int damage) {
    int actualDamage = std::max(1, damage);
    _currentHp -= actualDamage;

    auto label = Label::createWithSystemFont("-" + std::to_string(actualDamage), "Arial", 20);
    label->setColor(Color3B::RED);
    label->enableOutline(Color4B::BLACK, 1);
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

    if (_unitSprite->getChildrenCount() == 0) {
        if (_ownerId == 0) _unitSprite->setColor(Color3B(100, 150, 255));
        else if (_ownerId == 1) _unitSprite->setColor(Color3B(255, 100, 100));
        else _unitSprite->setColor(Color3B::WHITE);
        return;
    }

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

    // 从旧玩家移除
    if (GameManager::getInstance()) {
        auto oldPlayer = GameManager::getInstance()->getPlayer(_ownerId);
        if (oldPlayer) {
            oldPlayer->removeUnit(this);
        }
    }

    _ownerId = newOwnerId;
    _currentMoves = 0;
    updateVisualColor();

    // 添加到新玩家
    if (GameManager::getInstance()) {
        auto newPlayer = GameManager::getInstance()->getPlayer(newOwnerId);
        if (newPlayer) {
            newPlayer->addUnit(this);
        }
    }

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
    
    // 从玩家列表中移除
    auto removeFromPlayer = CallFunc::create([this]() {
        if (GameManager::getInstance()) {
            auto player = GameManager::getInstance()->getPlayer(_ownerId);
            if (player) {
                player->removeUnit(this);
                CCLOG("Unit %s removed from Player %d", getUnitName().c_str(), _ownerId);
            }
        }
    });

    auto remove = RemoveSelf::create();

    this->runAction(Sequence::create(
        Spawn::create(fade, scale, nullptr), 
        removeFromPlayer,
        remove,
        nullptr
    ));

    CCLOG("Unit %s died at (%d, %d)", getUnitName().c_str(), _gridPos.q, _gridPos.r);
}

// 更新血条
void AbstractUnit::updateHpBar() {
    if (!_hpBarNode) return;

    _hpBarNode->clear();

    float width = 40.0f;
    float height = 5.0f;
    float x = -width / 2;

    _hpBarNode->drawSolidRect(Vec2(x, 0), Vec2(x + width, height), Color4F(0.2f, 0.2f, 0.2f, 0.8f));

    float hpPercent = (float)_currentHp / (float)getMaxHp();
    if (hpPercent > 0) {
        Color4F barColor = Color4F::GREEN;
        if (hpPercent < 0.3f) barColor = Color4F::RED;
        else if (hpPercent < 0.6f) barColor = Color4F(1.0f, 0.8f, 0.0f, 1.0f);

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

// 显示范围（修复：确保与 getReachableHexes 的逻辑一致）
void AbstractUnit::showMoveRange(HexLayout* layout, std::function<int(Hex)> getCost) {
    if (!_rangeNode) return;
    _rangeNode->clear();

    // 【修复】使用与可达范围计算相同的逻辑
    auto reachableHexes = PathFinder::getReachableHexes(_gridPos, _currentMoves, getCost);
    Vec2 myPixelPos = layout->hexToPixel(_gridPos);

    for (const auto& hex : reachableHexes) {
        if (hex == _gridPos) continue; // 跳过当前位置

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

bool AbstractUnit::canErectUnit()
{
    if (prereqTechID == -1 || GameManager::getInstance()->getPlayer(_ownerId)->getTechTree()->isActivated(prereqTechID))
    {
        return true;
    }
    else
        return false;
}
