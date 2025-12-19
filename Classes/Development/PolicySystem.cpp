#include "PolicySystem.h"
#include "CultureSystem.h"
#include <sstream>
#include <iostream>
#include <cocos2d.h>

PolicyManager::PolicyManager()
    : _militarySlotCount(0), _economicSlotCount(0),
    _diplomaticSlotCount(0), _wildcardSlotCount(0) {

    // 初始化回调函数为默认值
    _getCurrentGovernmentFunc = []() -> GovernmentType {
        return GovernmentType::CHIEFDOM; // 默认值
        };

    _getPoliciesByCultureFunc = [](int cultureId) -> std::vector<int> {
        return {}; // 默认返回空列表
        };

    _onPolicyUnlockedCallback = [](int policyId) {
        // 默认不做任何事
        };

    initializePolicies();
}

PolicyManager::~PolicyManager() {
    _listeners.clear();
}

void PolicyManager::initializePolicies() {
    // 清空现有数据
    _allPolicies.clear();
    _unlockedPolicyIds.clear();
    _equippedPolicies.clear();
    _policyCombos.clear();

    // ==================== 军事政策卡 ====================
    _allPolicies.emplace(1001, PolicyCard(1001, u8"征兵", PolicyType::MILITARY,
        u8"训练近战和远程陆地单位时+50%生产力", PolicyRarity::COMMON, 0, -1, 0));
    _allPolicies[1001].addEffect(PolicyEffectType::UNIT_PRODUCTION, 50, u8"近战和远程陆地单位");
    // 所有政体都兼容
    // _allPolicies[1001].compatibleGovernments 保持为空，表示全部兼容

    _allPolicies.emplace(1002, PolicyCard(1002, u8"军团", PolicyType::MILITARY,
        u8"所有陆地单位+5战斗力", PolicyRarity::RARE, 0, 101, 25));
    _allPolicies[1002].addEffect(PolicyEffectType::UNIT_STRENGTH, 5, u8"所有陆地单位");
    // 添加兼容政体
    _allPolicies[1002].addCompatibleGovernment(GovernmentType::CHIEFDOM);
    _allPolicies[1002].addCompatibleGovernment(GovernmentType::AUTOCRACY);
    _allPolicies[1002].addCompatibleGovernment(GovernmentType::MONARCHY);
    _allPolicies[1002].addCompatibleGovernment(GovernmentType::FASCISM);

    _allPolicies.emplace(1003, PolicyCard(1003, u8"海军传统", PolicyType::MILITARY,
        u8"训练海军单位时+100%生产力", PolicyRarity::COMMON, 0, -1, 0));
    _allPolicies[1003].addEffect(PolicyEffectType::UNIT_PRODUCTION, 100, u8"海军单位");
    // 民主政体下特别有效
    _allPolicies[1003].addCompatibleGovernment(GovernmentType::CLASSICAL_REPUBLIC);
    _allPolicies[1003].addCompatibleGovernment(GovernmentType::MERCHANT_REPUBLIC);
    _allPolicies[1003].addCompatibleGovernment(GovernmentType::DEMOCRACY);
    _allPolicies[1003].addCompatibleGovernment(GovernmentType::CORPORATE_LIBERTY);
    _allPolicies[1003].addCompatibleGovernment(GovernmentType::DIGITAL_DEMOCRACY);

    // 古典时代军事政策
    _allPolicies.emplace(1004, PolicyCard(1004, u8"纪律", PolicyType::MILITARY,
        u8"所有单位+1移动力，单位维护费-25%", PolicyRarity::EPIC, 1, 103, 50));
    _allPolicies[1004].addEffect(PolicyEffectType::UNIT_MOVEMENT, 1, u8"所有单位");
    _allPolicies[1004].addEffect(PolicyEffectType::MAINTENANCE_REDUCTION, 25, u8"所有单位");
    // 专制和法西斯政体特别适用
    _allPolicies[1004].addCompatibleGovernment(GovernmentType::AUTOCRACY);
    _allPolicies[1004].addCompatibleGovernment(GovernmentType::FASCISM);

    _allPolicies.emplace(1005, PolicyCard(1005, u8"佣兵制", PolicyType::MILITARY,
        u8"所有单位战斗力+3，战斗经验+25%", PolicyRarity::RARE, 1, 103, 40));
    _allPolicies[1005].addEffect(PolicyEffectType::UNIT_STRENGTH, 3, u8"所有单位");
    _allPolicies[1005].addEffect(PolicyEffectType::COMBAT_EXPERIENCE, 25, u8"所有单位");
    // 寡头和商人共和国适用
    _allPolicies[1005].addCompatibleGovernment(GovernmentType::OLIGARCHY);
    _allPolicies[1005].addCompatibleGovernment(GovernmentType::MERCHANT_REPUBLIC);

    // ==================== 经济政策卡 ====================
    _allPolicies.emplace(2001, PolicyCard(2001, u8"城市规划", PolicyType::ECONOMIC,
        u8"城市中心建筑和区域+30%生产力", PolicyRarity::COMMON, 0, 101, 0));
    _allPolicies[2001].addEffect(PolicyEffectType::PRODUCTION_BONUS, 30, u8"城市中心和区域");
    // 所有政体都兼容

    _allPolicies.emplace(2002, PolicyCard(2002, u8"贸易联盟", PolicyType::ECONOMIC,
        u8"所有城市金币+10%", PolicyRarity::COMMON, 0, -1, 0));
    _allPolicies[2002].addEffect(PolicyEffectType::GOLD_BONUS, 10, u8"所有城市");
    // 民主和商人共和国特别适用
    _allPolicies[2002].addCompatibleGovernment(GovernmentType::MERCHANT_REPUBLIC);
    _allPolicies[2002].addCompatibleGovernment(GovernmentType::DEMOCRACY);
    _allPolicies[2002].addCompatibleGovernment(GovernmentType::CORPORATE_LIBERTY);

    _allPolicies.emplace(2003, PolicyCard(2003, u8"市场经济", PolicyType::ECONOMIC,
        u8"商业中心建筑+50%生产力，商路金币+2", PolicyRarity::RARE, 1, 102, 50));
    _allPolicies[2003].addEffect(PolicyEffectType::BUILDING_PRODUCTION, 50, u8"商业中心建筑");
    _allPolicies[2003].addEffect(PolicyEffectType::GOLD_BONUS, 20, u8"商路", PolicyActivationCondition::TRADE_ROUTE);
    // 资本主义相关政体
    _allPolicies[2003].addCompatibleGovernment(GovernmentType::MERCHANT_REPUBLIC);
    _allPolicies[2003].addCompatibleGovernment(GovernmentType::DEMOCRACY);
    _allPolicies[2003].addCompatibleGovernment(GovernmentType::CORPORATE_LIBERTY);

    _allPolicies.emplace(2004, PolicyCard(2004, u8"工业革命", PolicyType::ECONOMIC,
        u8"工厂+100%生产力，工业区+50%生产力", PolicyRarity::EPIC, 4, 110, 100));
    _allPolicies[2004].addEffect(PolicyEffectType::BUILDING_PRODUCTION, 100, u8"工厂");
    _allPolicies[2004].addEffect(PolicyEffectType::DISTRICT_PRODUCTION, 50, u8"工业区");
    _allPolicies[2004].addCompatibleGovernment(GovernmentType::COMMUNISM);
    _allPolicies[2004].addCompatibleGovernment(GovernmentType::CORPORATE_LIBERTY);

    // ==================== 外交政策卡 ==================== 
    _allPolicies.emplace(3002, PolicyCard(3002, u8"启蒙思想", PolicyType::DIPLOMATIC,
        u8"所有城市科研+10%，伟人点数+1", PolicyRarity::RARE, 3, 107, 60));
    _allPolicies[3002].addEffect(PolicyEffectType::SCIENCE_BONUS, 10, u8"所有城市");
    _allPolicies[3002].addEffect(PolicyEffectType::GREAT_PERSON_POINTS, 1, u8"所有城市");
    _allPolicies[3002].addCompatibleGovernment(GovernmentType::CLASSICAL_REPUBLIC);
    _allPolicies[3002].addCompatibleGovernment(GovernmentType::DEMOCRACY);
    _allPolicies[3002].addCompatibleGovernment(GovernmentType::DIGITAL_DEMOCRACY);

    _allPolicies.emplace(3003, PolicyCard(3003, u8"国际合作", PolicyType::DIPLOMATIC,
        u8"外交关系+50%加成，贸易路线收益+50%", PolicyRarity::RARE, 5, 113, 80));
    _allPolicies[3003].addEffect(PolicyEffectType::TRADE_ROUTE_YIELD, 50, u8"所有贸易路线");
    _allPolicies[3003].addCompatibleGovernment(GovernmentType::DEMOCRACY);
    _allPolicies[3003].addCompatibleGovernment(GovernmentType::DIGITAL_DEMOCRACY);

    // ==================== 通用政策卡 ====================
    _allPolicies.emplace(4003, PolicyCard(4003, u8"黄金时代", PolicyType::WILDCARD,
        u8"黄金时代时所有产出+10%", PolicyRarity::EPIC, 1, 107, 80));
    _allPolicies[4003].addEffect(PolicyEffectType::PRODUCTION_BONUS, 10, u8"所有城市", PolicyActivationCondition::GOLDEN_AGE);
    _allPolicies[4003].addEffect(PolicyEffectType::GOLD_BONUS, 10, u8"所有城市", PolicyActivationCondition::GOLDEN_AGE);
    _allPolicies[4003].addEffect(PolicyEffectType::SCIENCE_BONUS, 10, u8"所有城市", PolicyActivationCondition::GOLDEN_AGE);
    _allPolicies[4003].addEffect(PolicyEffectType::CULTURE_BONUS, 10, u8"所有城市", PolicyActivationCondition::GOLDEN_AGE);

    _allPolicies.emplace(4004, PolicyCard(4004, u8"科学革命", PolicyType::WILDCARD,
        u8"大学+100%科研，科研建筑加速建造", PolicyRarity::EPIC, 5, 111, 120));
    _allPolicies[4004].addEffect(PolicyEffectType::SCIENCE_BONUS, 100, u8"大学");
    _allPolicies[4004].addEffect(PolicyEffectType::BUILDING_PRODUCTION, 50, u8"科研建筑");
    // 所有政体都兼容（通用政策卡通常没有政体限制）

    // ==================== 政策卡组合 ====================

    // 军事组合：军团+纪律
    addPolicyCombo(u8"常备军", { 1002, 1004 });

    // 经济组合：城市规划+市场经济
    addPolicyCombo(u8"商业帝国", { 2001, 2003 });

    // 初始解锁一些基础政策卡
    unlockPolicy(1001);  // 征兵
    unlockPolicy(1003);  // 海军传统
    unlockPolicy(2001);  // 城市规划
    unlockPolicy(2002);  // 贸易联盟
    unlockPolicy(3001);  // 外交联盟
    unlockPolicy(4001);  // 启示录
    unlockPolicy(4002);  // 哲学
}

