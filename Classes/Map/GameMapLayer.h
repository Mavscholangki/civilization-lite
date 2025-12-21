#ifndef __GAME_MAP_LAYER_H__
#define __GAME_MAP_LAYER_H__

#include "cocos2d.h"
#include <map>
#include "../Utils/HexUtils.h"
#include "TileData.h"
#include "../Units/Base/AbstractUnit.h"
#include <functional> 
#include "../Units/Civilian/Settler.h"
#include "../City/BaseCity.h"

class BaseCity;

class GameMapLayer : public cocos2d::Layer {
public:
    virtual bool init();
    CREATE_FUNC(GameMapLayer);
    void setOnUnitSelectedCallback(const std::function<void(AbstractUnit*)>& cb);
    void onBuildCityAction();
    void onNextTurnAction();
	TileData getTileData(Hex h);
private:
    void generateMap();
    void drawHexOnNode(cocos2d::DrawNode* node, cocos2d::Vec2 pos, float size, cocos2d::Color4F color);
    void drawTileResources(Hex hex, const TileData& data);
    void updateSelection(Hex h);
    int getTerrainCost(Hex h);

    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    HexLayout* _layout;
    AbstractUnit* _myUnit;
    cocos2d::DrawNode* _selectionNode;
    cocos2d::DrawNode* _tilesDrawNode;
    bool _isDragging;
    std::function<void(AbstractUnit*)> _onUnitSelected;
    std::map<Hex, TileData> _mapData;
    std::vector<BaseCity*> _cities;
    AbstractUnit* _selectedUnit;
};

#endif