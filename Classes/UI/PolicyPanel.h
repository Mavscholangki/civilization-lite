#ifndef POLICY_PANEL_H
#define POLICY_PANEL_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Development/CultureSystem.h"
#include "../Development/PolicySystem.h"

USING_NS_CC;
using namespace ui;

class PolicyPanel;

// 政策面板事件监听器
class PolicyPanelEventListener : public PolicyEventListener, public CultureEventListener {
private:
    PolicyPanel* _owner;

public:
    PolicyPanelEventListener(PolicyPanel* owner) : _owner(owner) {}

    // PolicyEventListener接口
    virtual void onPolicyUnlocked(int policyId, const std::string& policyName) override;
    virtual void onPolicyEquipped(int policyId, PolicyType slotType, int slotIndex) override;
    virtual void onPolicyUnequipped(int policyId, PolicyType slotType, int slotIndex) override;
    virtual void onPolicyComboTriggered(const std::vector<int>& policyIds,
        const std::string& comboName) override;

    // CultureEventListener接口
    virtual void onCultureUnlocked(int cultureId, const std::string& cultureName,
        const std::string& effect) override;
    virtual void onCultureProgress(int cultureId, int currentProgress, int totalCost) override;
    virtual void onInspirationTriggered(int cultureId, const std::string& cultureName) override;
};

// 政策卡UI槽位状态
enum class PolicySlotState {
    EMPTY,          // 空槽位
    OCCUPIED,       // 已被占用
    INCOMPATIBLE    // 不兼容当前政体
};

// 政策面板类
class PolicyPanel : public Layer {
    friend class PolicyPanelEventListener;

public:
    CREATE_FUNC(PolicyPanel);

    virtual bool init() override;
    virtual void onExit() override;

    // 设置政策管理器
    void setPolicyManager(PolicyManager* policyManager);

    // 设置文化树
    void setCultureTree(CultureTree* cultureTree);

    // 刷新UI
    void refreshUI();

    // 获取当前选中的政策卡ID
    int getSelectedPolicyId() const { return _selectedPolicyId; }

    // 设置选中的政策卡
    void setSelectedPolicy(int policyId);

private:
    // 系统引用
    PolicyManager* _policyManager = nullptr;
    CultureTree* _cultureTree = nullptr;

    // 事件监听器
    PolicyPanelEventListener* _eventListener = nullptr;

    // UI元素
    LayerColor* _background = nullptr;
    Node* _contentNode = nullptr;

    // 顶部信息栏
    Label* _currentGovernmentLabel = nullptr;
    Label* _policySlotsLabel = nullptr;

    // 政策卡类型标签页
    TabControl* _policyTabs = nullptr;
    std::unordered_map<PolicyType, ScrollView*> _policyListViews;

    // 政策槽位UI
    struct PolicySlotUI {
        Layout* slotBg = nullptr;
        Label* slotLabel = nullptr;
        Node* policyCardNode = nullptr;
        PolicySlotState state = PolicySlotState::EMPTY;
    };

    std::unordered_map<PolicyType, std::vector<PolicySlotUI>> _policySlotsUI;

    // 选中的政策卡
    int _selectedPolicyId = -1;
    Node* _selectedCardNode = nullptr;

    // 详情面板
    LayerColor* _detailPanel = nullptr;
    Label* _policyNameLabel = nullptr;
    Label* _policyDescLabel = nullptr;
    Label* _policyEffectLabel = nullptr;
    Button* _equipButton = nullptr;
    Button* _unequipButton = nullptr;

    // 组合效果显示
    std::vector<Label*> _comboLabels;

    // UI常量
    const float CARD_WIDTH = 200.0f;
    const float CARD_HEIGHT = 100.0f;
    const float SLOT_WIDTH = 180.0f;
    const float SLOT_HEIGHT = 80.0f;
    const Color3B POLICY_COLORS[4] = {
        Color3B(255, 100, 100),   // 军事 - 红色
        Color3B(100, 255, 100),   // 经济 - 绿色
        Color3B(100, 150, 255),   // 外交 - 蓝色
        Color3B(255, 200, 50)     // 通用 - 金色
    };

    // 私有方法
    Node* createPolicyCardUI(const PolicyCard* policy);
    void createPolicySlotsUI();
    void updatePolicySlotsUI();
    void updatePolicyCardsUI();
    void showPolicyDetail(int policyId);
    void hidePolicyDetail();
    void equipPolicyToSlot(int policyId, PolicyType slotType, int slotIndex);
    void unequipPolicy(int policyId);
    PolicySlotState getSlotState(PolicyType slotType, int slotIndex) const;
    std::string getPolicyTypeName(PolicyType type) const;
    std::string getGovernmentName(GovernmentType gov) const;

    // 事件处理
    void handlePolicyUnlocked(int policyId, const std::string& policyName);
    void handlePolicyEquipped(int policyId, PolicyType slotType, int slotIndex);
    void handlePolicyUnequipped(int policyId, PolicyType slotType, int slotIndex);
    void handlePolicyComboTriggered(const std::vector<int>& policyIds,
        const std::string& comboName);
    void handleCultureUnlocked(int cultureId, const std::string& cultureName,
        const std::string& effect);

    // 工具方法
    bool isPolicyCompatible(int policyId) const;
    int findAvailableSlot(PolicyType type) const;
};

#endif // POLICY_PANEL_H