void PolicyManager::setPolicySlots(int military, int economic, int diplomatic, int wildcard) {
    _militarySlotCount = military;
    _economicSlotCount = economic;
    _diplomaticSlotCount = diplomatic;
    _wildcardSlotCount = wildcard;

    // 检查并卸下超出槽位数量的政策卡
    auto it = _equippedPolicies.begin();
    while (it != _equippedPolicies.end()) {
        bool shouldRemove = false;

        switch (it->slotType) {
            case PolicyType::MILITARY:
                shouldRemove = (it->slotIndex >= _militarySlotCount);
                break;
            case PolicyType::ECONOMIC:
                shouldRemove = (it->slotIndex >= _economicSlotCount);
                break;
            case PolicyType::DIPLOMATIC:
                shouldRemove = (it->slotIndex >= _diplomaticSlotCount);
                break;
            case PolicyType::WILDCARD:
                shouldRemove = (it->slotIndex >= _wildcardSlotCount);
                break;
        }

        if (shouldRemove) {
            // 通知卸下
            notifyPolicyUnequipped(it->cardId, it->slotType, it->slotIndex);

            // 更新政策卡状态
            auto cardIt = _allPolicies.find(it->cardId);
            if (cardIt != _allPolicies.end()) {
                cardIt->second.isActive = false;
            }

            // 从装备列表中移除
            it = _equippedPolicies.erase(it);
        }
        else {
            ++it;
        }
    }
}

