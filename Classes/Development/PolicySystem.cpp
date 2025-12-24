#include "PolicySystem.h"

PolicyManager::PolicyManager(CultureTree* cultureTree) : _cultureTree(cultureTree) {
    initGameData();
    if (_cultureTree) {
        _cultureTree->addEventListener(this);
    }
    updateGovernmentSlots();
}

PolicyManager::~PolicyManager() {
    if (_cultureTree) {
        _cultureTree->removeEventListener(this);
    }
}

void PolicyManager::initGameData() {
    // Tier 0
    _govDatabase[GovernmentType::CHIEFDOM] = { u8"酋邦", 1, 1, 0, {} };

    // Tier 1
    _govDatabase[GovernmentType::AUTOCRACY] = { u8"独裁统治", 2, 1, 1,
        {{EffectType::COMBAT_STRENGTH, 1, u8"AllUnits"}} };

    _govDatabase[GovernmentType::OLIGARCHY] = { u8"寡头政体", 2, 1, 1,
        {{EffectType::COMBAT_STRENGTH, 4, u8"Melee"}} };

    _govDatabase[GovernmentType::CLASSICAL_REPUBLIC] = { u8"古典共和", 0, 3, 1,
        {{EffectType::MODIFIER_CULTURE, 10, u8"City"}} };

    // Tier 2
    _govDatabase[GovernmentType::MONARCHY] = { u8"君主制", 3, 2, 1,
        {{EffectType::MODIFIER_PRODUCTION, 20, u8"Walls"}} };

    _govDatabase[GovernmentType::DEMOCRACY] = { u8"民主制", 1, 4, 1,
        {{EffectType::MODIFIER_SCIENCE, 10, u8"City"}} };

    // 默认
    GovernmentConfig defaultConfig = { u8"未知政体", 1, 1, 0, {} };
    _govDatabase[GovernmentType::THEOCRACY] = defaultConfig;
    _govDatabase[GovernmentType::MERCHANT_REPUBLIC] = defaultConfig;
    _govDatabase[GovernmentType::COMMUNISM] = defaultConfig;
    _govDatabase[GovernmentType::FASCISM] = defaultConfig;
    _govDatabase[GovernmentType::CORPORATE_LIBERTY] = defaultConfig;
    _govDatabase[GovernmentType::DIGITAL_DEMOCRACY] = defaultConfig;

    // 政策卡初始化
    _cardDatabase[1001] = { 1001, u8"征兵", u8"单位维护费-1", PolicyType::MILITARY, {{EffectType::MAINTENANCE_DISCOUNT, 1, u8"Unit"}} };
    _cardDatabase[1001].isUnlocked = true;
    _cardDatabase[1002] = { 1002, u8"军团", u8"所有单位+5战斗力", PolicyType::MILITARY, {{EffectType::COMBAT_STRENGTH, 5, u8"All"}} };
    _cardDatabase[1003] = { 1003, u8"海军传统", u8"海军+100%建造速度", PolicyType::MILITARY, {{EffectType::UNIT_PRODUCTION, 100, u8"Naval"}} };
    _cardDatabase[1004] = { 1004, u8"纪律", u8"+5战斗力对抗蛮族", PolicyType::MILITARY, {{EffectType::COMBAT_STRENGTH, 5, u8"Barbarian"}} };
    _cardDatabase[1005] = { 1005, u8"佣兵制", u8"升级费用-50%", PolicyType::MILITARY, {{EffectType::MAINTENANCE_DISCOUNT, 50, u8"Upgrade"}} };

    _cardDatabase[2001] = { 2001, u8"城市规划", u8"所有城市+1生产力", PolicyType::ECONOMIC, {{EffectType::MODIFIER_PRODUCTION, 5, u8"City"}} };
    _cardDatabase[2001].isUnlocked = true;
    _cardDatabase[2002] = { 2002, u8"贸易联盟", u8"+10% 金币", PolicyType::ECONOMIC, {{EffectType::MODIFIER_GOLD, 10, u8"City"}} };
    _cardDatabase[2003] = { 2003, u8"市场经济", u8"+20% 金币", PolicyType::ECONOMIC, {{EffectType::MODIFIER_GOLD, 20, u8"City"}} };
    _cardDatabase[2004] = { 2004, u8"工业革命", u8"+15% 生产力", PolicyType::ECONOMIC, {{EffectType::MODIFIER_PRODUCTION, 15, u8"City"}} };

    _cardDatabase[3001] = { 3001, u8"外交联盟", u8"+5% 科技 (原外交卡)", PolicyType::ECONOMIC, {{EffectType::MODIFIER_SCIENCE, 5, u8"City"}} };
    _cardDatabase[3002] = { 3002, u8"启蒙思想", u8"+100% 学院产出", PolicyType::ECONOMIC, {{EffectType::MODIFIER_SCIENCE, 100, u8"Campus"}} };
    _cardDatabase[3003] = { 3003, u8"国际合作", u8"+50% 贸易路线收益", PolicyType::ECONOMIC, {{EffectType::MODIFIER_GOLD, 50, u8"Trade"}} };

    _cardDatabase[4001] = { 4001, u8"启示录", u8"+2 大预言家点数", PolicyType::WILDCARD, {{EffectType::MODIFIER_CULTURE, 5, u8"City"}} };
    _cardDatabase[4001].isUnlocked = true;
    _cardDatabase[4002] = { 4002, u8"哲学", u8"+100% 剧院产出", PolicyType::ECONOMIC, {{EffectType::MODIFIER_CULTURE, 100, u8"Theater"}} };
    _cardDatabase[4003] = { 4003, u8"黄金时代", u8"所有产出+10%", PolicyType::WILDCARD, {{EffectType::MODIFIER_SCIENCE, 10, u8"City"}} };
    _cardDatabase[4004] = { 4004, u8"科学革命", u8"+100% 科技建筑产出", PolicyType::WILDCARD, {{EffectType::MODIFIER_SCIENCE, 100, u8"SciBuilding"}} };
}

