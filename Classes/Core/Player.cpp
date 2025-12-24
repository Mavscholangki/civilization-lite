#include "Player.h"
#include "../City/BaseCity.h"
#include "../Units/Base/AbstractUnit.h"
#include "GameConfig.h"
#include <Civilizations/CivChina.h>
#include <Civilizations/CivGermany.h>
#include <Civilizations/CivRussia.h>
#include "Scene/GameScene.h"
#include "../Units/Civilian/Settler.h"

USING_NS_CC;

// ==================== 构造函数 ====================
// 关键修改：PolicyManager 需要 CultureTree 指针作为参数
// 由于头文件中 CultureTree 在 PolicyManager 之前声明，这里可以直接传地址
Player::Player()
    : m_policyManager(&m_cultureTree)
    , m_civilization(nullptr)
    , m_startingSettler(nullptr)
{
}

// ==================== 创建和初始化 ====================

Player* Player::create(int playerId, CivilizationType civType) {
    Player* player = new Player();
    if (player && player->init(playerId, civType)) {
        player->autorelease();
        return player;
    }
    CC_SAFE_DELETE(player);
    return nullptr;
}

bool Player::init(int playerId, CivilizationType civType) {
    m_playerId = playerId;
    m_playerName = "Player " + std::to_string(playerId);
    m_state = PlayerState::ACTIVE;
    m_isHuman = true;

    // 设置玩家颜色
    static cocos2d::Color3B playerColors[] = {
        cocos2d::Color3B(255, 0, 0),     // 红色
        cocos2d::Color3B(0, 0, 255),     // 蓝色
        cocos2d::Color3B(0, 255, 0),     // 绿色
        cocos2d::Color3B(255, 255, 0),   // 黄色
        cocos2d::Color3B(255, 0, 255),   // 紫色
        cocos2d::Color3B(0, 255, 255)    // 青色
    };

    int colorIndex = playerId % (sizeof(playerColors) / sizeof(playerColors[0]));
    m_color = playerColors[colorIndex];

    // 创建文明
    createCivilization(civType);

    // 初始化资源
    m_gold = GameConfig::STARTING_GOLD;
    m_scienceStock = 0;
    m_cultureStock = 0;
    m_amenities = GameConfig::STARTING_AMENITIES;

    // 初始化子系统
    m_techTree.initializeTechTree();
    m_cultureTree.initializeCultureTree();

    // 注册事件监听器
    m_techTree.addEventListener(this);
    m_cultureTree.addEventListener(this);

    // 政策管理器已在构造函数中初始化，并在构造函数中自动注册了监听器（在新版PolicySystem中）
    // 这里只需确保游戏数据初始化（通常构造函数里已经调了，这里保险起见不用再调）

    // 初始化回合统计
    m_turnStats = TurnStats();

    CCLOG("Player %d (%s) initialized with civilization %s",
        m_playerId, m_playerName.c_str(),
        m_civilization ? typeid(*m_civilization).name() : "Unknown");

    m_getStartHexFunc = nullptr;
    m_addToMapFunc = nullptr;
    m_checkCityFunc = nullptr;
    m_getTerrainCostFunc = nullptr;

    return true;
}

Player::~Player() {
    CCLOG("Player %d destructor called", m_playerId);

    // 移除事件监听器
    m_techTree.removeEventListener(this);
    m_cultureTree.removeEventListener(this);

    // PolicyManager 会在自己的析构函数中移除监听器，无需手动处理

    // 清理资源
    cleanupResources();
}

void Player::createCivilization(CivilizationType civType) {
    switch (civType) {
    case CivilizationType::CHINA:
        m_civilization = CivChina::create();
        break;
    case CivilizationType::GERMANY:
        m_civilization = CivGermany::create();
        break;
    case CivilizationType::RUSSIA:
        m_civilization = CivRussia::create();
        break;
    default:  // 处理意外情况
        CCLOG("Warning: Unknown civilization type %d, using default", (int)civType);
        m_civilization = BaseCiv::create();
        break;
    }

    if (m_civilization) {
        m_civilization->retain();
        CCLOG("Created civilization: %s", typeid(*m_civilization).name());
    }
    else {
        CCLOG("Failed to create civilization");
    }
}

