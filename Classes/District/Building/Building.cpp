#include "Building.h"
#include "Development/TechSystem.h"
#include "Development/CultureSystem.h"

// 构造函数，根据建筑类型初始化属性
Building::Building(BuildingType type)
	: _type(type), 
	baseProductionCost(0),
	basePurchaseCost(0),
	maintenanceCost(0), 
	citizenBenefit({ 0,0,0,0,0 }),
	prereqTech(""), // 默认无前置科技
	prereqCivic(""), // 默认无前置市政
	isUnique(false), // 默认非唯一建筑
	yield({ 0,0,0,0,0 })
{
	// 根据建筑类型初始化属性
	switch (type)
	{
	case BuildingType::PALACE:
		baseProductionCost = 1;
		isUnique = true;
		yield.cultureYield = 1;
		yield.productionYield = 2;
		yield.goldYield = 5;
		yield.scienceYield = 2;
		break;
	case BuildingType::MONUMENT:
		baseProductionCost = 60;
		basePurchaseCost = 240;
		isUnique = false;
		yield.cultureYield = 1; // +1文化
		break;
	case BuildingType::GRANARY:
		baseProductionCost = 65;
		basePurchaseCost = 260;
		yield.foodYield = 1;
		break;
	case BuildingType::LIBRARY:
		baseProductionCost = 90;
		basePurchaseCost = 360;
		maintenanceCost = 1;
		prereqTech = u8"书写"; // 前置科技：书写
		yield.scienceYield = 2; // +2科技
		break;
	case BuildingType::UNIVERSITY:
		baseProductionCost = 250;
		basePurchaseCost = 1000;
		maintenanceCost = 2;
		prereqTech = u8"数学"; // 前置科技：数学
		yield.scienceYield = 4; // +4科技
		break;
	case BuildingType::LABORATORY:
		baseProductionCost = 440;
		basePurchaseCost = 1760;
		maintenanceCost = 3;
		prereqTech = u8"化学"; // 前置科技：化学
		yield.scienceYield = 5; // +5科技
		break;
	case BuildingType::MARKET:
		baseProductionCost = 120;
		basePurchaseCost = 480;
		prereqTech = u8"货币"; // 前置科技：货币
		yield.goldYield = 2; // +2金币
		break;
	case BuildingType::BANK:
		baseProductionCost = 290;
		basePurchaseCost = 1160;
		prereqTech = u8"银行"; // 前置科技：银行
		yield.goldYield = 5; // +5金币
		break;
	case BuildingType::STOCK_EXCHANGE:
		baseProductionCost = 330;
		basePurchaseCost = 1320;
		prereqTech = u8"经济学"; // 前置科技：经济学
		yield.goldYield = 7; // +7金币
		break;
	case BuildingType::WORKSHOP:
		baseProductionCost = 195;
		basePurchaseCost = 780;
		maintenanceCost = 1;
		prereqTech = u8"学徒制"; // 前置科技：学徒制
		yield.productionYield = 3; // +3生产力
		break;
	case BuildingType::FACTORY:
		baseProductionCost = 330;
		basePurchaseCost = 1320;
		maintenanceCost = 2;
		prereqTech = u8"工业化"; // 前置科技：工业化
		yield.productionYield = 6; // +6生产力
		break;
	case BuildingType::POWER_PLANT:
		baseProductionCost = 300;
		basePurchaseCost = 1200;
		maintenanceCost = 3;
		prereqTech = u8"工业化"; // 前置科技：工业化
		citizenBenefit.productionYield = 1; // +1生产力给每位市民
		break;
	case BuildingType::RHUR_VALLEY:
		baseProductionCost = 1240;
		basePurchaseCost = INT_MAX; // 不可购买
		isUnique = true;
		prereqTech = u8"工业化"; // 前置科技：工业化
		yield.productionYield = 8; // +8生产力
		break;
	case BuildingType::AMPHITHEATER:
		baseProductionCost = 150;
		basePurchaseCost = 600;
		maintenanceCost = 1;
		prereqCivic = u8"戏剧与诗歌"; // 前置市政：戏剧与诗歌
		yield.cultureYield = 2; // +2文化
		break;
	case BuildingType::MUSEUM:
		baseProductionCost = 290;
		basePurchaseCost = 1160;
		maintenanceCost = 2;
		prereqCivic = u8"人文主义"; // 前置市政：人文主义
		yield.cultureYield = 2; // +2文化
		break;
	case BuildingType::BROADCAST_CENTER:
		baseProductionCost = 440;
		basePurchaseCost = 1760;
		maintenanceCost = 3;
		prereqCivic = u8"无线电"; // 前置市政：无线电
		yield.cultureYield = 4; // +4文化
		citizenBenefit.cultureYield = 1; // +1文化给每位市民
		break;
	case BuildingType::LIGHTHOUSE:
		baseProductionCost = 120;
		basePurchaseCost = 480;
		prereqTech = u8"天文导航"; // 前置科技：天文导航
		yield.foodYield = 1; // +1食物
		break;
	case BuildingType::DOCKYARD:
		baseProductionCost = 290;
		basePurchaseCost = 1160;
		maintenanceCost = 1;
		prereqTech = u8"批量生产"; // 前置科技：造船术
		yield.foodYield = 2; // +2食物
		break;
	case BuildingType::DOCKS:
		baseProductionCost = 440;
		basePurchaseCost = 1760;
		prereqTech = u8"电力"; // 前置科技：造船术
		yield.foodYield = 3; // +3食物
		yield.goldYield = 2; // +2金币
		citizenBenefit.foodYield = 1; // +1食物给每位市民
		break;
	case BuildingType::LAUNCH_SATELLITE:
		baseProductionCost = 1000;
		basePurchaseCost = 4000;
		isUnique = true;
		prereqTech = u8"火箭学"; // 前置科技：火箭学
		yield.scienceYield = 10; // +10科技
		break;
	default:
		break;
	}
}

// 获取建筑的邻接加成
Yield Building::getAdjacencyBonus()
{
	// 默认没有邻接加成
	return { 0,0,0,0,0 };
}

// 检查是否可以建造该建筑
bool Building::canErectBuilding()
{
	// 检查前置科技
	if (!prereqTech.empty() && !TechSystem::getInstance().isTechResearched(prereqTech))
	{
		return false;
	}
	// 检查前置市政
	if (!prereqCivic.empty() && !CultureSystem::getInstance().isCivicUnlocked(prereqCivic))
	{
		return false;
	}
	return true;
}