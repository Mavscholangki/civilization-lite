#ifndef TECH_TREE_PANEL_H
#define TECH_TREE_PANEL_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "./Development/TechSystem.h"

USING_NS_CC;
using namespace ui;

class TechTreePanel;

// 科技节点UI状态
enum class TechNodeState {
    LOCKED,          // 未解锁
    RESEARCHABLE,    // 可研究
    RESEARCHING,     // 研究中（当前正在研究）
    IN_PROGRESS,     // 有进度但未设为当前研究
    ACTIVATED        // 已激活
};

// 内部事件监听器类（作为TechTreePanel的成员）
class TechTreePanelEventListener : public TechEventListener {
private:
    TechTreePanel* _owner;  // 指向外部TechTreePanel的指针

public:
    TechTreePanelEventListener(TechTreePanel* owner) : _owner(owner) {}

    // TechEventListener接口实现
    virtual void onTechActivated(int techId, const std::string& techName,
        const std::string& effect) override;

    virtual void onResearchProgress(int techId, int currentProgress, int totalCost) override;

    virtual void onEurekaTriggered(int techId, const std::string& techName) override;
};

// 科技树面板
class TechTreePanel : public Layer {
    friend class TechTreePanelEventListener;  // 允许事件监听器访问私有成员

public:
    CREATE_FUNC(TechTreePanel);

    virtual bool init() override;
    virtual void onExit() override;

    // 设置科技系统
    void setTechTree(TechTree* tree);

    // 更新UI
    void refreshUI();

    // 设置每回合科研值
    void setSciencePerTurn(int science);

    // 显示科技详情
    void showTechDetail(int techId);

    // 隐藏科技详情
    void hideTechDetail();

    // 提供给事件监听器调用的公共方法
    void handleTechActivated(int techId, const std::string& techName, const std::string& effect);
    void handleResearchProgress(int techId, int currentProgress, int totalCost);
    void handleEurekaTriggered(int techId, const std::string& techName);

private:
    // 科技系统引用
    TechTree* _techTree = nullptr;

    // 事件监听器
    TechTreePanelEventListener* _eventListener = nullptr;

    // UI元素
    LayerColor* _background = nullptr;
    ScrollView* _scrollView = nullptr;
    Node* _contentNode = nullptr;

    // 底部控制面板
    Node* _controlPanel = nullptr;
    Label* _currentResearchLabel = nullptr;
    ProgressTimer* _researchProgressBar = nullptr;
    Label* _sciencePerTurnLabel = nullptr;

    // 详情面板
    LayerColor* _detailPanel = nullptr;

    // 节点映射
    std::unordered_map<int, Node*> _nodeUIMap;

    // UI常量
    const float NODE_WIDTH = 180.0f;
    const float NODE_HEIGHT = 110.0f;
    const float NODE_CORNER_RADIUS = 10.0f;
    const float ERA_SPACING = 280.0f;
    const float NODE_SPACING = 120.0f;
    const float LINE_WIDTH = 4.0f;

    // 私有方法
    Node* createTechNodeUI(const TechNode* techData);
    void updateNodeUIState(int techId, TechNodeState state);
    void updateControlPanel();
    void updateConnectionLines();
    void drawRoundedRect(DrawNode* drawNode, const Rect& rect, float radius, const Color4F& color);
    void onTechNodeClicked(Ref* sender, Widget::TouchEventType type);
    void setAsCurrentResearch(int techId);
    void createEurekaEffect(int techId);
    void drawColumnBackgrounds();  // 添加这个声明

    // 布局相关的新方法（确保与cpp中的实现匹配）
    std::vector<int> topologicalSort();
    void calculateTechDepths();
    void layoutTechTree();
    void createSplineConnection(int fromTechId, int toTechId);
    void adjustPositionsForConnections();
    bool doLinesCross(const Vec2& p1, const Vec2& p2, const Vec2& q1, const Vec2& q2);

    // 布局数据结构
    std::unordered_map<int, int> _techDepth;      // 科技深度（列）
    std::unordered_map<int, int> _techRow;        // 科技行号
    std::unordered_map<int, Vec2> _techPositions; // 最终位置
    std::unordered_map<int, std::vector<int>> _techByDepth; // 按深度分组的科技
};

#endif // TECH_TREE_PANEL_H