void Player::cleanupResources() {
    if (m_civilization) {
        m_civilization->release();
        m_civilization = nullptr;
    }

    for (auto city : m_cities) {
        city->release();
    }
    m_cities.clear();

    for (auto unit : m_units) {
        unit->release();
    }
    m_units.clear();
}

// ==================== 回合管理 ====================

void Player::onTurnBegin() {
    m_turnStats = TurnStats();

    // 更新单位状态
    for (auto unit : m_units) {
        unit->onTurnStart();
    }

    if (m_cities.empty()) {
        CCLOG("Player %d has no cities!", m_playerId);
        int maintenance = calculateMaintenanceCost();
        m_gold -= maintenance;
        if (m_gold < 0) m_gold = 0;
        dispatchResourceChangedEvent();
        return;
    }

    // 1. 收集所有城市产出
    Yield totalYield = calculateTotalYield();

    // 记录基础产出
    m_turnStats.goldGenerated = totalYield.goldYield;
    m_turnStats.scienceGenerated = totalYield.scienceYield;
    m_turnStats.cultureGenerated = totalYield.cultureYield;

    // 2. 应用文明特性加成
    totalYield.scienceYield = applyScienceBonus(totalYield.scienceYield);
    totalYield.cultureYield = applyCultureBonus(totalYield.cultureYield);

    if (m_civilization) {
        totalYield.productionYield = static_cast<int>(totalYield.productionYield * m_civilization->getProductionBonus());
        totalYield.goldYield = static_cast<int>(totalYield.goldYield * m_civilization->getGoldBonus());
    }

    // 3. 应用政策卡加成 (新API：getYieldModifier返回百分比数值，例如10.0f)
    float prodMod = m_policyManager.getYieldModifier(EffectType::MODIFIER_PRODUCTION);
    float goldMod = m_policyManager.getYieldModifier(EffectType::MODIFIER_GOLD);
    float sciMod = m_policyManager.getYieldModifier(EffectType::MODIFIER_SCIENCE);
    float cultMod = m_policyManager.getYieldModifier(EffectType::MODIFIER_CULTURE);

    // 应用百分比加成 (base * (1 + mod/100))
    totalYield.productionYield = static_cast<int>(totalYield.productionYield * (1.0f + prodMod / 100.0f));
    totalYield.goldYield = static_cast<int>(totalYield.goldYield * (1.0f + goldMod / 100.0f));
    totalYield.scienceYield = static_cast<int>(totalYield.scienceYield * (1.0f + sciMod / 100.0f));
    totalYield.cultureYield = static_cast<int>(totalYield.cultureYield * (1.0f + cultMod / 100.0f));

    // 4. 更新资源库存
    m_gold += totalYield.goldYield;
    addScience(totalYield.scienceYield);
    addCulture(totalYield.cultureYield);

    // 5. 更新研究进度
    updateResearchProgress();

    

    // 7. 计算维护费
    int maintenance = calculateMaintenanceCost();
    m_gold -= maintenance;
    if (m_gold < 0) {
        m_gold = 0;
        CCLOG("Player %d has negative gold after maintenance!", m_playerId);
    }

    // 8. 发送事件
    dispatchResourceChangedEvent();

    CCLOG("Player %d turn begin: Gold=%d(+%d-%d), Science=%d(+%d), Culture=%d(+%d)",
        m_playerId, m_gold, totalYield.goldYield, maintenance,
        m_scienceStock, totalYield.scienceYield,
        m_cultureStock, totalYield.cultureYield);
}

void Player::dispatchResourceChangedEvent() {
    ValueMap data;
    data["player_id"] = m_playerId;
    data["gold"] = m_gold;
    data["gold_per_turn"] = calculateNetGoldPerTurn();
    data["science_per_turn"] = getSciencePerTurn();
    data["culture_per_turn"] = getCulturePerTurn();
    data["science_stock"] = m_scienceStock;
    data["culture_stock"] = m_cultureStock;

    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
        "player_resource_changed", &data
    );

    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
        "hud_update_resources"
    );
}