void PolicyManager::updateGovernmentSlots() {
    if (!_cultureTree) return;
    GovernmentType currentGov = _cultureTree->getCurrentGovernment();

    if (_govDatabase.find(currentGov) == _govDatabase.end()) {
        currentGov = GovernmentType::CHIEFDOM;
    }

    const auto& config = _govDatabase[currentGov];

    for (auto& pair : _cardDatabase) pair.second.isActive = false;

    _equippedMilitary.assign(config.militarySlots, -1);
    _equippedEconomic.assign(config.economicSlots, -1);
    _equippedWildcard.assign(config.wildcardSlots, -1);
}

bool PolicyManager::equipPolicy(int cardId, PolicyType slotType, int slotIndex) {
    auto it = _cardDatabase.find(cardId);
    if (it == _cardDatabase.end()) return false;
    auto& card = it->second;

    if (!card.isUnlocked) return false;
    if (card.isActive) return false;

    if (slotType != PolicyType::WILDCARD && card.type != slotType) {
        return false;
    }

    auto& slots = getSlotArray(slotType);
    if (slotIndex < 0 || slotIndex >= slots.size()) return false;

    if (slots[slotIndex] != -1) {
        unequipPolicy(slots[slotIndex]);
    }

    slots[slotIndex] = cardId;
    card.isActive = true;
    return true;
}

bool PolicyManager::unequipPolicy(int cardId) {
    auto it = _cardDatabase.find(cardId);
    if (it == _cardDatabase.end()) return false;

    if (it->second.isActive) {
        removeCardFromSlots(cardId);
        it->second.isActive = false;
        return true;
    }
    return false;
}

void PolicyManager::removeCardFromSlots(int cardId) {
    auto remove = [cardId](std::vector<int>& slots) {
        for (size_t i = 0; i < slots.size(); ++i) {
            if (slots[i] == cardId) slots[i] = -1;
        }
        };
    remove(_equippedMilitary);
    remove(_equippedEconomic);
    remove(_equippedWildcard);
}

float PolicyManager::getYieldModifier(EffectType type) const {
    float modifier = 0.0f;

    if (_cultureTree) {
        GovernmentType gov = _cultureTree->getCurrentGovernment();
        auto it = _govDatabase.find(gov);
        if (it != _govDatabase.end()) {
            for (const auto& eff : it->second.inherentBonuses) {
                if (eff.type == type) modifier += eff.value;
            }
        }
    }

    auto sumCards = [&](const std::vector<int>& slots) {
        for (int id : slots) {
            if (id != -1) {
                auto itCard = _cardDatabase.find(id);
                if (itCard != _cardDatabase.end()) {
                    for (const auto& eff : itCard->second.effects) {
                        if (eff.type == type) modifier += eff.value;
                    }
                }
            }
        }
        };
    sumCards(_equippedMilitary);
    sumCards(_equippedEconomic);
    sumCards(_equippedWildcard);

    return modifier;
}

