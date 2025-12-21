#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "Scene/GameScene.h"
#include "City/Yield.h"
#include <string>


class Building {
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

	Building(BuildingType type);
	inline int			getBaseProductionCost() const { return baseProductionCost; }
	inline int			getBasePurchaseCost() const { return basePurchaseCost; }
	inline int			getMaintenanceCost() const { return maintenanceCost; }
	inline Yield		getCitizenBenefit() const { return citizenBenefit; }
	inline std::string	getPrereqTech() const { return prereqTech; }
	inline std::string	getPrereqCivic() const { return prereqCivic; }
	inline bool			getIsUnique() const { return isUnique; }
	inline BuildingType getType() const { return _type; }
	inline Yield		getYield() const { return yield; }

	virtual bool canErectBuilding(); // 检查是否能建造该建筑

private:
	int			baseProductionCost; // 建筑基础建造成本
	int			basePurchaseCost;	// 建筑基础购买成本
	int			maintenanceCost;	// 建筑维护费用
	Yield		citizenBenefit;		// 建筑市民加成
	std::string prereqTech;			// 前置科技
	std::string prereqCivic;		// 前置市政
	bool		isUnique;			// 是否为唯一建筑
	bool		hasCitizenBenefit;	// 是否有市民加成

	BuildingType _type;
	Yield yield; // 建筑产出
};

#endif // __BUILDING_H__