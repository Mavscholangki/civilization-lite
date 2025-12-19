#ifndef POLICY_SYSTEM_H
#define POLICY_SYSTEM_H

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <functional>  // 用于回调函数
#include "CultureSystem.h"

class CultureTree;
enum class GovernmentType;

// 政策卡类型 - 对应4种政策槽位
enum class PolicyType {
    MILITARY,      // 军事政策卡
    ECONOMIC,      // 经济政策卡  
    DIPLOMATIC,    // 外交政策卡
    WILDCARD       // 通用政策卡（可放入任何类型槽位）
};

// 政策卡稀有度（用于视觉区分）
enum class PolicyRarity {
    COMMON,        // 普通（白色/蓝色）
    RARE,          // 稀有（紫色）
    EPIC,          // 史诗（金色）
    LEGENDARY      // 传奇（橙色）
};

// 政策卡激活条件
enum class PolicyActivationCondition {
    ALWAYS,        // 总是激活
    AT_WAR,        // 处于战争状态
    AT_PEACE,      // 处于和平状态
    GOLDEN_AGE,    // 黄金时代
    DARK_AGE,      // 黑暗时代
    TRADE_ROUTE,   // 有贸易路线
    CITY_COUNT,    // 城市数量条件
    POPULATION     // 人口条件
};

// 政策卡效果类型
enum class PolicyEffectType {
    PRODUCTION_BONUS,      // 生产力加成
    GOLD_BONUS,            // 金币加成
    SCIENCE_BONUS,         // 科研加成
    CULTURE_BONUS,         // 文化加成
    FAITH_BONUS,           // 信仰加成
    UNIT_PRODUCTION,       // 单位生产加速
    BUILDING_PRODUCTION,   // 建筑生产加速
    DISTRICT_PRODUCTION,   // 区域生产加速
    UNIT_STRENGTH,         // 单位战斗力加成
    UNIT_MOVEMENT,         // 单位移动力加成
    TRADE_ROUTE_CAPACITY,  // 贸易路线容量
    TRADE_ROUTE_YIELD,     // 贸易路线收益加成
    HAPPINESS_BONUS,       // 满意度加成
    GRIEVANCE_REDUCTION,   // 不满度减少
    SPY_CAPACITY,          // 间谍容量
    ENVOY_BONUS,           // 使者加成
    WONDER_PRODUCTION,     // 奇观生产加速
    GREAT_PERSON_POINTS,   // 伟人点数加成
    COMBAT_EXPERIENCE,     // 战斗经验加成
    MAINTENANCE_REDUCTION  // 维护费减少
};

// 政策卡效果结构
struct PolicyEffect {
    PolicyEffectType type;
    int value;  // 效果数值（百分比或固定值）
    std::string target;  // 作用目标（如"所有单位"、"城市中心"等）
    PolicyActivationCondition condition;  // 激活条件
    int conditionValue;  // 条件数值（如需要多少城市）

    PolicyEffect(PolicyEffectType t, int v, const std::string& tg = "",
        PolicyActivationCondition cond = PolicyActivationCondition::ALWAYS,
        int condVal = 0)
        : type(t), value(v), target(tg), condition(cond), conditionValue(condVal) {
    }
};

// 政策卡结构
struct PolicyCard {
    int id;                     // 唯一ID
    std::string name;          // 名称
    std::string description;   // 描述文本
    PolicyType type;           // 政策卡类型
    PolicyRarity rarity;       // 稀有度
    int era;                   // 解锁时代（0=远古，1=古典，2=中世纪...）
    int unlockCultureId;       // 通过哪个市政解锁（-1表示初始解锁）
    int cost;                  // 解锁所需文化值（如果是通过文化解锁）
    bool isUnlocked;           // 是否已解锁
    bool isActive;             // 是否已激活（装备在槽位中）

    // 效果列表（一个政策卡可以有多个效果）
    std::vector<PolicyEffect> effects;

    // 新增：兼容的政体类型列表（如果为空，表示所有政体都兼容）
    std::vector<GovernmentType> compatibleGovernments;

    // 构造函数
    PolicyCard() = default;
    PolicyCard(int id, const std::string& name, PolicyType type,
        const std::string& desc, PolicyRarity rarity = PolicyRarity::COMMON,
        int era = 0, int unlockCultureId = -1, int cost = 0)
        : id(id), name(name), description(desc), type(type),
        rarity(rarity), era(era), unlockCultureId(unlockCultureId),
        cost(cost), isUnlocked(false), isActive(false) {
    }

    // 添加效果
    void addEffect(PolicyEffectType effectType, int value,
        const std::string& target = "",
        PolicyActivationCondition condition = PolicyActivationCondition::ALWAYS,
        int conditionValue = 0) {
        effects.emplace_back(effectType, value, target, condition, conditionValue);
    }

