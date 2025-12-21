/*
* 区域类
*/

#ifndef __DISTRICT_H__
#define __DISTRICT_H__

#include "Utils/HexUtils.h"
#include "District/Building/Building.h"
#include "Map/TileData.h"
#include "City/Yield.h"
#include <vector> 

class District {
public:
	enum class DistrictType { // 区域类型枚举
		DOWNTOWN, // 市中心(默认区域类型)
		CAMPUS, // 学院
		INDUSTRY_ZONE, // 工业区
		COMMERCIAL_HUB, // 商业中心
		THEATER_SQUARE, // 剧院广场
		HARBOR, // 港口
		SPACEPORT, // 航天中心
		//... 军营
		TO_BE_DEFINED // 未定义
	};

	District(Hex pos, DistrictType type, std::string name);
	~District() {
		// 释放建筑对象
		for (auto building : buildings) {
			delete building;
		}
	}
	virtual bool canErectDistrict(Hex where);
	bool updateProduction(int productionYield);
	virtual void calculateBonus();
	void updateMaintenanceCost();
	void updateGrossYield();
	void updateCitizenBenefit();

	virtual bool addBuilding(Building::BuildingType building) = 0;
	inline int			getID() const { return _id; }
	inline DistrictType getType() const { return _type; }
	inline std::string	getName() { return _name; }
	inline Hex			getPos() const { return _pos; }
	inline bool			getIsConstructed() const { return isConstructed; }
	inline int			getProductionCost() const { return productionCost; }
	inline int			getConstructionProgress() const { return currentProgress; }
	inline int			getTurnsRemaining() const { return turnsRemaining; }
	inline std::string	getPrereqTech() const { return prereqTech; }
	inline std::string	getPrereqCivic() const { return prereqCivic; }
	inline int			getMaintenanceCost() const { return maintanenceCost; }
	inline Yield		getCitizenBenefit() const { return citizenBenefit; }
	inline Yield		getYield() const { return grossYield; }

protected:
	std::vector<Hex> getHexNeighbors(Hex center);
	// 计数所有的区域,便于编号
	static int count;

	// 基础信息
	int _id; // 区域的唯一标志符
	DistrictType _type; // 区域类型
	std::string _name; // 区域名称

	// 归属相关
	Hex _pos; // 区域坐标
	 
	// 建造相关
	bool isConstructed; // 区域是否建成
	int productionCost; // 区域所耗生产力
	int currentProgress; // 区域建造进度
	int turnsRemaining; // 区域建成预计剩余回合数
	
	std::string prereqTech; // 建造前置科技
	std::string prereqCivic; // 建造前置市政

	// 维护相关
	int baseMaintenanceCost; // 区域维护费用
	int buildingMaintenanceCost;// 建筑维护费用
	int maintanenceCost; // 区域总维护费用

	// 产出相关
	Yield buildingBonus; // 建筑加成产出
	Yield adjacencyBonus; // 区域相邻加成
	Yield grossYield; // 区域总产出(前两项之和)
	Yield baseBenefit; // 区域基础福利影响
	Yield buildingBenefit; // 建筑对市民的福利影响
	Yield citizenBenefit; // 区域对市民的福利影响

	// 建筑
	std::vector<Building*> buildings; // 区域容纳的建筑列表
};

class Downtown : public District {
public:
	Downtown(Hex pos, std::string name);
	virtual void calculateBonus();
	virtual bool canErectDistrict(Hex where);
	virtual bool addBuilding(Building::BuildingType buildingType);
	cocos2d::Node* _downtownVisual;
};


#endif // __DISTRICT_H__
