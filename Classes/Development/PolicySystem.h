#ifndef POLICY_SYSTEM_H
#define POLICY_SYSTEM_H

#include "CultureSystem.h" 
#include <vector>
#include <map>
#include <string>
#include <iostream>

// ==========================================
// 1. 基础定义
// ==========================================

enum class PolicyType {
    MILITARY,
    ECONOMIC,
    WILDCARD
};

enum class EffectType {
    MODIFIER_PRODUCTION,
    MODIFIER_GOLD,
    MODIFIER_SCIENCE,
    MODIFIER_CULTURE,
    COMBAT_STRENGTH,
    UNIT_PRODUCTION,
    MAINTENANCE_DISCOUNT
};

struct PolicyEffect {
    EffectType type;
    float value;
    std::string target;
};

struct PolicyCard {
    int id;
    std::string name;
    std::string desc;
    PolicyType type;
    std::vector<PolicyEffect> effects;
    bool isUnlocked = false;
    bool isActive = false;
};

struct GovernmentConfig {
    std::string name;
    int militarySlots;
    int economicSlots;
    int wildcardSlots;
    std::vector<PolicyEffect> inherentBonuses;
};

struct EquippedPolicyInfo {
    int cardId;
    PolicyType slotType;
    int slotIndex;
};

// ==========================================
// 2. 政策管理器
// ==========================================

class PolicyManager : public CultureEventListener {
private:
    CultureTree* _cultureTree;

    std::map<int, PolicyCard> _cardDatabase;
    std::map<GovernmentType, GovernmentConfig> _govDatabase;

    std::vector<int> _equippedMilitary;
    std::vector<int> _equippedEconomic;
    std::vector<int> _equippedWildcard;

public:
    PolicyManager(CultureTree* cultureTree);
    ~PolicyManager();

    void initGameData();

    // --- 核心操作 (非 const) ---
    bool equipPolicy(int cardId, PolicyType slotType, int slotIndex);
    bool unequipPolicy(int cardId);
    void updateGovernmentSlots();

    // --- 数据查询 (必须是 const) ---
    // 修改点：添加 const
    float getYieldModifier(EffectType type) const;
    // 修改点：添加 const
    int getCombatBonus(const std::string& targetUnit) const;

    // --- UI 数据查询接口 (const) ---
    const GovernmentConfig& getCurrentGovConfig() const;
    std::vector<PolicyCard> getUnlockedCards(PolicyType type) const;
    const PolicyCard* getPolicyCard(int cardId) const;
    std::vector<EquippedPolicyInfo> getEquippedPolicies() const;
    bool isPolicyCompatible(int cardId) const { return true; }
    const GovernmentConfig& getGovConfig(GovernmentType type) const;

    // --- CultureEventListener 实现 ---
    virtual void onCultureUnlocked(int cultureId, const std::string& cultureName, const std::string& effect) override;
    virtual void onCultureProgress(int cultureId, int currentProgress, int totalCost) override {}
    virtual void onInspirationTriggered(int cultureId, const std::string& cultureName) override {}

private:
    std::vector<int>& getSlotArray(PolicyType type);
    // 增加 const 版本的辅助函数
    const std::vector<int>& getSlotArray(PolicyType type) const;
    void removeCardFromSlots(int cardId);
};

#endif // POLICY_SYSTEM_H