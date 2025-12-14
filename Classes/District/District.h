/*
* 区域类
*/

#ifndef __DISTRICT_H__
#define __DISTRICT_H__

#include <list> 
#include "City/BaseCity.h"
#include "Utils/HexUtils.h"
#include "Map/TileData.h"
#ifdef __BUILDING_H__ // Building 类尚未完成...
class Building;
#endif



class District {
public:
	enum class DistrictTypeName { // 区域类型枚举
		CAMPUS, // 学院
		INDUSTRY_ZONE, // 工业区
		COMMERCIAL_HUB, // 商业中心
		THEATER_SQUARE, // 剧院广场
		HARBOR, // 港口
		SPACEPORT, // 航天中心
		//... 军营
		TO_BE_DEFINED // 未定义
	};

	struct DistrictYield { // 区域产出结构体
		int foodYield; // 食物产出
		int productionYield; // 生产力产出
		int scienceYield; // 科技值产出
		int goldYield; // 金币产出
		int cultureYield; // 文化值产出

		DistrictYield operator+(const DistrictYield& other)
		{
			DistrictYield ret;
			ret.foodYield = foodYield + other.foodYield;
			ret.productionYield = productionYield + other.productionYield;
			ret.scienceYield = scienceYield + other.scienceYield;
			ret.goldYield = goldYield + other.goldYield;
			ret.cultureYield = cultureYield + other.cultureYield;
			return ret;
		}
	};

	struct DistrictType { // 区域类型结构体
		DistrictTypeName typeName;
		int cost;
		DistrictYield baseYield;
		TileData tileRequirement;
		int scienceRequirement;

		DistrictType() :
			typeName(DistrictTypeName::TO_BE_DEFINED), cost(0), baseYield({ 0,0,0,0,0 }), scienceRequirement(0)
		{
		};

		DistrictType(DistrictTypeName type) :
			typeName(type), cost(0), baseYield({ 0,0,0,0,0 }), tileRequirement(TileData()), scienceRequirement(0)
		{
			// 实际的cost比这个要复杂,需要全局考虑所有的同类区域
			switch (typeName)
			{
			case DistrictTypeName::SPACEPORT:
				cost = 360;
				break;
			default:
				cost = 60;
				break;
			}
			// (记得补充地形要求和科技要求)
			switch (typeName) // 后续加上地形需求
			{
			case DistrictTypeName::CAMPUS:
				baseYield.scienceYield = 1;
				break;
			case DistrictTypeName::COMMERCIAL_HUB:
				baseYield.goldYield = 1;
				break;
			case DistrictTypeName::INDUSTRY_ZONE:
				baseYield.productionYield = 1;
				break;
			case DistrictTypeName::THEATER_SQUARE:
				baseYield.cultureYield = 1;
				break;
			case DistrictTypeName::HARBOR:
				baseYield.goldYield = 1;
				break;
			default:
				break;
			}
		}
	};


	District(Hex pos, DistrictType type, std::string name, BaseCity* city);
	bool addProduction(int production);

	void calculateTurnsRemaining();
	void calculateAdjacencyBonus();

	void updateGrossYield();
#ifdef __BUILDING_H__
	bool addBuilding(Building addedBuilding);
#endif
	inline int getConstructionProgress() { return currentProgress; }

	inline int getID() { return _id; }
	inline DistrictType getType() { return _type; }
	inline std::string getName() { return _name; }

	inline Hex getPos() { return _pos; }
	inline BaseCity* getCity() { return _city; }

	inline DistrictYield getYield() { return grossYield; }

protected:

	bool canErectDistrict();

	// 计数所有的区域,便于编号
	static int count;

	// 基础信息
	int _id; // 区域的唯一标志符
	DistrictType _type; // 区域类型
	std::string _name; // 区域名称

	// 归属相关
	Hex _pos; // 区域坐标
	BaseCity* _city; // 区域所属城市
	 
	// 建造相关
	bool isConstructed; // 区域是否建成
	int productionCost; // 区域所耗生产力
	int currentProgress; // 区域建造进度
	int turnsRemaining; // 区域建成预计剩余回合数

	// 产出相关
	DistrictYield baseYield; // 区域基础产出 
	DistrictYield adjacencyBonus; // 区域相邻加成
	DistrictYield grossYield; // 区域总产出(前两项之和)

#ifdef __BUILDING_H__
	// 建筑
	std::list<Building*> buildings; // 区域容纳的建筑列表
#endif
};

int District::count = 0;

#endif // __DISTRICT_H__