    // 添加兼容政体
    void addCompatibleGovernment(GovernmentType government) {
        if (std::find(compatibleGovernments.begin(), compatibleGovernments.end(),
            government) == compatibleGovernments.end()) {
            compatibleGovernments.push_back(government);
        }
    }

    // 检查是否与指定政体兼容
    bool isCompatibleWithGovernment(GovernmentType government) const {
        if (compatibleGovernments.empty()) {
            return true;  // 如果没有指定兼容政体，默认全部兼容
        }
        return std::find(compatibleGovernments.begin(),
            compatibleGovernments.end(),
            government) != compatibleGovernments.end();
    }

    // 获取效果描述文本
    std::string getEffectDescription() const {
        std::string result;
        for (const auto& effect : effects) {
            std::string effectText;

            // 根据效果类型生成描述
            switch (effect.type) {
                case PolicyEffectType::PRODUCTION_BONUS:
                    effectText = "+" + std::to_string(effect.value) + "% 生产力";
                    break;
                case PolicyEffectType::GOLD_BONUS:
                    effectText = "+" + std::to_string(effect.value) + "% 金币";
                    break;
                case PolicyEffectType::SCIENCE_BONUS:
                    effectText = "+" + std::to_string(effect.value) + "% 科研";
                    break;
                case PolicyEffectType::CULTURE_BONUS:
                    effectText = "+" + std::to_string(effect.value) + "% 文化";
                    break;
                case PolicyEffectType::UNIT_PRODUCTION:
                    effectText = "+" + std::to_string(effect.value) + "% 单位生产速度";
                    break;
                case PolicyEffectType::BUILDING_PRODUCTION:
                    effectText = "+" + std::to_string(effect.value) + "% 建筑生产速度";
                    break;
                case PolicyEffectType::UNIT_STRENGTH:
                    effectText = "+" + std::to_string(effect.value) + " 单位战斗力";
                    break;
                case PolicyEffectType::UNIT_MOVEMENT:
                    effectText = "+" + std::to_string(effect.value) + " 单位移动力";
                    break;
                case PolicyEffectType::COMBAT_EXPERIENCE:
                    effectText = "+" + std::to_string(effect.value) + "% 战斗经验";
                    break;
                case PolicyEffectType::MAINTENANCE_REDUCTION:
                    effectText = "-" + std::to_string(effect.value) + "% 维护费";
                    break;
                    // 可以添加更多效果类型...
                default:
                    effectText = "效果（值：" + std::to_string(effect.value) + "）";
                    break;
            }

            // 添加作用目标
            if (!effect.target.empty()) {
                effectText += "（" + effect.target + "）";
            }

            // 添加激活条件
            if (effect.condition != PolicyActivationCondition::ALWAYS) {
                std::string condText;
                switch (effect.condition) {
                    case PolicyActivationCondition::AT_WAR:
                        condText = "战争时";
                        break;
                    case PolicyActivationCondition::AT_PEACE:
                        condText = "和平时";
                        break;
                    case PolicyActivationCondition::GOLDEN_AGE:
                        condText = "黄金时代";
                        break;
                    case PolicyActivationCondition::TRADE_ROUTE:
                        condText = "有贸易路线时";
                        break;
                    case PolicyActivationCondition::CITY_COUNT:
                        condText = "城市数≥" + std::to_string(effect.conditionValue);
                        break;
                    default:
                        condText = "特殊条件";
                        break;
                }
                effectText += " [" + condText + "]";
            }

            result += effectText + "\n";
        }

        // 移除最后一个换行符
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }

        return result;
    }
};

// 已装备的政策卡信息
struct EquippedPolicy {
    int cardId;           // 政策卡ID
    PolicyType slotType;  // 槽位类型
    int slotIndex;        // 在哪个槽位（从0开始）

    EquippedPolicy() = default;
    EquippedPolicy(int id, PolicyType type, int index)
        : cardId(id), slotType(type), slotIndex(index) {
    }
};

// 政策卡事件监听器接口
class PolicyEventListener {
public:
    virtual ~PolicyEventListener() {}

    // 政策卡被解锁
    virtual void onPolicyUnlocked(int policyId, const std::string& policyName) = 0;

    // 政策卡被装备
    virtual void onPolicyEquipped(int policyId, PolicyType slotType, int slotIndex) = 0;

    // 政策卡被卸下
    virtual void onPolicyUnequipped(int policyId, PolicyType slotType, int slotIndex) = 0;

    // 政策卡组合效果触发
    virtual void onPolicyComboTriggered(const std::vector<int>& policyIds, const std::string& comboName) = 0;
};

// 政策卡管理器类
class PolicyManager : public CultureEventListener {
private:
    // 所有政策卡（按ID索引）
    std::unordered_map<int, PolicyCard> _allPolicies;

    // 已解锁的政策卡ID列表
    std::vector<int> _unlockedPolicyIds;

    // 当前装备的政策卡
    std::vector<EquippedPolicy> _equippedPolicies;

