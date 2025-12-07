#ifndef TECH_TREE_PANEL_H
#define TECH_TREE_PANEL_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "./Development/TechSystem.h"

USING_NS_CC;
using namespace ui;

// 科技节点UI状态
enum class TechNodeState {
    LOCKED,          // 未解锁
    RESEARCHABLE,    // 可研究
    RESEARCHING,     // 研究中（当前正在研究）
    IN_PROGRESS,     // 有进度但未设为当前研究
    ACTIVATED        // 已激活
};

// 科技树面板
class TechTreePanel : public Layer, public TechEventListener {
private:
    TechTree* techTree;                   // 科技系统引用
    std::unordered_map<int, Node*> nodeUIMap; // 科技ID到UI节点的映射

    // UI元素
    ScrollView* scrollView;               // 滚动容器
    Node* contentNode;                    // 内容节点
    LayerColor* background;               // 背景

    // 当前显示详情
    Node* detailPanel;                    // 详情面板

    // 底部控制面板
    Node* controlPanel;                   // 底部控制面板
    Label* currentResearchLabel;          // 当前研究显示
    ProgressTimer* researchProgressBar;   // 研究进度条
    Label* sciencePerTurnLabel;           // 每回合科研显示

    // UI常量
    const float NODE_SIZE = 90.0f;
    const float NODE_SPACING = 140.0f;
    const float LINE_WIDTH = 4.0f;
    const Color4F LINE_COLOR = Color4F(0.4f, 0.4f, 0.5f, 0.6f);
    const Color4F LINE_ACTIVE_COLOR = Color4F(0.2f, 0.8f, 0.2f, 0.8f);
    const Color4F LINE_CURRENT_COLOR = Color4F(0.9f, 0.6f, 0.1f, 0.8f); // 当前研究的连接线颜色

public:
    CREATE_FUNC(TechTreePanel);

    // 设置科技系统
    void setTechTree(TechTree* tree);

    // 初始化UI
    virtual bool init() override;

    // 更新UI
    void refreshUI();

    // 清理
    virtual void onExit() override;

    // TechEventListener接口实现
    virtual void onTechActivated(int techId, const std::string& techName,
        const std::string& effect) override;
    virtual void onResearchProgress(int techId, int currentProgress, int totalCost) override;
    virtual void onEurekaTriggered(int techId, const std::string& techName) override;

    // 设置每回合科研值（供外部调用更新）
    void setSciencePerTurn(int science);

private:
    // 创建科技节点UI
    Node* createTechNodeUI(const TechNode* techData);

    // 创建连接线
    void createConnectionLine(int fromTechId, int toTechId);

    // 更新科技节点UI状态
    void updateNodeUIState(int techId, TechNodeState state);

    // 节点点击回调
    void onTechNodeClicked(Ref* sender, Widget::TouchEventType type);

    // 显示科技详情
    void showTechDetail(int techId);

    // 隐藏科技详情
    void hideTechDetail();

    // 设置为当前研究
    void setAsCurrentResearch(int techId);

    // 布局科技树
    void layoutTechTree();

    // 创建尤里卡特效
    void createEurekaEffect(int techId);

    // 更新底部控制面板
    void updateControlPanel();

    // 计算节点位置（可以根据依赖关系自动布局）
    Vec2 calculateNodePosition(int techId);

    // 获取科技时代（用于布局）
    int getTechEra(int techId) const;

    // 更新连接线状态
    void updateConnectionLines();
};

#endif // TECH_TREE_PANEL_H