void PolicyManager::getPolicySlots(int& military, int& economic, int& diplomatic, int& wildcard) const {
    military = _militarySlotCount;
    economic = _economicSlotCount;
    diplomatic = _diplomaticSlotCount;
    wildcard = _wildcardSlotCount;
}

void PolicyManager::unlockPolicyByCulture(int cultureId) {
    // 遍历所有政策卡，解锁与该市政关联的政策卡
    for (auto& pair : _allPolicies) {
        if (pair.second.unlockCultureId == cultureId && !pair.second.isUnlocked) {
            unlockPolicy(pair.first);
        }
    }
}

bool PolicyManager::unlockPolicy(int policyId) {
    auto it = _allPolicies.find(policyId);
    if (it == _allPolicies.end()) {
        return false;
    }

    if (it->second.isUnlocked) {
        return false;  // 已经解锁
    }

    it->second.isUnlocked = true;
    _unlockedPolicyIds.push_back(policyId);

    // 通知监听器
    notifyPolicyUnlocked(policyId, it->second.name);

    return true;
}

bool PolicyManager::equipPolicy(int policyId, PolicyType slotType, int slotIndex) {
    // 检查政策卡是否存在且已解锁
    auto cardIt = _allPolicies.find(policyId);
    if (cardIt == _allPolicies.end() || !cardIt->second.isUnlocked) {
        return false;
    }

    // 检查政策卡是否已经装备
    if (cardIt->second.isActive) {
        return false;  // 政策卡已经装备在其他槽位
    }

    // 检查槽位类型是否匹配
    if (!canEquipToSlot(cardIt->second, slotType)) {
        return false;
    }

    // 检查槽位索引是否有效
    if (!isSlotAvailable(slotType, slotIndex)) {
        return false;
    }

    // 检查该槽位是否已被占用
    const EquippedPolicy* existing = findPolicyInSlot(slotType, slotIndex);
    if (existing != nullptr) {
        return false;  // 槽位已被占用
    }

    if (!isPolicyCompatible(policyId)) {
        return false;
    }

    // 装备政策卡
    _equippedPolicies.emplace_back(policyId, slotType, slotIndex);
    cardIt->second.isActive = true;

    // 通知监听器
    notifyPolicyEquipped(policyId, slotType, slotIndex);

    // 检查政策卡组合
    checkPolicyCombos();

    return true;
}

