/*
* 城市基础类
*/
#ifndef __BASE_CITY_H__
#define __BASE_CITY_H__

#include "cocos2d.h"
#include "Utils/HexUtils.h"
#include "District/Base/District.h"
#include "../UI/CityProductionPanel.h"
#include "Yield.h"

class District;

class BaseCity : public cocos2d::Node {
public:
    Hex gridPos;

    static BaseCity* create(int player, Hex pos, std::string name);
	~BaseCity() {
		// 释放区域对象
		for (auto district : districts) {
			delete district;
		}
	}
	bool initCity(int player, Hex pos, std::string name);
	void drawBoundary(); // 绘制城市边界
	void updateYield(); // 更新城市总产出
	void deduceHealth(int damage); // 扣除健康度
    void updateDistribution(); // 更新分配信息
    
	void updatePanel(); // 更新生产面板信息

    // 城市属性
	int ownerPlayer; // 城市所属玩家ID
    std::string cityName;
	int population; // 城市人口
	int unallocated; // 未分配人口
	Yield cityYield; // 城市总产出
	int maxHealth; // 城市最大健康度
	int currentHealth; // 当前健康度
	int addedHealth; // 额外健康度加成(由城墙等提供)
	std::list <District*> districts; // 城市内的区域列表
	std::vector<Hex> territory; // 城市领土范围(包含城市所在格子)
	std::map<Hex, int> populationDistribution; // 人口分配情况(地块坐标 -> 分配人口数)

    
    // 回合结算逻辑
    void onTurnEnd();

private:
    void addDistrict(District* district) {
        districts.push_back(district);
    }
	void addToTerritory(Hex tile) {
		territory.push_back(tile);
		populationDistribution[tile] = 0;
	}
    CityProductionPanel* productionPanelLayer;
    cocos2d::ui::Button * _nameLabel;
    cocos2d::Node* _visual;
	cocos2d::Node* _boundaryVisual;
};

#endif