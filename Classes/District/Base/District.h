/*
* 区域类
*/

#ifndef __DISTRICT_H__
#define __DISTRICT_H__

#include "Utils/HexUtils.h"
#include "Map/TileData.h"
#include "City/Yield.h"
#include "Development/ProductionProgram.h"
#include <vector> 

class Building;


class District : public ProductionProgram {
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

	enum class BuildingCategory
	{
		PALACE, // 宫殿
		MONUMENT, // 纪念碑
		GRANARY, // 谷仓

		LIBRARY, // 图书馆
		UNIVERSITY, // 大学
		LABORATORY, // 实验室

		MARKET, // 市场
		BANK, // 银行
		STOCK_EXCHANGE, // 证券交易所

		WORKSHOP, // 工坊
		FACTORY, // 工厂
		POWER_PLANT, // 发电厂
		RHUR_VALLEY, // 鲁尔工业区(特殊建筑-德国， 原版是奇观，这里当作建筑处理)

		AMPHITHEATER, // 露天剧场
		MUSEUM, // 博物馆
		BROADCAST_CENTER, // 广播中心

		LIGHTHOUSE, // 灯塔
		DOCKYARD, // 造船厂
		DOCKS, // 码头

		// 这里把航天中心的项目视为建筑
		LAUNCH_SATELLITE, // 发射卫星

		TO_BE_DEFINED
	};

	District(int playerID, Hex pos, DistrictType type, std::string name);
	~District();
	static bool isThereDistrictAt(Hex where);

	// 基类函数重写
	virtual void addProgress(int amount) override { updateProduction(amount); };


	bool canErectDistrict(Hex where);
	bool updateProduction(int productionYield);
	virtual void calculateBonus();
	void updateMaintenanceCost();
	void updateGrossYield();
	void updateCitizenBenefit();


	virtual bool addBuilding(std::string buildingName);

	inline int						getPlayerID() const { return playerID; }
	inline int						getDistrictID() const { return _id; }
	inline DistrictType				getType() const { return _type; }
	inline std::string				getName() { return _name; }
	inline Hex						getPos() const { return _pos; }

	inline int						getPrereqTech() const { return prereqTechID; }
	inline int						getPrereqCivic() const { return prereqCivicID; }
	inline std::vector<TerrainType> getPrereqTerrains() const { return prereqTerrains; }; // 建造前置地形
	inline int						getMaintenanceCost() const { return maintanenceCost; }
	inline Yield					getCitizenBenefit() const { return citizenBenefit; }
	inline Yield					getYield() const { return grossYield; }

	inline std::vector<Building*>   getBuildings() const { return buildings; }
protected:
	std::vector<Hex> getHexNeighbors(Hex center);
	// 计数所有的区域,便于编号
	static int count;
	static std::vector<Hex> districtPositions; // 已有区域的位置列表
	// 基础信息
	int playerID; // 区域所属玩家ID
	int _id; // 区域的唯一标志符
	DistrictType _type; // 区域类型
	std::string _name; // 区域名称

	// 归属相关
	Hex _pos; // 区域坐标
	 
	// 建造相关
	
	int prereqTechID; // 建造前置科技
	int prereqCivicID; // 建造前置市政
	std::vector<TerrainType> prereqTerrains; // 建造前置地形
	std::vector<BuildingCategory> possibleBuildings; // 包含在内的建筑
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
	Downtown(int player, Hex pos, std::string name);
	void completeDirectly() { // 建立新城市时直接完成市中心区的建造，修复已有的市中心才需要正常建造流程
		progress = cost;
		status = ProductionStatus::COMPLETED;
		updateGrossYield();
	}
	virtual void calculateBonus();
	cocos2d::Node* _downtownVisual;
};


#endif // __DISTRICT_H__
