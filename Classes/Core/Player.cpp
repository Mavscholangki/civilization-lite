// Player.cpp
#include "Player.h"
#include "../City/BaseCity.h"
#include "Units/Base/AbstractUnit.h"
#include "GameConfig.h"
#include <Civilizations/CivChina.h>
#include <Civilizations/CivGermany.h>
#include <Civilizations/CivRussia.h>

USING_NS_CC;

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
    m_faith = GameConfig::STARTING_FAITH;
    m_amenities = GameConfig::STARTING_AMENITIES;
    m_happiness = GameConfig::STARTING_HAPPINESS;
    m_grievances = 0;

    // 初始化子系统
    m_techTree.initializeTechTree();
    m_cultureTree.initializeCultureTree();

    // 注册事件监听器
    m_techTree.addEventListener(this);
    m_cultureTree.addEventListener(this);

    // 设置政策管理器回调
    setupPolicyManagerCallbacks();
    m_policyManager.initializePolicies();

    // 初始化回合统计
    m_turnStats = TurnStats();

    CCLOG("Player %d (%s) initialized with civilization %s",
        m_playerId, m_playerName.c_str(),
        m_civilization ? typeid(*m_civilization).name() : "Unknown");

    return true;
}

Player::~Player() {
    CCLOG("Player %d destructor called", m_playerId);

    // 移除事件监听器
    m_techTree.removeEventListener(this);
    m_cultureTree.removeEventListener(this);

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
    // 释放文明对象
    if (m_civilization) {
        m_civilization->release();
        m_civilization = nullptr;
    }

    // 释放城市对象（它们应该由场景管理，这里只是清除引用）
    for (auto city : m_cities) {
        city->release();
    }
    m_cities.clear();

    // 释放单位对象
    for (auto unit : m_units) {
        unit->release();
    }
    m_units.clear();
}

// ==================== 回合管理 ====================

void Player::onTurnBegin() {
    // 重置回合统计
    m_turnStats = TurnStats();

    // 1. 收集所有城市产出
    Yield totalYield = calculateTotalYield();

    // 更新回合统计
    m_turnStats.scienceGenerated = totalYield.scienceYield;
    m_turnStats.cultureGenerated = totalYield.cultureYield;
    m_turnStats.goldGenerated = totalYield.goldYield;

    // 2. 应用文明特性加成
    totalYield.scienceYield = applyScienceBonus(totalYield.scienceYield);
    totalYield.cultureYield = applyCultureBonus(totalYield.cultureYield);

    // TODO: 应用其他资源加成（生产力、金币、信仰等）
    if (m_civilization) {
        totalYield.productionYield = static_cast<int>(
            totalYield.productionYield * m_civilization->getProductionBonus());
        totalYield.goldYield = static_cast<int>(
            totalYield.goldYield * m_civilization->getGoldBonus());
    }

    // 3. 应用政策卡加成
    auto policyEffects = m_policyManager.getActivePolicyEffects();
    // TODO: 根据政策效果调整产出

    // 4. 更新资源库存
    addScience(totalYield.scienceYield);
    addCulture(totalYield.cultureYield);
    m_gold += totalYield.goldYield;

    // 5. 更新科技和文化进度
    int currentTechId = m_techTree.getCurrentResearch();
    if (currentTechId != -1 && m_scienceStock > 0) {
        m_techTree.updateProgress(m_scienceStock);
        m_scienceStock = 0;
    }

    int currentCivicId = m_cultureTree.getCurrentResearch();
    if (currentCivicId != -1 && m_cultureStock > 0) {
        m_cultureTree.updateProgress(m_cultureStock);
        m_cultureStock = 0;
    }

    // 6. 更新单位状态
    for (auto unit : m_units) {
        // TODO: 恢复单位移动力，治疗单位等
        // unit->onTurnBegin();
    }

    // 7. 计算和维护费
    int maintenance = calculateMaintenanceCost();
    m_gold -= maintenance;
    if (m_gold < 0) {
        m_gold = 0;
    }
}

void Player::onTurnEnd() {
    // 回合结束时的清理工作
    // TODO: 更新城市状态（生产队列等）
    for (auto city : m_cities) {
        // city->onTurnEnd();
    }

    CCLOG("Player %d turn end - Final Gold: %d", m_playerId, m_gold);
}

// ==================== 城市管理 ====================