void Player::updateResearchProgress() {
    // 科技研究
    int currentTechId = m_techTree.getCurrentResearch();
    if (currentTechId != -1 && m_scienceStock > 0) {
        const TechNode* techNode = m_techTree.getTechInfo(currentTechId);
        if (techNode && !techNode->activated) {
            int scienceToUse = m_scienceStock;
            int remaining = techNode->cost - techNode->progress;
            if (scienceToUse > remaining) scienceToUse = remaining;

            if (scienceToUse > 0) {
                m_techTree.updateProgress(scienceToUse);
                m_scienceStock -= scienceToUse;
            }
        }
    }

    // 文化研究
    int currentCivicId = m_cultureTree.getCurrentResearch();
    if (currentCivicId != -1 && m_cultureStock > 0) {
        const CultureNode* cultureNode = m_cultureTree.getCultureInfo(currentCivicId);
        if (cultureNode && !cultureNode->activated) {
            int cultureToUse = m_cultureStock;
            int remaining = cultureNode->cost - cultureNode->progress;
            if (cultureToUse > remaining) cultureToUse = remaining;

            if (cultureToUse > 0) {
                m_cultureTree.updateProgress(cultureToUse);
                m_cultureStock -= cultureToUse;

                // 文化完成时，政策槽位由PolicyManager监听CultureUnlocked事件自动更新
                // 但为了确保UI同步，可以调用一次 updatePolicySlots (实际上它调用updateGovernmentSlots)
                if (cultureToUse >= remaining) {
                    updatePolicySlots();
                }
            }
        }
    }
}

void Player::onTurnEnd() {
    for (auto city : m_cities) {
        city->onTurnEnd();
    }
    CCLOG("Player %d turn end - Final Gold: %d", m_playerId, m_gold);
}

// ... (城市管理、单位管理 部分代码省略，因为不需要修改) ...
// 请保留原有的 addCity, removeCity, getCapital, calculateTotalYield 等函数
// 它们不需要变动，直接复制原代码即可

BaseCity* Player::getCapital() const {
    if (m_cities.empty()) return nullptr;
    return m_cities[0];
}

Yield Player::calculateTotalYield() const {
    Yield total{};
    for (auto city : m_cities) {
        if (city) {
            total = total + city->cityYield;
        }
    }
    return total;
}

Yield Player::calculateBaseYield() const {
    Yield base{};
    for (auto city : m_cities) {
        if (city) {
            base.foodYield += 2;
            base.productionYield += 20;
            base.goldYield += 20;
            base.scienceYield += 50;
            base.cultureYield += 30;
        }
    }
    return base;
}

// ... (省略 addUnit, removeUnit 等标准函数) ...

void Player::addCity(BaseCity* city) {
    if (city) {
        m_cities.push_back(city);
        city->updatePanel();
        city->retain();
        if (m_cities.size() == 1) {
            addControlledCapital(m_playerId);
        }
    }
}

void Player::removeCity(BaseCity* city) {
    auto it = std::find(m_cities.begin(), m_cities.end(), city);
    if (it != m_cities.end()) {
        bool wasCapital = (city == getCapital());
        (*it)->release();
        m_cities.erase(it);
        if (wasCapital) removeControlledCapital(m_playerId);
        if (m_cities.empty()) m_state = PlayerState::DEFEATED;
        else if (wasCapital) addControlledCapital(m_playerId);
    }
}

bool Player::isCapital(BaseCity* city) const {
    if (m_cities.empty() || !city) return false;
    return city == m_cities[0];
}

void Player::addControlledCapital(int playerId) {
    auto it = std::find(m_vicprogress.controlledCapitals.begin(), m_vicprogress.controlledCapitals.end(), playerId);
    if (it == m_vicprogress.controlledCapitals.end()) {
        m_vicprogress.controlledCapitals.push_back(playerId);
    }
}

void Player::removeControlledCapital(int playerId) {
    auto it = std::find(m_vicprogress.controlledCapitals.begin(), m_vicprogress.controlledCapitals.end(), playerId);
    if (it != m_vicprogress.controlledCapitals.end()) {
        m_vicprogress.controlledCapitals.erase(it);
    }
}