bool PolicyManager::unequipPolicy(int policyId) {
    auto it = findEquippedPolicy(policyId);
    if (it == _equippedPolicies.end()) {
        return false;
    }

    // 保存信息用于通知
    PolicyType slotType = it->slotType;
    int slotIndex = it->slotIndex;

    // 更新政策卡状态
    auto cardIt = _allPolicies.find(policyId);
    if (cardIt != _allPolicies.end()) {
        cardIt->second.isActive = false;
    }

    // 从装备列表中移除
    _equippedPolicies.erase(it);

    // 通知监听器
    notifyPolicyUnequipped(policyId, slotType, slotIndex);

    // 重新检查政策卡组合
    checkPolicyCombos();

    return true;
}

bool PolicyManager::isSlotAvailable(PolicyType slotType, int slotIndex) const {
    int maxSlots = 0;

    switch (slotType) {
        case PolicyType::MILITARY:
            maxSlots = _militarySlotCount;
            break;
        case PolicyType::ECONOMIC:
            maxSlots = _economicSlotCount;
            break;
        case PolicyType::DIPLOMATIC:
            maxSlots = _diplomaticSlotCount;
            break;
        case PolicyType::WILDCARD:
            maxSlots = _wildcardSlotCount;
            break;
    }

    return slotIndex >= 0 && slotIndex < maxSlots;
}

