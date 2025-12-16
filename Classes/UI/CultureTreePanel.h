#ifndef CULTURE_TREE_PANEL_H
#define CULTURE_TREE_PANEL_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "./Development/CultureSystem.h"

USING_NS_CC;
using namespace ui;

class CultureTreePanel;

// 市政节点UI状态 - 与科技树保持一致
enum class CultureNodeState {
    LOCKED,           // 未解锁
    UNLOCKABLE,       // 可解锁（对应科技树的RESEARCHABLE）
    RESEARCHING,      // 研究中（当前正在研究）
    IN_PROGRESS,      // 有进度但未设为当前研究
    ACTIVATED         // 已激活
};

// 内部事件监听器类
class CultureTreePanelEventListener : public CultureEventListener {
private:
    CultureTreePanel* _owner;

public:
    CultureTreePanelEventListener(CultureTreePanel* owner) : _owner(owner) {}

    // CultureEventListener接口实现
    virtual void onCultureUnlocked(int cultureId, const std::string& cultureName,
        const std::string& effect) override;
    virtual void onCultureProgress(int cultureId, int currentProgress, int totalCost) override;
    virtual void onInspirationTriggered(int cultureId, const std::string& cultureName) override;
};

// 文化树面板
class CultureTreePanel : public Layer {
    friend class CultureTreePanelEventListener;

public:
    CREATE_FUNC(CultureTreePanel);

    virtual bool init() override;
    virtual void onExit() override;

    // 设置文化系统
    void setCultureTree(CultureTree* tree);

    // 设置每回合文化值
    void setCulturePerTurn(int culture);

    // 更新UI
    void refreshUI();

    // 显示市政详情
    void showCultureDetail(int cultureId);

    // 隐藏市政详情
    void hideCultureDetail();

    // 提供给事件监听器调用的公共方法
    void handleCultureUnlocked(int cultureId, const std::string& cultureName, const std::string& effect);
    void handleCultureProgress(int cultureId, int progress, int totalCost);
    void handleInspirationTriggered(int cultureId, const std::string& cultureName);

private:
    // 文化系统引用
    CultureTree* _cultureTree = nullptr;

    // 事件监听器
    CultureTreePanelEventListener* _eventListener = nullptr;

    // UI元素
    LayerColor* _background = nullptr;
    ScrollView* _scrollView = nullptr;
    Node* _contentNode = nullptr;

    // 底部控制面板
    Node* _controlPanel = nullptr;
    Label* _currentGovernmentLabel = nullptr;
    Label* _policySlotsLabel = nullptr;
    Label* _culturePerTurnLabel = nullptr;

    // 详情面板
    LayerColor* _detailPanel = nullptr;

    // 节点映射
    std::unordered_map<int, Node*> _nodeUIMap;

    // UI常量 - 与科技树保持一致
    const float NODE_WIDTH = 180.0f;
    const float NODE_HEIGHT = 110.0f;
    const float COLUMN_SPACING = 280.0f;
    const float ROW_SPACING = 150.0f;
    const float LINE_WIDTH = 4.0f;

    // 私有方法
    Node* createCultureNodeUI(const CultureNode* cultureData);
    void updateNodeUIState(int cultureId, CultureNodeState state);
    void updateControlPanel();
    void updateConnectionLines();
    void onCultureNodeClicked(Ref* sender, Widget::TouchEventType type);
    void switchGovernment(GovernmentType government);
    void createInspirationEffect(int cultureId);
    void drawColumnBackgrounds();
    void setAsCurrentResearch(int cultureId);  // 新增：设为当前研究

    // 布局相关方法
    std::vector<int> topologicalSort();
    void calculateCultureDepths();
    void layoutCultureTree();
    void createSplineConnection(int fromCultureId, int toCultureId);
    void adjustPositionsForConnections();
    bool doLinesCross(const Vec2& p1, const Vec2& p2, const Vec2& q1, const Vec2& q2);

    // 布局数据结构
    std::unordered_map<int, int> _cultureDepth;      // 市政深度（列）
    std::unordered_map<int, Vec2> _culturePositions; // 最终位置
    std::unordered_map<int, std::vector<int>> _cultureByDepth; // 按深度分组的市政
};

#endif // CULTURE_TREE_PANEL_H