std::vector<int> Player::getControlledCapitals() const {
    return m_vicprogress.controlledCapitals;
}

bool Player::controlsCapitalOf(int playerId) const {
    auto it = std::find(m_vicprogress.controlledCapitals.begin(), m_vicprogress.controlledCapitals.end(), playerId);
    return it != m_vicprogress.controlledCapitals.end();
}

void Player::addUnit(AbstractUnit* unit) {
    if (unit) {
        m_units.push_back(unit);
        unit->retain();
    }
}

void Player::removeUnit(AbstractUnit* unit) {
    auto it = std::find(m_units.begin(), m_units.end(), unit);
    if (it != m_units.end()) {
        (*it)->release();
        m_units.erase(it);
    }
}

void Player::setMapCallbacks(std::function<Hex()> getStartHexFunc,
    std::function<void(AbstractUnit*)> addToMapFunc,
    std::function<bool(Hex)> checkCityFunc,
    std::function<int(Hex)> getTerrainCostFunc) {
    m_getStartHexFunc = getStartHexFunc;
    m_addToMapFunc = addToMapFunc;
    m_checkCityFunc = checkCityFunc;
    m_getTerrainCostFunc = getTerrainCostFunc;

    CCLOG("Player %d map callbacks set", m_playerId);
}

AbstractUnit* Player::createStartingSettler(cocos2d::Node* parentNode,
    std::function<Hex()> getStartHexFunc,
    std::function<void(AbstractUnit*)> addToMapFunc,
    std::function<bool(Hex)> checkCityFunc) {

    // 如果提供了参数，更新回调函数
    if (getStartHexFunc) m_getStartHexFunc = getStartHexFunc;
    if (addToMapFunc) m_addToMapFunc = addToMapFunc;
    if (checkCityFunc) m_checkCityFunc = checkCityFunc;

    // 检查必要的回调函数
    if (!m_getStartHexFunc) {
        CCLOG("Error: Player %d cannot create settler - no start hex function", m_playerId);
        return nullptr;
    }

    if (!m_addToMapFunc) {
        CCLOG("Error: Player %d cannot create settler - no add to map function", m_playerId);
        return nullptr;
    }

    // 获取起始位置
    Hex startHex = m_getStartHexFunc();

    // 寻找最近的陆地
    int safeGuard = 0;
    while (m_getTerrainCostFunc && m_getTerrainCostFunc(startHex) < 0 && safeGuard < 500) {
        startHex.q++; // 向右移动

        if (safeGuard % 20 == 0) {
            startHex.q -= 20;
            startHex.r++;
        }
        safeGuard++;
    }

    CCLOG("Player %d starting position: Hex(%d, %d)", m_playerId, startHex.q, startHex.r);

    // 创建开拓者单位
    auto unit = Settler::create();
    if (unit && unit->initUnit(m_playerId, startHex)) {
        unit->autorelease();

        // 设置城市检查回调
        if (m_checkCityFunc) {
            unit->onCheckCity = m_checkCityFunc;
        }

        m_startingSettler = unit;

        // 添加到玩家单位列表
        addUnit(unit);

        // 通过回调函数添加到地图
        m_addToMapFunc(unit);

        CCLOG("Player %d created starting settler at (%d, %d)",
            m_playerId, startHex.q, startHex.r);

        return unit;
    }

    CCLOG("Error: Player %d failed to create starting settler", m_playerId);
    return nullptr;
}

// ==================== 科技系统接口 ====================

void Player::onTechActivated(int techId, const std::string& techName, const std::string& effect) {
    CCLOG("Player %d: Tech activated - %s (ID: %d)", m_playerId, techName.c_str(), techId);
    if (techId == 22) m_vicprogress.hasSatelliteTech = true;
}

void Player::onResearchProgress(int techId, int currentProgress, int totalCost) {
    // UI更新事件发送
}

void Player::onEurekaTriggered(int techId, const std::string& techName) {
    applyEurekaBonus(techId);
}

