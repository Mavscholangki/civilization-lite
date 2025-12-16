/*
* 城市基础类
*/
#ifndef __BASE_CITY_H__
#define __BASE_CITY_H__

#include "cocos2d.h"
#include "Utils/HexUtils.h"
#include "District/Base/District.h"
#include "Yield.h"

class District;

class BaseCity : public cocos2d::Node {
public:
    Hex gridPos;

    static BaseCity* create(Hex pos, std::string name);
    bool initCity(Hex pos, std::string name);
	void drawBoundary(); // 绘制城市边界
    // 城市属性
    std::string cityName;
    int population;
	Yield cityYield; // 城市总产出
	std::list <District*> districts; // 城市内的区域列表
	std::vector<Hex> territory; // 城市领土范围(包含城市所在格子)


    // 回合结算逻辑
    void onTurnEnd();

private:
    void addDistrict(District* district) {
        districts.push_back(district);
    }
	void addToTerritory(Hex tile) {
		territory.push_back(tile);
	}
    cocos2d::Label* _nameLabel;
    cocos2d::Node* _visual;
	cocos2d::Node* _boundaryVisual;
};

#endif