    // 政策卡组合（某些政策卡一起装备时有额外效果）
    std::unordered_map<std::string, std::vector<int>> _policyCombos;

    // 事件监听器
    std::vector<PolicyEventListener*> _listeners;

    // 当前各类型政策槽位数量
    int _militarySlotCount;
    int _economicSlotCount;
    int _diplomaticSlotCount;
    int _wildcardSlotCount;

    // 改为使用函数回调而不是直接持有指针
    std::function<GovernmentType()> _getCurrentGovernmentFunc;
    std::function<std::vector<int>(int)> _getPoliciesByCultureFunc;
    std::function<void(int)> _onPolicyUnlockedCallback;
public:
    PolicyManager();
    ~PolicyManager();

    // 回调函数设置方法
    void setGovernmentGetter(std::function<GovernmentType()> func) {
        _getCurrentGovernmentFunc = func;
    }

    void setPolicyGetter(std::function<std::vector<int>(int)> func) {
        _getPoliciesByCultureFunc = func;
    }

    void setPolicyUnlockedCallback(std::function<void(int)> func) {
        _onPolicyUnlockedCallback = func;
    }

    // 初始化所有政策卡
    void initializePolicies();

    // 设置政策槽位数量（根据当前政体）
    void setPolicySlots(int military, int economic, int diplomatic, int wildcard);

    // 获取政策槽位数量
    void getPolicySlots(int& military, int& economic, int& diplomatic, int& wildcard) const;

    // 解锁政策卡（当市政解锁时调用）
    void unlockPolicyByCulture(int cultureId);

    // 直接解锁政策卡
    bool unlockPolicy(int policyId);

    // 装备政策卡到槽位
    bool equipPolicy(int policyId, PolicyType slotType, int slotIndex);

    // 卸下政策卡
    bool unequipPolicy(int policyId);

    // 检查槽位是否可用
    bool isSlotAvailable(PolicyType slotType, int slotIndex) const;

    // 获取已装备的政策卡
    std::vector<EquippedPolicy> getEquippedPolicies() const { return _equippedPolicies; }

    // 获取已解锁的政策卡
    std::vector<int> getUnlockedPolicies() const { return _unlockedPolicyIds; }

    // 获取所有可用的政策卡（按类型过滤）
    std::vector<PolicyCard> getAvailablePolicies(PolicyType type = PolicyType::WILDCARD) const;

    // 获取政策卡信息
    const PolicyCard* getPolicyCard(int policyId) const;

    // 检查政策卡是否已装备
    bool isPolicyEquipped(int policyId) const;

    // 检查政策卡是否已解锁
    bool isPolicyUnlocked(int policyId) const;

    // 获取政策卡在当前政体下的激活效果
    std::vector<PolicyEffect> getActivePolicyEffects() const;

    // 计算政策卡组合效果
    void checkPolicyCombos();

    // 添加事件监听器
    void addEventListener(PolicyEventListener* listener);

    // 移除事件监听器
    void removeEventListener(PolicyEventListener* listener);

    // 保存/加载政策卡状态
    void savePolicyState(std::string& data) const;
    void loadPolicyState(const std::string& data);

    // 根据文化ID解锁政策（在文化系统回调中使用）
    void unlockPoliciesForCulture(int cultureId);

    // 更新政策槽位（在文化系统回调中使用）
    void updatePolicySlots(int military, int economic, int diplomatic, int wildcard);

    // 检查政策卡是否与当前政体兼容
    bool isPolicyCompatible(int policyId) const;

    virtual void onCultureUnlocked(int cultureId, const std::string& cultureName,
        const std::string& effect) override;

    virtual void onCultureProgress(int cultureId, int currentProgress, int totalCost) override;

    virtual void onInspirationTriggered(int cultureId, const std::string& cultureName) override;
private:
    // 通知监听器
    void notifyPolicyUnlocked(int policyId, const std::string& policyName);
    void notifyPolicyEquipped(int policyId, PolicyType slotType, int slotIndex);
    void notifyPolicyUnequipped(int policyId, PolicyType slotType, int slotIndex);
    void notifyPolicyComboTriggered(const std::vector<int>& policyIds, const std::string& comboName);

    // 检查政策卡是否可以装备到指定槽位
    bool canEquipToSlot(const PolicyCard& card, PolicyType slotType) const;

    // 查找已装备政策卡的迭代器
    std::vector<EquippedPolicy>::iterator findEquippedPolicy(int policyId);
    std::vector<EquippedPolicy>::const_iterator findEquippedPolicy(int policyId) const;

    // 查找指定槽位的政策卡
    const EquippedPolicy* findPolicyInSlot(PolicyType slotType, int slotIndex) const;

    // 内部：添加政策卡组合
    void addPolicyCombo(const std::string& comboName, const std::vector<int>& policyIds);
};

#endif // POLICY_SYSTEM_H