void Player::updateUnclockedProduction()
{
    unlockedBuildings.clear();
    unlockedDistricts.clear();
    unlockedUnits.clear();

    auto activatedTech = m_techTree.getActivatedTechList();
    auto activatedCivic = m_cultureTree.getActivatedCultureList();
    auto search = ProductionProgram::findProgram(-1, -1);
    for (auto program : search)
    {
        if (program.first < 100)
        {
            this->unlockedUnits.push_back(
                new ProductionProgram(ProductionProgram::ProductionType::UNIT,
                    program.second.name, Hex(), 0, true, 0));
        }
        else if (program.first % 100 == 0)
        {
            this->unlockedDistricts.push_back(
                new ProductionProgram(ProductionProgram::ProductionType::DISTRICT,
                    program.second.name, Hex(), 0, true, 0));
        }
        else
        {
            this->unlockedBuildings.push_back(
                new ProductionProgram(ProductionProgram::ProductionType::BUILDING,
                    program.second.name, Hex(), 0, true, 0));
        }
    }
    for (int tech : activatedTech)
    {
        auto search = ProductionProgram::findProgram(tech, -1);
        for (auto program : search)
        {
            if (program.first < 100)
            {
                this->unlockedUnits.push_back(
                    new ProductionProgram(ProductionProgram::ProductionType::UNIT,
                        program.second.name, Hex(), 0, true, 0));
            }
            else if (program.first % 100 == 0)
            {
                this->unlockedUnits.push_back(
                    new ProductionProgram(ProductionProgram::ProductionType::DISTRICT,
                        program.second.name, Hex(), 0, true, 0));
            }
            else
            {
                this->unlockedUnits.push_back(
                    new ProductionProgram(ProductionProgram::ProductionType::BUILDING,
                        program.second.name, Hex(), 0, true, 0));
            }
        }
    }

    for (int civic : activatedCivic)
    {
        auto search = ProductionProgram::findProgram(-1, civic);
        for (auto program : search)
        {
            if (program.first < 100)
            {
                this->unlockedUnits.push_back(
                    new ProductionProgram(ProductionProgram::ProductionType::UNIT,
                        program.second.name, Hex(), 0, true, 0));
            }
            else if (program.first % 100 == 0)
            {
                this->unlockedUnits.push_back(
                    new ProductionProgram(ProductionProgram::ProductionType::DISTRICT,
                        program.second.name, Hex(), 0, true, 0));
            }
            else
            {
                this->unlockedUnits.push_back(
                    new ProductionProgram(ProductionProgram::ProductionType::BUILDING,
                        program.second.name, Hex(), 0, true, 0));
            }
        }
    }
}

void Player::getUnlockedProduction(std::vector<ProductionProgram*>& Units,
    std::vector<ProductionProgram*>& Districts,
    std::vector<ProductionProgram*>& Buildings)
{
    updateUnclockedProduction();
    Units = unlockedUnits;
    Districts = unlockedDistricts;
    Buildings = unlockedBuildings;
}

void Player::addScience(int amount) {
    if (amount > 0) {
        m_scienceStock += amount;
        updateResearchProgress();
    }
}

int Player::getSciencePerTurn() const {
    int total = 0;
    for (auto city : m_cities) {
        if (city) total += city->cityYield.scienceYield;
    }
    // 可以在这里累加百分比加成后的总值
    float mod = m_policyManager.getYieldModifier(EffectType::MODIFIER_SCIENCE);
    total = static_cast<int>(total * (1.0f + mod / 100.0f));
    return total;
}

void Player::setCurrentResearch(int techId) {
    m_techTree.setCurrentResearch(techId);
}

// ==================== 文化系统接口 ====================

void Player::onCultureUnlocked(int cultureId, const std::string& cultureName, const std::string& effect) {
    CCLOG("Player %d: Culture unlocked - %s", m_playerId, cultureName.c_str());

    // 关键修改：不需要手动调用 unlockPoliciesForCulture
    // PolicyManager 也是 Listener，它会自动处理解锁逻辑

    // 如果该市政解锁了新政体，需要更新槽位
    updatePolicySlots();
}

void Player::onCultureProgress(int cultureId, int currentProgress, int totalCost) {
    // UI更新
}

void Player::onInspirationTriggered(int cultureId, const std::string& cultureName) {
    applyInspirationBonus(cultureId);
}

