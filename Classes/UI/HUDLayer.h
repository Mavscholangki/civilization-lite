#ifndef __HUD_LAYER_H__
#define __HUD_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Units/Base/AbstractUnit.h"
#include "../Development/TechSystem.h"
#include "../Development/CultureSystem.h"
#include "TechTreePanel.h"
#include "CultureTreePanel.h"

class HUDLayer : public cocos2d::Layer {
public:
    virtual bool init();
    CREATE_FUNC(HUDLayer);

    // 更新顶部资源
    void updateResources(int gold, int science, int culture, int turn);

    // 【新功能】显示单位详情面板
    void showUnitInfo(AbstractUnit* unit);
    // 【新功能】隐藏单位详情面板
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

private:
    cocos2d::Label* _resLabel;  // 资源文字
    cocos2d::ui::Layout* _unitPanel; // 左下角的单位信息面板容器
    cocos2d::Label* _unitNameLabel;  // 面板上的名字
    cocos2d::Label* _unitStatLabel;  // 面板上的属性

    std::function<void()> _onNextTurn;
    cocos2d::ui::Button* _btnBuildCity; // 新增按钮
    std::function<void()> _onBuildCity;

    // 科技树相关成员
    cocos2d::ui::Button* _btnTechTree;       // 打开科技树按钮
    TechTreePanel* _techTreePanel;           // 科技树面板
    TechTree* _techTree;                     // 科技系统引用
    bool _isTechTreeOpen;                    // 科技树是否打开

    // 文化树相关成员
    cocos2d::ui::Button* _btnCultureTree;       // 文化树按钮
    CultureTreePanel* _cultureTreePanel;        // 文化树面板
    CultureTree* _cultureTree;                  // 文化系统引用
    bool _isCultureTreeOpen;                    // 文化树是否打开
};

#endif