std::vector<PolicyCard> PolicyManager::getAvailablePolicies(PolicyType type) const {
    std::vector<PolicyCard> result;

    CCLOG("getAvailablePolicies for type %d, total unlocked IDs: %zu",
        static_cast<int>(type), _unlockedPolicyIds.size());

    for (int policyId : _unlockedPolicyIds) {
        auto it = _allPolicies.find(policyId);
        if (it == _allPolicies.end()) {
            CCLOG("  WARNING: Policy %d not found in _allPolicies", policyId);
            continue;
        }

        const PolicyCard& card = it->second;

        // 检查类型匹配
        bool typeMatches = false;
        if (type == PolicyType::WILDCARD) {
            // 在通用标签页：显示所有政策卡
            typeMatches = true;
        }
        else {
            // 在特定标签页：显示匹配类型的政策卡
            typeMatches = (card.type == type);
        }

        if (typeMatches) {
            result.push_back(card);
            CCLOG("  Added policy %d: %s (type=%d, active=%s)",
                policyId, card.name.c_str(),
                static_cast<int>(card.type),
                card.isActive ? "YES" : "NO");
        }
    }

    CCLOG("Returning %zu policies for type %d", result.size(), static_cast<int>(type));
    return result;
}

const PolicyCard* PolicyManager::getPolicyCard(int policyId) const {
    auto it = _allPolicies.find(policyId);
    if (it == _allPolicies.end()) {
        return nullptr;
    }
    return &(it->second);
}

bool PolicyManager::isPolicyEquipped(int policyId) const {
    return findEquippedPolicy(policyId) != _equippedPolicies.end();
}

bool PolicyManager::isPolicyUnlocked(int policyId) const {
    auto it = _allPolicies.find(policyId);
    if (it == _allPolicies.end()) {
        return false;
    }
    return it->second.isUnlocked;
}

std::vector<PolicyEffect> PolicyManager::getActivePolicyEffects() const {
    std::vector<PolicyEffect> allEffects;

    // 收集所有已装备政策卡的效果
    for (const auto& equipped : _equippedPolicies) {
        auto cardIt = _allPolicies.find(equipped.cardId);
        if (cardIt != _allPolicies.end()) {
            // 添加政策卡的所有效果
            for (const auto& effect : cardIt->second.effects) {
                allEffects.push_back(effect);
            }
        }
    }

    // 这里可以添加政策卡组合的额外效果

    return allEffects;
}

void PolicyManager::checkPolicyCombos() {
    // 获取当前已装备的政策卡ID列表
    std::vector<int> equippedIds;
    for (const auto& equipped : _equippedPolicies) {
        equippedIds.push_back(equipped.cardId);
    }

    // 排序以便比较
    std::sort(equippedIds.begin(), equippedIds.end());

    // 检查每个组合
    for (const auto& comboPair : _policyCombos) {
        const std::string& comboName = comboPair.first;
        const std::vector<int>& comboPolicyIds = comboPair.second;

        // 复制并排序组合中的政策卡ID
        std::vector<int> sortedComboIds = comboPolicyIds;
        std::sort(sortedComboIds.begin(), sortedComboIds.end());

        // 检查是否包含了组合中的所有政策卡
        bool hasAll = true;
        for (int policyId : sortedComboIds) {
            if (std::find(equippedIds.begin(), equippedIds.end(), policyId) == equippedIds.end()) {
                hasAll = false;
                break;
            }
        }

        if (hasAll) {
            // 触发组合效果
            notifyPolicyComboTriggered(sortedComboIds, comboName);
        }
    }
}