void Player::addCulture(int amount) {
    if (amount > 0) {
        m_cultureStock += amount;
        updateResearchProgress();
    }
}

int Player::getCulturePerTurn() const {
    int total = 0;
    for (auto city : m_cities) {
        if (city) total += city->cityYield.cultureYield;
    }
    float mod = m_policyManager.getYieldModifier(EffectType::MODIFIER_CULTURE);
    total = static_cast<int>(total * (1.0f + mod / 100.0f));
    return total;
}

void Player::setCurrentCivic(int cultureId) {
    m_cultureTree.setCurrentResearch(cultureId);
}

// ==================== 政策系统集成 ====================

// 关键修改：新版API是 updateGovernmentSlots，无需参数
void Player::updatePolicySlots() {
    m_policyManager.updateGovernmentSlots();

    // 获取当前政体配置用于日志
    auto config = m_policyManager.getCurrentGovConfig();
    CCLOG("Player %d policy slots updated: Mil=%d, Eco=%d, Wild=%d",
        m_playerId, config.militarySlots, config.economicSlots, config.wildcardSlots);
}

// ==================== 经济系统 ====================

int Player::calculateMaintenanceCost() const {
    int totalCost = 0;
    totalCost += m_units.size(); // 基础维护费

    // 应用政策卡减免
    // 假设 getYieldModifier 返回正数，这里可能是 MAINTENANCE_DISCOUNT 类型的累加值
    // 新版PolicySystem中 EffectType::MAINTENANCE_DISCOUNT 是单位减免数值
    // 我们这里假设是百分比或固定值，先用简单逻辑
    // 如果PolicySystem实现了 MAINTENANCE_DISCOUNT，可以通过 getYieldModifier 获取

    // 这里暂时不做复杂处理
    return totalCost;
}

int Player::calculateNetGoldPerTurn() const {
    int income = 0;
    for (auto city : m_cities) if (city) income += city->cityYield.goldYield;

    // 应用金币加成
    float goldMod = m_policyManager.getYieldModifier(EffectType::MODIFIER_GOLD);
    income = static_cast<int>(income * (1.0f + goldMod / 100.0f));

    return income - calculateMaintenanceCost();
}

void Player::spendGold(int amount) {
    if (amount > 0 && canAfford(amount)) {
        m_gold -= amount;
    }
}

// ==================== 文明加成 ====================

int Player::applyScienceBonus(int baseScience) const {
    if (!m_civilization) return baseScience;
    return m_civilization->applyScienceBonus(baseScience);
}

int Player::applyCultureBonus(int baseCulture) const {
    if (!m_civilization) return baseCulture;
    return m_civilization->applyCultureBonus(baseCulture);
}

void Player::applyEurekaBonus(int techId) {
    if (!m_civilization) return;
    const TechNode* node = m_techTree.getTechInfo(techId);
    if (!node || node->activated) return;

    // 注意：Civilization::applyEurekaBonus 需要 TechTree 指针
    int boost = m_civilization->applyEurekaBonus(techId, const_cast<TechTree*>(&m_techTree));
    if (boost > 0) {
        int current = m_techTree.getCurrentResearch();
        m_techTree.setCurrentResearch(techId);
        m_techTree.updateProgress(boost);
        if (current != -1 && current != techId) {
            m_techTree.setCurrentResearch(current);
        }
    }
}

void Player::applyInspirationBonus(int cultureId) {
    if (!m_civilization) return;
    const CultureNode* node = m_cultureTree.getCultureInfo(cultureId);
    if (!node || node->activated) return;

    int boost = m_civilization->applyInspirationBonus(cultureId, const_cast<CultureTree*>(&m_cultureTree));
    if (boost > 0) {
        int current = m_cultureTree.getCurrentResearch();
        m_cultureTree.setCurrentResearch(cultureId);
        m_cultureTree.updateProgress(boost);
        if (current != -1 && current != cultureId) {
            m_cultureTree.setCurrentResearch(current);
        }
    }
}

std::string Player::getCivilizationTraitName() const {
    if (!m_civilization) return "Unknown";
    return m_civilization->getTraits().name;
}

