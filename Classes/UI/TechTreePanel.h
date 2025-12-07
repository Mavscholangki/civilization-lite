#ifndef TECH_TREE_PANEL_H
#define TECH_TREE_PANEL_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Development/TechSystem.h"

USING_NS_CC;
using namespace ui;

// 科技节点UI状态
enum class TechNodeState {
    LOCKED,      // 未解锁
    RESEARCHABLE, // 可研究
    RESEARCHING,  // 研究中
    ACTIVATED     // 已激活
};

// 科技树面板
class TechTreePanel : public Layer, public TechEventListener {
private:
    TechTree* techTree;                   // 科技系统引用
    std::unordered_map<int, Node*> nodeUIMap; // 科技ID到UI节点的映射

    // UI元素
    ScrollView* scrollView;               // 滚动容器
    Node* contentNode;                    // 内容节点
    Sprite* background;                   // 背景

    // 当前选中的科技
    int selectedTechId;

    // UI常量
    const float NODE_SIZE = 80.0f;
    const float NODE_SPACING = 120.0f;
    const float LINE_WIDTH = 3.0f;
    const Color4F LINE_COLOR = Color4F(0.5f, 0.5f, 0.5f, 0.7f);
    const Color4F LINE_ACTIVE_COLOR = Color4F(0.2f, 0.8f, 0.2f, 1.0f);

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
    virtual void onTechActivated(int techId, const std::string& techName, const std::string& effect) override;
    virtual void onResearchProgress(int techId, int currentProgress) override;
    virtual void onEurekaTriggered(int techId, const std::string& techName) override;

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

    // 发出研究指令，送至TechSystem进行处理
    void sendResearchCommand(int techId);

    // 布局科技树
    void layoutTechTree();

    // 设置节点位置
    void setNodePosition(int techId, const Vec2& pos);

    // 创建尤里卡特效
    void createEurekaEffect(int techId);
};

#endif // TECH_TREE_PANEL_H