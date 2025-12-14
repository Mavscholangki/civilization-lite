#ifndef __HUD_LAYER_H__
#define __HUD_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Units/Base/AbstractUnit.h" // 引用单位基类
#include "../Development/TechSystem.h"  // 引用科技树
#include "TechTreePanel.h"              // 引用科技树UI

class HUDLayer : public cocos2d::Layer {
public:
    virtual bool init();
    CREATE_FUNC(HUDLayer);

    // 更新顶部资源
    void updateResources(int gold, int science, int turn);

    // 【新功能】显示单位详情面板
    void showUnitInfo(AbstractUnit* unit);
    // 【新功能】隐藏单位详情面板
    void hideUnitInfo();

    // 设置回调
    void setNextTurnCallback(const std::function<void()>& cb);
    void setBuildCityCallback(const std::function<void()>& cb);

    // 新增方法 - 科技树相关
    void openTechTree();                     // 打开科技树面板
    void closeTechTree();                    // 关闭科技树面板
    void setTechTree(TechTree* techTree);    // 设置科技系统
    void updateSciencePerTurn(int science);  // 更新每回合科研值

private:
    cocos2d::Label* _resLabel;  // 资源文字
    cocos2d::ui::Layout* _unitPanel; // 左下角的单位信息面板容器
    cocos2d::Label* _unitNameLabel;  // 面板上的名字
    cocos2d::Label* _unitStatLabel;  // 面板上的属性

    std::function<void()> _onNextTurn;
    cocos2d::ui::Button* _btnBuildCity; // 新增按钮
    std::function<void()> _onBuildCity;

    // 新增成员 - 科技树相关
    cocos2d::ui::Button* _btnTechTree;       // 打开科技树按钮
    TechTreePanel* _techTreePanel;           // 科技树面板
    TechTree* _techTree;                     // 科技系统引用
    bool _isTechTreeOpen;                    // 科技树是否打开
};

#endif