void PolicyManager::addEventListener(PolicyEventListener* listener) {
    if (listener && std::find(_listeners.begin(), _listeners.end(), listener) == _listeners.end()) {
        _listeners.push_back(listener);
    }
}

void PolicyManager::removeEventListener(PolicyEventListener* listener) {
    auto it = std::find(_listeners.begin(), _listeners.end(), listener);
    if (it != _listeners.end()) {
        _listeners.erase(it);
    }
}

void PolicyManager::savePolicyState(std::string& data) const {
    // 简化的保存逻辑
    std::ostringstream oss;

    // 保存已解锁的政策卡ID
    oss << "unlocked:";
    for (size_t i = 0; i < _unlockedPolicyIds.size(); ++i) {
        if (i > 0) oss << ",";
        oss << _unlockedPolicyIds[i];
    }
    oss << ";";

    // 保存已装备的政策卡
    oss << "equipped:";
    for (size_t i = 0; i < _equippedPolicies.size(); ++i) {
        if (i > 0) oss << ",";
        const auto& equipped = _equippedPolicies[i];
        oss << equipped.cardId << ":" << static_cast<int>(equipped.slotType) << ":" << equipped.slotIndex;
    }

    data = oss.str();
}

void PolicyManager::loadPolicyState(const std::string& data) {
    // 清空当前状态
    for (auto& pair : _allPolicies) {
        pair.second.isUnlocked = false;
        pair.second.isActive = false;
    }
    _unlockedPolicyIds.clear();
    _equippedPolicies.clear();

    // 简化的加载逻辑
    std::istringstream iss(data);
    std::string section;

    while (std::getline(iss, section, ';')) {
        size_t colonPos = section.find(':');
        if (colonPos == std::string::npos) continue;

        std::string key = section.substr(0, colonPos);
        std::string value = section.substr(colonPos + 1);

        if (key == "unlocked") {
            // 加载已解锁的政策卡
            std::istringstream idStream(value);
            std::string idStr;
            while (std::getline(idStream, idStr, ',')) {
                int policyId = std::stoi(idStr);
                unlockPolicy(policyId);
            }
        }
        else if (key == "equipped") {
            // 加载已装备的政策卡
            std::istringstream equipStream(value);
            std::string equipStr;
            while (std::getline(equipStream, equipStr, ',')) {
                // 格式：cardId:slotType:slotIndex
                size_t pos1 = equipStr.find(':');
                size_t pos2 = equipStr.find(':', pos1 + 1);

                if (pos1 != std::string::npos && pos2 != std::string::npos) {
                    int cardId = std::stoi(equipStr.substr(0, pos1));
                    int slotType = std::stoi(equipStr.substr(pos1 + 1, pos2 - pos1 - 1));
                    int slotIndex = std::stoi(equipStr.substr(pos2 + 1));

                    equipPolicy(cardId, static_cast<PolicyType>(slotType), slotIndex);
                }
            }
        }
    }
}

// 新增：根据文化ID解锁政策
void PolicyManager::unlockPoliciesForCulture(int cultureId) {
    if (!_getPoliciesByCultureFunc) return;

    auto policyIds = _getPoliciesByCultureFunc(cultureId);
    for (int policyId : policyIds) {
        unlockPolicy(policyId);
        if (_onPolicyUnlockedCallback) {
            _onPolicyUnlockedCallback(policyId);
        }
    }
}