void Player::addCity(BaseCity* city) {
    if (city) {
        m_cities.push_back(city);
        city->retain();

        // 如果是第一个城市，应用首都加成
        if (m_cities.size() == 1) {
            CCLOG("Player %d founded capital: %s", m_playerId, city->cityName.c_str());
            // TODO: 设置首都标志
        }

        CCLOG("Player %d now has %d cities", m_playerId, (int)m_cities.size());
    }
}

void Player::removeCity(BaseCity* city) {
    auto it = std::find(m_cities.begin(), m_cities.end(), city);
    if (it != m_cities.end()) {
        (*it)->release();
        m_cities.erase(it);

        CCLOG("Player %d lost a city, remaining: %d", m_playerId, (int)m_cities.size());

        // 如果失去所有城市，玩家失败
        if (m_cities.empty()) {
            m_state = PlayerState::DEFEATED;
            CCLOG("Player %d has been defeated (no cities remaining)", m_playerId);
        }
    }
}

BaseCity* Player::getCapital() const {
    if (m_cities.empty()) return nullptr;
    // 假设第一个城市是首都
    return m_cities[0];
}

Yield Player::calculateTotalYield() const {
    Yield total;

    // 累加所有城市的产出
    for (auto city : m_cities) {
        if (city) {
            total = total + city->cityYield;
        }
    }

    return total;
}

Yield Player::calculateBaseYield() const {
    // 计算基础产出（不考虑加成）
    Yield base;

    // 简单实现：每个城市提供基础产出
    for (auto city : m_cities) {
        if (city) {
            base.foodYield += 2;        // 每个城市基础食物
            base.productionYield += 2;  // 每个城市基础生产力
            base.goldYield += 2;        // 每个城市基础金币
            base.scienceYield += 1;     // 每个城市基础科研
            base.cultureYield += 1;     // 每个城市基础文化
        }
    }

    return base;
}

// ==================== 科技系统接口 ====================

void Player::onTechActivated(int techId, const std::string& techName,
    const std::string& effect) {
    CCLOG("Player %d: Tech activated - %s (ID: %d), Effect: %s",
        m_playerId, techName.c_str(), techId, effect.c_str());

    if (techId == 22) {  // 卫星科技ID
        m_vicprogress.hasSatelliteTech = true;
        CCLOG("Player %d unlocked Satellite technology! Can now launch satellite for science victory.",
            m_playerId);
    }
    // TODO: 处理科技激活效果
    // 1. 解锁新的单位类型
    // 2. 解锁新的建筑
    // 3. 解锁新的区域
    // 4. 提供全局加成

    // 通知UI更新
    // Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("tech_activated", data);
}

void Player::onResearchProgress(int techId, int currentProgress, int totalCost) {
    // 更新UI显示研究进度
    CCLOG("Player %d: Research progress - Tech %d: %d/%d",
        m_playerId, techId, currentProgress, totalCost);

    // TODO: 发送进度更新事件到UI
    /*
    ValueMap data;
    data["player_id"] = m_playerId;
    data["tech_id"] = techId;
    data["progress"] = currentProgress;
    data["total_cost"] = totalCost;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("research_progress", data);
    */
}

void Player::onEurekaTriggered(int techId, const std::string& techName) {
    CCLOG("Player %d: Eureka triggered for %s (ID: %d)",
        m_playerId, techName.c_str(), techId);

    applyEurekaBonus(techId);

    // TODO: 通知UI显示尤里卡提示
}

void Player::addScience(int amount) {
    if (amount > 0) {
        m_scienceStock += amount;
        CCLOG("Player %d gained %d science, total: %d",
            m_playerId, amount, m_scienceStock);
    }
}

int Player::getSciencePerTurn() const {
    int total = 0;
    for (auto city : m_cities) {
        if (city) {
            total += city->cityYield.scienceYield;
        }
    }

    // TODO: 应用文明加成、政策加成等

    return total;
}

void Player::setCurrentResearch(int techId) {
    if (m_techTree.setCurrentResearch(techId)) {
        CCLOG("Player %d started researching tech %d", m_playerId, techId);
    }
    else {
        CCLOG("Player %d failed to start researching tech %d", m_playerId, techId);
    }
}

// ==================== 文化系统接口 ====================

