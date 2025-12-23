#ifndef POLICY_PANEL_H
#define POLICY_PANEL_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Development/PolicySystem.h"

USING_NS_CC;
using namespace ui;

class PolicyPanel : public Layer {
public:
    CREATE_FUNC(PolicyPanel);
    virtual bool init() override;

    void setPolicyManager(PolicyManager* mgr);
    void setCultureTree(CultureTree* tree);
    void refreshUI();

private:
    PolicyManager* _policyManager = nullptr;
    CultureTree* _cultureTree = nullptr;

    Node* _leftPanel = nullptr;
    Node* _rightPanel = nullptr;
    Node* _leftSlotsContainer = nullptr;
    Label* _govLabel = nullptr;

    // 拖拽相关
    Node* _draggedNode = nullptr; // 拖拽时的替身节点
    int _draggedCardId = -1;
    PolicyType _draggedCardType;
    bool _isDragActive = false;   // 标记当前是否处于拖拽状态

    // 简介面板
    Node* _descriptionPanel = nullptr;
    Label* _descriptionTitle = nullptr;
    Label* _descriptionText = nullptr;

    struct SlotTarget {
        PolicyType type;
        int index;
        Rect worldBoundingBox;
        bool isOccupied;
    };
    std::vector<SlotTarget> _slotTargets;

    // 【修改点】：变大，变成竖着的长方形
    const float CARD_W = 140.0f;
    const float CARD_H = 220.0f;

    // 颜色配置
    const Color3B COLOR_MIL = Color3B(180, 40, 40);   // 深红
    const Color3B COLOR_ECO = Color3B(220, 180, 40);  // 金黄
    const Color3B COLOR_WILD = Color3B(100, 40, 180); // 紫色
    const Color3B COLOR_BG_SLOT = Color3B(30, 30, 35); // 槽位底色

    void initLayout();
    void createLeftSlots();
    void createRightList();

    // 核心 UI 创建
    Node* createDraggableCard(const PolicyCard& card);
    Node* createSlotUI(PolicyType type, int index, int equippedCardId);
    Node* createCardVisual(const PolicyCard& card, bool isGhost); // 提取出来的通用外观创建函数

    // 拖拽逻辑
    void onCardDragBegan(int cardId, PolicyType type, Vec2 touchPos);
    void onCardDragMoved(Vec2 touchPos);
    void onCardDragEnded(Vec2 touchPos);
    void onEquippedCardDragEnded(Touch* touch); // 从槽位卸载的逻辑

    void onChangeGovClicked();
    void showGovSelectLayer();
    Node* createGovOptionUI(GovernmentType type);
    std::vector<GovernmentType> getAllGovTypes();

    void createDescriptionPanel();
    void showPolicyDescription(const PolicyCard& card);
    void hidePolicyDescription();
    void onCardClicked(int cardId, const PolicyCard& card);

    Color3B getTypeColor(PolicyType type);
    std::string getTypeName(PolicyType type);
};

#endif // POLICY_PANEL_H