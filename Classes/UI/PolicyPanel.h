#ifndef POLICY_PANEL_H
#define POLICY_PANEL_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Development/PolicySystem.h" // 请确保路径正确指向你的 PolicySystem.h

USING_NS_CC;
using namespace ui;

class PolicyPanel : public Layer {
public:
    CREATE_FUNC(PolicyPanel);
    virtual bool init() override;

    // 注入依赖
    void setPolicyManager(PolicyManager* mgr);
    void setCultureTree(CultureTree* tree);

    // 刷新界面数据
    void refreshUI();

private:
    // 系统引用
    PolicyManager* _policyManager = nullptr;
    CultureTree* _cultureTree = nullptr;

    // --- UI 容器 ---
    Node* _leftPanel = nullptr;
    Node* _rightPanel = nullptr;
    Node* _leftSlotsContainer = nullptr; // 用于存放槽位，防止刷新时误删标题
    Label* _govLabel = nullptr;          // 政体标题

    // --- 拖拽状态 ---
    Node* _draggedNode = nullptr; // 拖拽时的替身节点
    int _draggedCardId = -1;      // 当前拖拽的卡牌ID
    PolicyType _draggedCardType;  // 当前拖拽的卡牌类型

    // --- 槽位碰撞检测结构 ---
    struct SlotTarget {
        PolicyType type;
        int index;
        Rect worldBoundingBox;
        bool isOccupied;
    };
    std::vector<SlotTarget> _slotTargets;

    // --- UI常量 ---
    const float CARD_W = 180.0f;
    const float CARD_H = 60.0f;
    const Color3B COLOR_MIL = Color3B(220, 80, 80);
    const Color3B COLOR_ECO = Color3B(240, 200, 60);
    const Color3B COLOR_WILD = Color3B(140, 60, 200);

    // --- 内部方法 ---
    void initLayout();       // 初始化左右布局

    void createLeftSlots();  // 创建左侧槽位
    void createRightList();  // 创建右侧统一列表 (取代了原来的 Tabs)

    // 创建UI组件
    Node* createDraggableCard(const PolicyCard& card);
    Node* createSlotUI(PolicyType type, int index, int equippedCardId);

    // 拖拽逻辑
    void onCardDragBegan(Node* cardNode, int cardId, PolicyType type, Touch* touch);
    void onCardDragMoved(Touch* touch);
    void onCardDragEnded(Touch* touch);         // 从右侧拖入左侧结束
    void onEquippedCardDragEnded(Touch* touch); // 从左侧槽位拖出结束(卸下)

    // --- 政体选择相关 ---
    void onChangeGovClicked();      // 点击更换按钮
    void showGovSelectLayer();      // 显示选择弹窗
    Node* createGovOptionUI(GovernmentType type); // 创建单个政体选项
    std::vector<GovernmentType> getAllGovTypes(); // 获取所有政体列表

    // 辅助工具
    Color3B getTypeColor(PolicyType type);
    std::string getTypeName(PolicyType type);
};

#endif // POLICY_PANEL_H