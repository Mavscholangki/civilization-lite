#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "Scene/GameScene.h"
#include "City/Yield.h"
#include "Development/ProductionProgram.h"
#include "District/Base/District.h"
#include <string>


class Building : public ProductionProgram {
public:
	enum class BuildingType {
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

	Building(int player, BuildingType type);
	Building(int player, std::string name);
	~Building() {}
	inline int			getPlayerID() const { return playerID; }
	inline std::string	getDistrictName() const { return districtName; }
	inline int			getMaintenanceCost() const { return maintenanceCost; }
	inline Yield		getCitizenBenefit() const { return citizenBenefit; }
	inline int			getPrereqTechID() const { return prereqTechID; }
	inline int			getPrereqCivicID() const { return prereqCivicID; }
	inline bool			getIsUnique() const { return isUnique; }
	District::BuildingCategory getType() const;
	inline Yield		getYield() const { return yield; }

	virtual bool canErectBuilding(); // 检查是否能建造该建筑

private:
	int			playerID;			// 拥有该建筑的玩家ID
	std::string districtName;		// 所属区域名称
	int			maintenanceCost;	// 建筑维护费用
	Yield		citizenBenefit;		// 建筑市民加成
	int			prereqTechID;		// 前置科技
	int			prereqCivicID;		// 前置市政
	bool		isUnique;			// 是否为唯一建筑
	bool		hasCitizenBenefit;	// 是否有市民加成

	BuildingType _type;
	Yield yield; // 建筑产出
};

#endif // __BUILDING_H__