std::string Player::getCivilizationTraitDescription() const {
    if (!m_civilization) return "No traits";
    return m_civilization->getTraits().description;
}

bool Player::hasCivilizationBonus(const std::string& bonusName) const {
    if (!m_civilization) return false;
    // 简化的检查逻辑，具体取决于 CivilizationTrait 的定义
    auto traits = m_civilization->getTraits();
    if (bonusName == "science_bonus") return traits.scienceBonus > 1.0f;
    if (bonusName == "culture_bonus") return traits.cultureBonus > 1.0f;
    return false;
}

void Player::applyAllCivilizationBonuses() {
    // 这里的具体实现依赖于 BaseCiv 的具体逻辑
    // 主要是为了在加载游戏后刷新状态
    updateCivilizationBonusState();
}

void Player::updateCivilizationBonusState() {
    if (!m_civilization) return;
    // 示例：检查特殊单位解锁
    if (dynamic_cast<CivChina*>(m_civilization)) {
        // ... 特定文明逻辑
    }
}

// ==================== 胜利条件 ====================

bool Player::checkScienceVictory() const {
    return m_vicprogress.hasSatelliteTech && m_vicprogress.hasLaunchedSatellite;
}

bool Player::checkDominationVictory() const {
    return m_vicprogress.hasDominationVictory;
}

// ==================== 序列化 ====================

ValueMap Player::toValueMap() const {
    ValueMap data;

    // 1. 基础属性
    data["playerId"] = m_playerId;
    data["playerName"] = m_playerName;
    data["state"] = (int)m_state;
    data["gold"] = m_gold;
    data["scienceStock"] = m_scienceStock;
    data["cultureStock"] = m_cultureStock;
    data["amenities"] = m_amenities;
    data["isHuman"] = m_isHuman;

    // 颜色
    data["color_r"] = (int)m_color.r;
    data["color_g"] = (int)m_color.g;
    data["color_b"] = (int)m_color.b;

    // 文明类型
    if (dynamic_cast<CivChina*>(m_civilization)) data["civilization"] = "China";
    else if (dynamic_cast<CivGermany*>(m_civilization)) data["civilization"] = "Germany";
    else if (dynamic_cast<CivRussia*>(m_civilization)) data["civilization"] = "Russia";
    else data["civilization"] = "Default";

    // 2. 科技树状态
    ValueMap techData;
    techData["currentResearch"] = m_techTree.getCurrentResearch();

    ValueVector activeTechs;
    for (int id : m_techTree.getActivatedTechList()) {
        activeTechs.push_back(Value(id));
    }
    techData["activatedTechs"] = activeTechs;

    // 保存进度
    // 注意：简化版TechTree可能没有直接遍历所有节点的接口，这里假设通过激活列表和当前研究保存
    // 实际项目中应遍历所有节点保存进度
    data["techTree"] = techData;

    // 3. 文化树状态
    ValueMap cultureData;
    cultureData["currentResearch"] = m_cultureTree.getCurrentResearch();
    cultureData["currentGovernment"] = (int)m_cultureTree.getCurrentGovernment();

    ValueVector activeCivics;
    for (int id : m_cultureTree.getActivatedCultureList()) {
        activeCivics.push_back(Value(id));
    }
    cultureData["activatedCivics"] = activeCivics;
    data["cultureTree"] = cultureData;

    // 4. 政策状态
    // 我们保存已装备的政策 ID
    ValueVector equippedPolicies;
    auto equippedList = m_policyManager.getEquippedPolicies();
    for (const auto& info : equippedList) {
        ValueMap pMap;
        pMap["id"] = info.cardId;
        pMap["type"] = (int)info.slotType;
        pMap["index"] = info.slotIndex;
        equippedPolicies.push_back(Value(pMap));
    }
    data["equippedPolicies"] = equippedPolicies;

    // 5. 胜利进度
    ValueMap vicData;
    vicData["hasSatellite"] = m_vicprogress.hasSatelliteTech;
    vicData["launched"] = m_vicprogress.hasLaunchedSatellite;
    data["victoryProgress"] = vicData;

    return data;
}