// 新增：更新政策槽位
void PolicyManager::updatePolicySlots(int military, int economic,
    int diplomatic, int wildcard) {
    setPolicySlots(military, economic, diplomatic, wildcard);
}

// 新增：检查政策兼容性
bool PolicyManager::isPolicyCompatible(int policyId) const {
    if (!_getCurrentGovernmentFunc) return true; // 如果没有设置，默认兼容

    auto it = _allPolicies.find(policyId);
    if (it == _allPolicies.end()) return false;

    // 如果政策卡没有指定兼容政体，则默认兼容
    if (it->second.compatibleGovernments.empty()) {
        return true;
    }

    GovernmentType currentGov = _getCurrentGovernmentFunc();
    return std::find(it->second.compatibleGovernments.begin(),
        it->second.compatibleGovernments.end(),
        currentGov) != it->second.compatibleGovernments.end();
}

// ==================== 私有方法 ====================

void PolicyManager::notifyPolicyUnlocked(int policyId, const std::string& policyName) {
    for (auto listener : _listeners) {
        listener->onPolicyUnlocked(policyId, policyName);
    }
}

void PolicyManager::notifyPolicyEquipped(int policyId, PolicyType slotType, int slotIndex) {
    for (auto listener : _listeners) {
        listener->onPolicyEquipped(policyId, slotType, slotIndex);
    }
}

void PolicyManager::notifyPolicyUnequipped(int policyId, PolicyType slotType, int slotIndex) {
    for (auto listener : _listeners) {
        listener->onPolicyUnequipped(policyId, slotType, slotIndex);
    }
}

void PolicyManager::notifyPolicyComboTriggered(const std::vector<int>& policyIds, const std::string& comboName) {
    for (auto listener : _listeners) {
        listener->onPolicyComboTriggered(policyIds, comboName);
    }
}

bool PolicyManager::canEquipToSlot(const PolicyCard& card, PolicyType slotType) const {
    // 通用政策卡可以放入任何类型的槽位
    if (card.type == PolicyType::WILDCARD) {
        return true;
    }

    // 其他类型的政策卡只能放入对应类型的槽位
    return card.type == slotType;
}

std::vector<EquippedPolicy>::iterator PolicyManager::findEquippedPolicy(int policyId) {
    for (auto it = _equippedPolicies.begin(); it != _equippedPolicies.end(); ++it) {
        if (it->cardId == policyId) {
            return it;
        }
    }
    return _equippedPolicies.end();
}

std::vector<EquippedPolicy>::const_iterator PolicyManager::findEquippedPolicy(int policyId) const {
    for (auto it = _equippedPolicies.begin(); it != _equippedPolicies.end(); ++it) {
        if (it->cardId == policyId) {
            return it;
        }
    }
    return _equippedPolicies.end();
}

const EquippedPolicy* PolicyManager::findPolicyInSlot(PolicyType slotType, int slotIndex) const {
    for (const auto& equipped : _equippedPolicies) {
        if (equipped.slotType == slotType && equipped.slotIndex == slotIndex) {
            return &equipped;
        }
    }
    return nullptr;
}

void PolicyManager::addPolicyCombo(const std::string& comboName, const std::vector<int>& policyIds) {
    _policyCombos[comboName] = policyIds;
}

// CultureEventListener接口实现
void PolicyManager::onCultureUnlocked(int cultureId, const std::string& cultureName,
    const std::string& effect) {

    // 解锁该文化对应的政策卡
    unlockPoliciesForCulture(cultureId);

    // 更新政策槽位数量
    if (_getCurrentGovernmentFunc) {
        // 通知监听器文化已解锁（如果需要）
        // 这里不直接通知，因为政策解锁时会通知
    }
}

void PolicyManager::onCultureProgress(int cultureId, int currentProgress, int totalCost) {
    // 不需要处理文化进度
}

void PolicyManager::onInspirationTriggered(int cultureId, const std::string& cultureName) {
    // 不需要处理灵感触发
}