void Player::onCultureUnlocked(int cultureId, const std::string& cultureName,
    const std::string& effect) {
    CCLOG("Player %d: Culture unlocked - %s (ID: %d), Effect: %s",
        m_playerId, cultureName.c_str(), cultureId, effect.c_str());

    // 更新政策槽位
    updatePolicySlots();

    // 解锁相关政策
    m_policyManager.unlockPoliciesForCulture(cultureId);

    // TODO: 处理市政解锁的其他效果
    // 1. 解锁新的政体
    // 2. 解锁新的建筑
    // 3. 提供全局加成

    // 通知UI更新
    /*
    ValueMap data;
    data["player_id"] = m_playerId;
    data["culture_id"] = cultureId;
    data["culture_name"] = cultureName;
    data["effect"] = effect;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("culture_unlocked", data);
    */
}

void Player::onCultureProgress(int cultureId, int currentProgress, int totalCost) {
    // 更新UI显示文化进度
    CCLOG("Player %d: Culture progress - Civic %d: %d/%d",
        m_playerId, cultureId, currentProgress, totalCost);

    // TODO: 发送进度更新事件到UI
}

void Player::onInspirationTriggered(int cultureId, const std::string& cultureName) {
    CCLOG("Player %d: Inspiration triggered for %s (ID: %d)",
        m_playerId, cultureName.c_str(), cultureId);

    // 应用文明灵感加成
    applyInspirationBonus(cultureId);

    // TODO: 通知UI显示灵感提示
}

void Player::addCulture(int amount) {
    if (amount > 0) {
        m_cultureStock += amount;
        CCLOG("Player %d gained %d culture, total: %d",
            m_playerId, amount, m_cultureStock);
    }
}

int Player::getCulturePerTurn() const {
    int total = 0;
    for (auto city : m_cities) {
        if (city) {
            total += city->cityYield.cultureYield;
        }
    }

    // TODO: 应用文明加成、政策加成等

    return total;
}

void Player::setCurrentCivic(int cultureId) {
    if (m_cultureTree.setCurrentResearch(cultureId)) {
        CCLOG("Player %d started researching civic %d", m_playerId, cultureId);
    }
    else {
        CCLOG("Player %d failed to start researching civic %d", m_playerId, cultureId);
    }
}

// ==================== 政策系统集成 ====================

void Player::setupPolicyManagerCallbacks() {
    // 设置获取当前政体的回调
    m_policyManager.setGovernmentGetter([this]() -> GovernmentType {
        return m_cultureTree.getCurrentGovernment();
        });

    // 设置获取政策列表的回调
    m_policyManager.setPolicyGetter([this](int cultureId) -> std::vector<int> {
        return m_cultureTree.getPoliciesUnlockedByCulture(cultureId);
        });

    // 设置政策解锁回调
    m_policyManager.setPolicyUnlockedCallback([this](int policyId) {
        CCLOG("Player %d: Policy %d unlocked", m_playerId, policyId);
        // TODO: 处理政策解锁事件，如更新UI
        });

    CCLOG("Player %d policy manager callbacks setup completed", m_playerId);
}

void Player::updatePolicySlots() {
    auto slotInfo = m_cultureTree.getPolicySlotInfo();
    m_policyManager.updatePolicySlots(
        slotInfo.military,
        slotInfo.economic,
        slotInfo.diplomatic,
        slotInfo.wildcard
    );

    CCLOG("Player %d policy slots updated: Military=%d, Economic=%d, Diplomatic=%d, Wildcard=%d",
        m_playerId, slotInfo.military, slotInfo.economic,
        slotInfo.diplomatic, slotInfo.wildcard);
}

// ==================== 外交系统 ====================

void Player::declareWar(int targetPlayerId) {
    DiplomaticRelation& relation = m_diplomaticRelations[targetPlayerId];
    relation.playerId = targetPlayerId;
    relation.isAtWar = true;
    relation.relationship = -100;

    CCLOG("Player %d declared war on Player %d", m_playerId, targetPlayerId);
}

void Player::makePeace(int targetPlayerId) {
    auto it = m_diplomaticRelations.find(targetPlayerId);
    if (it != m_diplomaticRelations.end()) {
        it->second.isAtWar = false;
        it->second.relationship = 0;
        CCLOG("Player %d made peace with Player %d", m_playerId, targetPlayerId);
    }
}