bool Player::fromValueMap(const ValueMap& data) {
    // 1. 基础属性
    if (data.count("playerId")) m_playerId = data.at("playerId").asInt();
    if (data.count("gold")) m_gold = data.at("gold").asInt();
    if (data.count("scienceStock")) m_scienceStock = data.at("scienceStock").asInt();
    if (data.count("cultureStock")) m_cultureStock = data.at("cultureStock").asInt();

    // 恢复颜色
    if (data.count("color_r")) {
        m_color = Color3B(data.at("color_r").asInt(), data.at("color_g").asInt(), data.at("color_b").asInt());
    }

    // 恢复文明
    if (data.count("civilization")) {
        std::string civName = data.at("civilization").asString();
        CivilizationType type = CivilizationType::BASIC;
        if (civName == "China") type = CivilizationType::CHINA;
        else if (civName == "Germany") type = CivilizationType::GERMANY;
        else if (civName == "Russia") type = CivilizationType::RUSSIA;
        createCivilization(type);
    }

    // 2. 恢复科技树
    if (data.count("techTree")) {
        auto techData = data.at("techTree").asValueMap();

        // 恢复激活科技
        if (techData.count("activatedTechs")) {
            auto vec = techData.at("activatedTechs").asValueVector();
            for (const auto& v : vec) {
                int techId = v.asInt();
                // 模拟研究完成
                m_techTree.setCurrentResearch(techId);
                m_techTree.updateProgress(99999);
            }
        }
        // 恢复当前研究
        if (techData.count("currentResearch")) {
            m_techTree.setCurrentResearch(techData.at("currentResearch").asInt());
        }
    }

    // 3. 恢复文化树
    if (data.count("cultureTree")) {
        auto cultureData = data.at("cultureTree").asValueMap();

        // 恢复政体
        if (cultureData.count("currentGovernment")) {
            m_cultureTree.switchGovernment((GovernmentType)cultureData.at("currentGovernment").asInt());
        }

        // 恢复激活市政
        if (cultureData.count("activatedCivics")) {
            auto vec = cultureData.at("activatedCivics").asValueVector();
            for (const auto& v : vec) {
                int civicId = v.asInt();
                m_cultureTree.setCurrentResearch(civicId);
                m_cultureTree.updateProgress(99999);
            }
        }

        if (cultureData.count("currentResearch")) {
            m_cultureTree.setCurrentResearch(cultureData.at("currentResearch").asInt());
        }
    }

    // 4. 恢复政策
    // 先更新槽位以匹配政体
    m_policyManager.updateGovernmentSlots();

    if (data.count("equippedPolicies")) {
        auto vec = data.at("equippedPolicies").asValueVector();
        for (const auto& v : vec) {
            auto pMap = v.asValueMap();
            int id = pMap.at("id").asInt();
            int type = pMap.at("type").asInt();
            int index = pMap.at("index").asInt();
            m_policyManager.equipPolicy(id, (PolicyType)type, index);
        }
    }

    // 5. 胜利进度
    if (data.count("victoryProgress")) {
        auto vicData = data.at("victoryProgress").asValueMap();
        m_vicprogress.hasSatelliteTech = vicData.at("hasSatellite").asBool();
        m_vicprogress.hasLaunchedSatellite = vicData.at("launched").asBool();
    }

    return true;
}

void Player::debugPrintStatus() const {
    CCLOG("--- Player %d Status ---", m_playerId);
    CCLOG("Gold: %d, Sci: %d, Cul: %d", m_gold, m_scienceStock, m_cultureStock);

    // 输出产出效率
    CCLOG("Yields per turn: Sci +%d, Cul +%d, Gold +%d",
        getSciencePerTurn(), getCulturePerTurn(), calculateNetGoldPerTurn());

    // 输出政策状态
    auto gov = m_policyManager.getCurrentGovConfig();
    CCLOG("Gov: %s", gov.name.c_str());

    auto policies = m_policyManager.getEquippedPolicies();
    CCLOG("Equipped Policies: %d", (int)policies.size());
    for (auto p : policies) {
        auto card = m_policyManager.getPolicyCard(p.cardId);
        if (card) CCLOG(" - %s", card->name.c_str());
    }
}