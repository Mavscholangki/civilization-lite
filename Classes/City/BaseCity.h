/*
* 城市基础类
*/
#ifndef __BASE_CITY_H__
#define __BASE_CITY_H__

#include "cocos2d.h"
#include "Utils/HexUtils.h"
#include "District/Base/District.h"
#include "../UI/CityProductionPanel.h"
#include "Development/ProductionProgram.h"
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
		districts.clear();
		delete currentProduction;
		currentProduction = nullptr;
		// 释放生产队列
		for (auto program : suspendedProductions) {
			delete program;
		}
		suspendedProductions.clear();
	}

	int								getOwnerPlayer() const { return ownerPlayer; } // 城市所属玩家ID
	std::string						getCityName() const { return cityName; }
	int								getPopulation() const { return population; } // 城市人口
	int 							getNeededFoodToMultiply() const { return neededFoodToMultiply; }
	int								getWhenPopulationIncrease() const { return whenPopulationIncrease; } // 下个人口在多少回合后到来
	int								getUnallocated() const { return unallocated; } // 未分配人口
	Yield							getCityYield() const { return cityYield; } // 城市总产出
	ProductionProgram*				getCurrentProduction() const { return currentProduction; } // 当前生产项目
	std::vector<ProductionProgram*> getSuspendedProductions() const { return suspendedProductions; } // 暂停的生产项目列表
	int								getMaxHealth() const { return maxHealth; } // 城市最大健康度
	int								getCurrentHealth() const { return currentHealth; } // 当前健康度
	int								getAddedHealth() const { return addedHealth; } // 额外健康度加成(由城墙等提供)
	std::list <District*>			getDistricts() const { return districts; } // 城市内的区域列表
	std::vector<Hex>				getTerritory() const { return territory; } // 城市领土范围(包含城市所在格子)
	std::map<Hex, int>				getPopulationDistribution() const { return populationDistribution; } // 人口分配情况(地块坐标 -> 分配人口数)



	bool initCity(int player, Hex pos, std::string name);
	void drawTerritory(); // 绘制城市边界
	void updateYield(); // 更新城市总产出
	void updatePopulation(); // 更新人口增长情况
	void addNewProduction(ProductionProgram* newProgram); // 添加新的生产项目
	void updateProduction(); // 更新生产
	void purchaseDirectly(ProductionProgram* newProgram); // 购买生产项目
	void deduceHealth(int damage); // 扣除健康度
    void updateDistribution(); // 更新分配信息
    

	void updatePanel(); // 更新生产面板信息

    // 城市属性
	int ownerPlayer; // 城市所属玩家ID
    std::string cityName;
	int population; // 城市人口
	int neededFoodToMultiply; // 下个人口需要多少增长量
	float currentAccumulation; // 当前积累的增长量
	int whenPopulationIncrease; // 下个人口在多少回合后到来
	int unallocated; // 未分配人口
	Yield cityYield; // 城市总产出
	ProductionProgram* currentProduction; // 当前生产项目
	std::vector<ProductionProgram*> suspendedProductions; // 暂停的生产项目列表
	int maxHealth; // 城市最大健康度
	int currentHealth; // 当前健康度
	int addedHealth; // 额外健康度加成(由城墙等提供)
	std::list <District*> districts; // 城市内的区域列表
	std::vector<Hex> territory; // 城市领土范围(包含城市所在格子)
	std::vector<Hex> vacantTiles; // 空闲格
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
    cocos2d::ui::Button * _nameLabel;
    cocos2d::Node* _visual;
	cocos2d::Node* _boundaryVisual;
};

#endif