bool Player::isAtWarWith(int playerId) const {
    auto it = m_diplomaticRelations.find(playerId);
    if (it != m_diplomaticRelations.end()) {
        return it->second.isAtWar;
    }
    return false;
}

void Player::setDiplomaticRelation(int playerId, const DiplomaticRelation& relation) {
    m_diplomaticRelations[playerId] = relation;
}

Player::DiplomaticRelation* Player::getDiplomaticRelation(int playerId) {
    auto it = m_diplomaticRelations.find(playerId);
    if (it != m_diplomaticRelations.end()) {
        return &it->second;
    }
    return nullptr;
}

// ==================== 经济系统 ====================

int Player::calculateMaintenanceCost() const {
    int totalCost = 0;

    // 单位维护费（简单实现：每个单位1金币）
    totalCost += m_units.size();

    // 建筑维护费（通过城市计算）
    for (auto city : m_cities) {
        if (city) {
            // 简单实现：每个区域1金币
            // TODO: 需要BaseCity添加getDistrictCount方法
            // totalCost += city->getDistrictCount();
        }
    }

    // 政策维护费
    // TODO: 通过PolicyManager计算政策维护费

    CCLOG("Player %d maintenance cost: %d gold", m_playerId, totalCost);
    return totalCost;
}

int Player::calculateNetGoldPerTurn() const {
    int income = 0;
    for (auto city : m_cities) {
        if (city) {
            income += city->cityYield.goldYield;
        }
    }

    int net = income - calculateMaintenanceCost();
    CCLOG("Player %d net gold per turn: %d (income: %d, maintenance: %d)",
        m_playerId, net, income, calculateMaintenanceCost());

    return net;
}

void Player::spendGold(int amount) {
    if (amount <= 0) return;

    if (canAfford(amount)) {
        m_gold -= amount;
        CCLOG("Player %d spent %d gold, remaining: %d", m_playerId, amount, m_gold);
    }
    else {
        CCLOG("Player %d cannot afford to spend %d gold (has %d)",
            m_playerId, amount, m_gold);
    }
}

// ==================== 文明加成应用方法实现 ====================

int Player::applyScienceBonus(int baseScience) const {
    if (!m_civilization) return baseScience;

    // 应用文明科研加成
    int bonusScience = m_civilization->applyScienceBonus(baseScience);

    // TODO: 这里可以添加其他加成（政策、建筑、奇观等）

    CCLOG("Player %d: Science bonus applied: %d -> %d (Bonus: %.2fx)",
        m_playerId, baseScience, bonusScience, m_civilization->getScienceBonus());

    return bonusScience;
}

int Player::applyCultureBonus(int baseCulture) const {
    if (!m_civilization) return baseCulture;

    // 应用文明文化加成
    int bonusCulture = m_civilization->applyCultureBonus(baseCulture);

    // TODO: 这里可以添加其他加成（政策、建筑、奇观等）

    CCLOG("Player %d: Culture bonus applied: %d -> %d (Bonus: %.2fx)",
        m_playerId, baseCulture, bonusCulture, m_civilization->getCultureBonus());

    return bonusCulture;
}

void Player::applyEurekaBonus(int techId) {
    if (!m_civilization) return;

    // 获取基础尤里卡加成（默认50%）
    const TechNode* techNode = m_techTree.getTechInfo(techId);
    if (!techNode || techNode->activated) return;

    int baseEurekaPoints = techNode->cost / 2;

    // 应用文明尤里卡加成
    int eurekaPoints = m_civilization->applyEurekaBonus(techId, &m_techTree);

    CCLOG("Player %d: Eureka bonus for tech %d: Base=%d, Applied=%d (Boost: %.2fx)",
        m_playerId, techId, baseEurekaPoints, eurekaPoints,
        m_civilization->getEurekaBoost());

    // 直接添加进度（避免循环调用）
    if (eurekaPoints > 0) {
        // 保存当前研究
        int currentTech = m_techTree.getCurrentResearch();

        // 临时设置为这个科技以接收加成
        m_techTree.setCurrentResearch(techId);

        // 添加尤里卡点数
        m_techTree.updateProgress(eurekaPoints);

        // 恢复原来的研究
        if (currentTech != -1 && currentTech != techId) {
            m_techTree.setCurrentResearch(currentTech);
        }
    }
}

