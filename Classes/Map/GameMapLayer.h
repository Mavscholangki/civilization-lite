#ifndef __GAME_MAP_LAYER_H__
#define __GAME_MAP_LAYER_H__

#include "cocos2d.h"
#include <map> // 必须引用 map
#include "../Utils/HexUtils.h"
#include "TileData.h"
#include "../Units/Base/AbstractUnit.h"
#include <functional> 
#include "../Units/Civilian/Settler.h" // 引用开拓者
#include "../City/BaseCity.h"          // 引用城市

class GameMapLayer : public cocos2d::Layer {
public:
    virtual bool init();
    CREATE_FUNC(GameMapLayer);
    void setOnUnitSelectedCallback(const std::function<void(AbstractUnit*)>& cb);
    void onBuildCityAction();
    void onNextTurnAction(); // 顺便把下回合也接上
private:
    // --- 核心逻辑 ---
    void generateMap();
    void drawHex(cocos2d::Vec2 pos, float size, cocos2d::Color4F color);
    void updateSelection(Hex h); // 画选中框
    int getTerrainCost(Hex h);   // 获取地形消耗 (给寻路用)

    // --- 触摸事件 ---
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // --- 成员变量 ---
    HexLayout* _layout;
    AbstractUnit* _myUnit;     // 你的红点勇士
    cocos2d::DrawNode* _selectionNode; // 黄色选中框
    bool _isDragging;          // 是否在拖拽地图
    std::function<void(AbstractUnit*)> _onUnitSelected;
    // 【关键】把地图数据存成成员变量，而不是局部变量
    std::map<Hex, TileData> _mapData;
    std::vector<BaseCity*> _cities; // 管理所有城市
    AbstractUnit* _selectedUnit;    // 记录当前选中的是谁
};

#endif