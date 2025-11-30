#ifndef __HUD_LAYER_H__
#define __HUD_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Units/Base/AbstractUnit.h" // 引用单位基类

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

private:
    cocos2d::Label* _resLabel;  // 资源文字
    cocos2d::ui::Layout* _unitPanel; // 左下角的单位信息面板容器
    cocos2d::Label* _unitNameLabel;  // 面板上的名字
    cocos2d::Label* _unitStatLabel;  // 面板上的属性

    std::function<void()> _onNextTurn;
    cocos2d::ui::Button* _btnBuildCity; // 新增按钮
    std::function<void()> _onBuildCity;
};

#endif