void Player::applyInspirationBonus(int cultureId) {
    if (!m_civilization) return;

    // 获取基础灵感加成（默认50%）
    const CultureNode* cultureNode = m_cultureTree.getCultureInfo(cultureId);
    if (!cultureNode || cultureNode->activated) return;

    int baseInspirationPoints = cultureNode->cost / 2;

    // 应用文明灵感加成
    int inspirationPoints = m_civilization->applyInspirationBonus(cultureId, &m_cultureTree);

    CCLOG("Player %d: Inspiration bonus for culture %d: Base=%d, Applied=%d (Boost: %.2fx)",
        m_playerId, cultureId, baseInspirationPoints, inspirationPoints,
        m_civilization->getInspirationBoost());

    // 直接添加进度（避免循环调用）
    if (inspirationPoints > 0) {
        // 保存当前研究
        int currentCulture = m_cultureTree.getCurrentResearch();

        // 临时设置为这个文化以接收加成
        m_cultureTree.setCurrentResearch(cultureId);

        // 添加灵感点数
        m_cultureTree.updateProgress(inspirationPoints);

        // 恢复原来的研究
        if (currentCulture != -1 && currentCulture != cultureId) {
            m_cultureTree.setCurrentResearch(currentCulture);
        }
    }
}

std::string Player::getCivilizationTraitName() const {
    if (!m_civilization) return "Unknown";

    CivilizationTrait traits = m_civilization->getTraits();
    return traits.name;
}

std::string Player::getCivilizationTraitDescription() const {
    if (!m_civilization) return "No civilization traits";

    CivilizationTrait traits = m_civilization->getTraits();
    return traits.description;
}

bool Player::hasCivilizationBonus(const std::string& bonusName) const {
    if (!m_civilization) return false;

    CivilizationTrait traits = m_civilization->getTraits();

    if (bonusName == "extra_initial_tiles") {
        return traits.initialTiles > 3; // 默认是3
    }
    else if (bonusName == "eureka_boost") {
        return traits.eurekaBoost > 0.5f; // 默认是0.5
    }
    else if (bonusName == "inspiration_boost") {
        return traits.inspirationBoost > 0.5f; // 默认是0.5
    }
    else if (bonusName == "science_bonus") {
        return traits.scienceBonus > 1.0f; // 默认是1.0
    }
    else if (bonusName == "culture_bonus") {
        return traits.cultureBonus > 1.0f; // 默认是1.0
    }
    else if (bonusName == "half_cost_industrial") {
        return traits.halfCostIndustrial;
    }
    else if (bonusName == "extra_district_slot") {
        return traits.extraDistrictSlot;
    }
    else if (bonusName == "military_production_bonus") {
        return traits.militaryProductionBonus < 1.0f; // 小于1.0表示成本降低
    }
    else if (bonusName == "builder_charges") {
        return traits.builderCharges > 3; // 默认是3
    }

    return false;
}

void Player::applyAllCivilizationBonuses() {
    if (!m_civilization) return;

    CivilizationTrait traits = m_civilization->getTraits();

    CCLOG("Player %d applying civilization bonuses:", m_playerId);
    CCLOG("  - Civilization: %s", typeid(*m_civilization).name());
    CCLOG("  - Trait Name: %s", traits.name.c_str());
    CCLOG("  - Description: %s", traits.description.c_str());
    CCLOG("  - Initial Tiles: %d", traits.initialTiles);
    CCLOG("  - Eureka Boost: %.2fx", traits.eurekaBoost);
    CCLOG("  - Inspiration Boost: %.2fx", traits.inspirationBoost);
    CCLOG("  - Science Bonus: %.2fx", traits.scienceBonus);
    CCLOG("  - Culture Bonus: %.2fx", traits.cultureBonus);
    CCLOG("  - Half Cost Industrial: %s", traits.halfCostIndustrial ? "Yes" : "No");
    CCLOG("  - Extra District Slot: %s", traits.extraDistrictSlot ? "Yes" : "No");
    CCLOG("  - Military Production Bonus: %.2fx", traits.militaryProductionBonus);
    CCLOG("  - Builder Charges: %d", traits.builderCharges);

    // 更新初始地块（需要在城市创建时应用）
    // TODO: 在创建第一个城市时应用初始地块加成

    // 更新政策管理器中的加成信息
    // TODO: 将文明加成信息传递给政策管理器
}

