#ifndef __HUD_LAYER_H__
#define __HUD_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Units/Base/AbstractUnit.h"
#include "../Development/TechSystem.h"
#include "../Development/CultureSystem.h"
#include "TechTreePanel.h"
#include "CultureTreePanel.h"
#include "PolicyPanel.h"

class HUDLayer : public cocos2d::Layer {
public:
    virtual bool init();
    CREATE_FUNC(HUDLayer);

    // 更新顶部资源
    void updateResources(int gold, int science, int culture, int turn);

    // 显示单位详情面板
    void showUnitInfo(AbstractUnit* unit);
    // 隐藏单位详情面板
    void hideUnitInfo();

    // 设置回调
    void setNextTurnCallback(const std::function<void()>& cb);
    void setBuildCityCallback(const std::function<void()>& cb);

    // 科技树相关方法
    void openTechTree();
    void closeTechTree();
    void setTechTree(TechTree* techTree);
    void updateSciencePerTurn(int science);

    // 文化树相关方法
    void openCultureTree();
    void closeCultureTree();
    void setCultureTree(CultureTree* cultureTree);
    void updateCulturePerTurn(int culture);


    // 政策系统相关方法
    void openPolicyPanel();
    void closePolicyPanel();
    void setPolicyManager(PolicyManager* policyManager);
    PolicyPanel* getPolicyPanel() const { return _policyPanel; }

private:
    // 创建文明6风格资源显示
    void createCiv6StyleResourceDisplay();
    // 创建文明6风格下一回合按钮
    void createCiv6StyleNextTurnButton();
    // 创建文明6风格功能按钮
    void createCiv6StyleFunctionButtons();

    // 更新资源项显示
    void updateResourceItem(cocos2d::Node* container, int value, int perTurn, const std::string& icon);

    cocos2d::LayerColor* _topBar;           // 顶部资源栏容器
    cocos2d::ui::Layout* _leftPanel;        // 左侧功能按钮面板
    cocos2d::Label* _turnLabel;             // 回合数标签
    cocos2d::Label* _goldLabel;             // 金币标签
    cocos2d::Label* _scienceLabel;          // 科技标签
    cocos2d::Label* _cultureLabel;          // 文化标签

    cocos2d::LayerColor* _unitPanel;        // 左下角的单位信息面板容器
    cocos2d::Label* _unitNameLabel;         // 面板上的名字
    cocos2d::Label* _unitStatLabel;         // 面板上的属性

    std::function<void()> _onNextTurn;
    cocos2d::ui::Button* _btnBuildCity;     // 建城按钮
    std::function<void()> _onBuildCity;

    cocos2d::ui::Button* _btnNextTurn;      // 文明6风格下一回合按钮

    // 科技树相关成员
    cocos2d::ui::Button* _btnTechTree;      // 科技树按钮
    TechTreePanel* _techTreePanel;          // 科技树面板
    TechTree* _techTree;                    // 科技系统引用
    bool _isTechTreeOpen;                   // 科技树是否打开

    // 文化树相关成员
    cocos2d::ui::Button* _btnCultureTree;   // 文化树按钮
    CultureTreePanel* _cultureTreePanel;    // 文化树面板
    CultureTree* _cultureTree;              // 文化系统引用
    bool _isCultureTreeOpen;                // 文化树是否打开

    // 政策系统相关成员
    cocos2d::ui::Button* _btnPolicySystem;  // 政策系统按钮
    PolicyPanel* _policyPanel;              // 政策面板
    PolicyManager* _policyManager;          // 政策管理器引用
    bool _isPolicyPanelOpen;                // 政策面板是否打开

    // 资源容器
    cocos2d::Node* _goldContainer;
    cocos2d::Node* _scienceContainer;
    cocos2d::Node* _cultureContainer;
};

#endif