int PolicyManager::getCombatBonus(const std::string& targetUnit) const {
    int bonus = 0;
    auto sumCombat = [&](const std::vector<int>& slots) {
        for (int id : slots) {
            if (id != -1) {
                auto itCard = _cardDatabase.find(id);
                if (itCard != _cardDatabase.end()) {
                    for (const auto& eff : itCard->second.effects) {
                        if (eff.type == EffectType::COMBAT_STRENGTH) bonus += (int)eff.value;
                    }
                }
            }
        }
        };
    sumCombat(_equippedMilitary);
    sumCombat(_equippedEconomic);
    sumCombat(_equippedWildcard);
    return bonus;
}

const GovernmentConfig& PolicyManager::getCurrentGovConfig() const {
    if (_cultureTree) {
        GovernmentType gov = _cultureTree->getCurrentGovernment();
        auto it = _govDatabase.find(gov);
        if (it != _govDatabase.end()) {
            return it->second;
        }
    }
    return _govDatabase.at(GovernmentType::CHIEFDOM);
}

std::vector<PolicyCard> PolicyManager::getUnlockedCards(PolicyType type) const {
    std::vector<PolicyCard> result;
    for (const auto& pair : _cardDatabase) {
        const auto& card = pair.second;
        if (card.isUnlocked) {
            if (type == PolicyType::WILDCARD) {
                if (card.type == PolicyType::WILDCARD) result.push_back(card);
            }
            else {
                if (card.type == type) result.push_back(card);
            }
        }
    }
    return result;
}

const PolicyCard* PolicyManager::getPolicyCard(int cardId) const {
    auto it = _cardDatabase.find(cardId);
    if (it != _cardDatabase.end()) return &it->second;
    return nullptr;
}

std::vector<EquippedPolicyInfo> PolicyManager::getEquippedPolicies() const {
    std::vector<EquippedPolicyInfo> result;
    auto add = [&](const std::vector<int>& slots, PolicyType type) {
        for (size_t i = 0; i < slots.size(); ++i) {
            if (slots[i] != -1) {
                result.push_back({ slots[i], type, (int)i });
            }
        }
        };
    add(_equippedMilitary, PolicyType::MILITARY);
    add(_equippedEconomic, PolicyType::ECONOMIC);
    add(_equippedWildcard, PolicyType::WILDCARD);
    return result;
}

void PolicyManager::onCultureUnlocked(int cultureId, const std::string& cultureName, const std::string& effect) {
    std::vector<int> unlockedIds = _cultureTree->getPoliciesUnlockedByCulture(cultureId);
    for (int id : unlockedIds) {
        if (_cardDatabase.find(id) != _cardDatabase.end()) {
            _cardDatabase[id].isUnlocked = true;
            std::cout << u8"Unlocked Policy: " << _cardDatabase[id].name << std::endl;
        }
    }
}

std::vector<int>& PolicyManager::getSlotArray(PolicyType type) {
    switch (type) {
    case PolicyType::MILITARY: return _equippedMilitary;
    case PolicyType::ECONOMIC: return _equippedEconomic;
    case PolicyType::WILDCARD: return _equippedWildcard;
    }
    return _equippedWildcard;
}

const std::vector<int>& PolicyManager::getSlotArray(PolicyType type) const {
    switch (type) {
    case PolicyType::MILITARY: return _equippedMilitary;
    case PolicyType::ECONOMIC: return _equippedEconomic;
    case PolicyType::WILDCARD: return _equippedWildcard;
    }
    return _equippedWildcard;
}

const GovernmentConfig& PolicyManager::getGovConfig(GovernmentType type) const {
    auto it = _govDatabase.find(type);
    if (it != _govDatabase.end()) {
        return it->second;
    }
    // 如果找不到，返回默认酋邦配置
    return _govDatabase.at(GovernmentType::CHIEFDOM);
}