void Player::updateCivilizationBonusState() {
    // 这个方法可以在游戏过程中更新文明加成状态
    // 例如：当科技或文化解锁后，更新相关的加成

    if (!m_civilization) return;

    // 检查是否有特殊单位解锁
    // 对于中国：检查虎蹲炮是否解锁
    if (dynamic_cast<CivChina*>(m_civilization)) {
        CivChina* chinaCiv = dynamic_cast<CivChina*>(m_civilization);
        bool isTigerCannonUnlocked = chinaCiv->isUniqueUnitUnlocked("虎蹲炮");
        if (isTigerCannonUnlocked) {
            CCLOG("Player %d (China): Tiger Cannon unit is now unlocked!", m_playerId);
        }
    }
}

// ==================== 胜利条件 ====================

bool Player::checkScienceVictory() const {
    return m_vicprogress.hasSatelliteTech && m_vicprogress.hasLaunchedSatellite;
}


bool Player::checkDominationVictory() const {
    // 需要游戏总信息
    // TODO: 获取游戏总城市数，控制所有城市即为统治胜利
    return false;
}

// ==================== 序列化 ====================

ValueMap Player::toValueMap() const {
    ValueMap data;

    // 基本属性
    data["playerId"] = m_playerId;
    data["playerName"] = m_playerName;
    data["state"] = (int)m_state;
    data["gold"] = m_gold;
    data["scienceStock"] = m_scienceStock;
    data["cultureStock"] = m_cultureStock;
    data["faith"] = m_faith;
    data["amenities"] = m_amenities;
    data["happiness"] = m_happiness;
    data["grievances"] = m_grievances;
    data["isHuman"] = m_isHuman;

    // 颜色
    data["color_r"] = (int)m_color.r;
    data["color_g"] = (int)m_color.g;
    data["color_b"] = (int)m_color.b;

    // 文明类型
    if (m_civilization) {
        if (dynamic_cast<CivChina*>(m_civilization)) {
            data["civilization"] = "China";
        }
        else if (dynamic_cast<CivGermany*>(m_civilization)) {
            data["civilization"] = "Germany";
        }
        else if (dynamic_cast<CivRussia*>(m_civilization)) {
            data["civilization"] = "Russia";
        }
        else {
            data["civilization"] = "Default";
        }
    }

    // TODO: 序列化子系统
    // 1. 科技树状态
    // 2. 文化树状态
    // 3. 政策管理器状态
    // 4. 城市列表
    // 5. 单位列表
    // 6. 外交关系

    return data;
}

bool Player::fromValueMap(const ValueMap& data) {
    // TODO: 反序列化实现
    return true;
}

// ==================== 调试和测试 ====================

void Player::debugPrintStatus() const {
    CCLOG("=== Player %d Status ===", m_playerId);
    CCLOG("Name: %s", m_playerName.c_str());
    CCLOG("State: %d", (int)m_state);
    CCLOG("Gold: %d", m_gold);
    CCLOG("Science Stock: %d", m_scienceStock);
    CCLOG("Culture Stock: %d", m_cultureStock);
    CCLOG("Cities: %d", (int)m_cities.size());
    CCLOG("Units: %d", (int)m_units.size());

    // 当前研究
    int currentTech = m_techTree.getCurrentResearch();
    if (currentTech != -1) {
        const TechNode* techNode = m_techTree.getTechInfo(currentTech);
        if (techNode) {
            CCLOG("Current Research: %s (%d/%d)",
                techNode->name.c_str(), techNode->progress, techNode->cost);
        }
    }

    int currentCivic = m_cultureTree.getCurrentResearch();
    if (currentCivic != -1) {
        const CultureNode* cultureNode = m_cultureTree.getCultureInfo(currentCivic);
        if (cultureNode) {
            CCLOG("Current Civic: %s (%d/%d)",
                cultureNode->name.c_str(), cultureNode->progress, cultureNode->cost);
        }
    }

    // 已激活的科技和市政数量
    CCLOG("Activated Techs: %d", (int)m_techTree.getActivatedTechList().size());
    CCLOG("Activated Civics: %d", (int)m_cultureTree.getActivatedCultureList().size());

    // 回合产出
    CCLOG("Science per turn: %d", getSciencePerTurn());
    CCLOG("Culture per turn: %d", getCulturePerTurn());
    CCLOG("Net gold per turn: %d", calculateNetGoldPerTurn());

    CCLOG("=== End Player Status ===");
}
