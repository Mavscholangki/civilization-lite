#include "Building.h"
#include "Development/TechSystem.h"
#include "Development/CultureSystem.h"
#include "Core/GameManager.h"
#include "District/Base/District.h"

// 构造函数，根据建筑类型初始化属性
Building::Building(int player, BuildingType type)
	: _type(type),
	ProductionProgram(ProductionType::BUILDING, "", Hex(), 0, true, 0), // 建筑可以用黄金购买完成
	playerID(player),
	districtName(""),
	maintenanceCost(0), 
	hasCitizenBenefit(false),
	citizenBenefit({ 0,0,0,0,0 }),
	prereqTechID(-1), // 默认无前置科技
	prereqCivicID(-1), // 默认无前置市政
	isUnique(false), // 默认非唯一建筑
	yield({ 0,0,0,0,0 })
{
	// 根据建筑类型初始化属性
	switch (type)
	{
	case BuildingType::PALACE:
		districtName = "Downtown";
		cost = 1;
		isUnique = true;
		yield.cultureYield = 1;
		yield.productionYield = 2;
		yield.goldYield = 5;
		yield.scienceYield = 2;
		break;
	case BuildingType::MONUMENT:
		districtName = "Downtown";
		cost = 60;
		purchaseCost = 240;
		isUnique = false;
		yield.cultureYield = 1; // +1文化
		break;
	case BuildingType::GRANARY:
		districtName = "Downtown";
		cost = 65;
		purchaseCost = 260;
		yield.foodYield = 1;
		break;
	case BuildingType::LIBRARY:
		districtName = "Campus";
		cost = 90;
		purchaseCost = 360;
		maintenanceCost = 1;
		prereqTechID = 4; // 前置科技：书写(ID 4)
		yield.scienceYield = 2; // +2科技
		break;
	case BuildingType::UNIVERSITY:
		districtName = "Campus";
		cost = 250;
		purchaseCost = 1000;
		maintenanceCost = 2;
		prereqTechID = 10; // 前置科技：数学(ID 10)
		yield.scienceYield = 4; // +4科技
		break;
	case BuildingType::LABORATORY:
		districtName = "Campus";
		cost = 440;
		purchaseCost = 1760;
		maintenanceCost = 3;
		prereqTechID = 14; // 前置科技：火药(ID 14)
		yield.scienceYield = 5; // +5科技
		break;
	case BuildingType::MARKET:
		districtName = "CommercialHub";
		cost = 120;
		purchaseCost = 480;
		prereqTechID = 6; // 前置科技：货币(ID 6)
		yield.goldYield = 2; // +2金币
		break;
	case BuildingType::BANK:
		districtName = "CommercialHub";
		cost = 290;
		purchaseCost = 1160;
		prereqTechID = 13; // 前置科技：银行(ID 13)
		yield.goldYield = 5; // +5金币
		break;
	case BuildingType::STOCK_EXCHANGE:
		districtName = "CommercialHub";
		cost = 330;
		purchaseCost = 1320;
		prereqTechID = 19; // 前置科技：经济学(ID 19)
		yield.goldYield = 7; // +7金币
		break;
	case BuildingType::WORKSHOP:
		districtName = "IndustryZone";
		cost = 195;
		purchaseCost = 780;
		maintenanceCost = 1;
		prereqTechID = 12; // 前置科技：学徒制(ID 12)
		yield.productionYield = 3; // +3生产力
		break;
	case BuildingType::FACTORY:
		districtName = "IndustryZone";
		cost = 330;
		purchaseCost = 1320;
		maintenanceCost = 2;
		prereqTechID = 17; // 前置科技：工业化(ID 17)
		yield.productionYield = 6; // +6生产力
		break;
	case BuildingType::POWER_PLANT:
		districtName = "IndustryZone";
		cost = 300;
		purchaseCost = 1200;
		maintenanceCost = 3;
		prereqTechID = 17; // 前置科技：工业化(ID 17)
		citizenBenefit.productionYield = 1; // +1生产力给每位市民
		break;
	case BuildingType::RHUR_VALLEY:
		districtName = "IndustryZone";
		cost = 1240;
		canPurchase = false; //	不能用黄金购买
		isUnique = true;
		prereqTechID = 17; // 前置科技：工业化(ID 17)
		yield.productionYield = 8; // +8生产力
		break;
	case BuildingType::AMPHITHEATER:
		districtName = "TheaterSquare";
		cost = 150;
		purchaseCost = 600;
		maintenanceCost = 1;
		prereqCivicID = 102; // 前置市政：技艺(ID 102)
		yield.cultureYield = 2; // +2文化
		break;
	case BuildingType::MUSEUM:
		districtName = "TheaterSquare";
		cost = 290;
		purchaseCost = 1160;
		maintenanceCost = 2;
		prereqCivicID = 107; // 前置市政：人文主义(ID 107)
		yield.cultureYield = 2; // +2文化
		break;
	case BuildingType::BROADCAST_CENTER:
		districtName = "TheaterSquare";
		cost = 440;
		purchaseCost = 1760;
		maintenanceCost = 3;
		prereqCivicID = 109; // 前置市政：意识形态(ID 109)
		yield.cultureYield = 4; // +4文化
		citizenBenefit.cultureYield = 1; // +1文化给每位市民
		break;
	case BuildingType::LIGHTHOUSE:
		districtName = "Harbor";
		cost = 120;
		purchaseCost = 480;
		prereqTechID = 5; // 前置科技：航海(ID 5)
		yield.foodYield = 1; // +1食物
		break;
	case BuildingType::DOCKYARD:
		districtName = "Harbor";
		cost = 290;
		purchaseCost = 1160;
		maintenanceCost = 1;
		prereqTechID = 9; // 前置科技：造船术(ID 9)
		yield.foodYield = 2; // +2食物
		break;
	case BuildingType::DOCKS:
		districtName = "Harbor";
		cost = 440;
		purchaseCost = 1760;
		prereqTechID = 16; // 前置科技：制图学(ID 16)
		yield.foodYield = 3; // +3食物
		yield.goldYield = 2; // +2金币
		citizenBenefit.foodYield = 1; // +1食物给每位市民
		break;
	case BuildingType::LAUNCH_SATELLITE:
		districtName = "Spaceport";
		cost = 1000;
		purchaseCost = 4000;
		isUnique = true;
		prereqTechID = 21; // 前置科技：火箭学(ID 21)
		yield.scienceYield = 10; // +10科技
		break;
	default:
		break;
	}
}
Building::Building(int player, std::string name)
	:_type(BuildingType::TO_BE_DEFINED),
	ProductionProgram(ProductionType::BUILDING, "", Hex(), 0, true, 0), // 建筑可以用黄金购买完成
	playerID(player),
	districtName(""),
	maintenanceCost(0),
	hasCitizenBenefit(false),
	citizenBenefit({ 0,0,0,0,0 }),
	prereqTechID(-1), // 默认无前置科技
	prereqCivicID(-1), // 默认无前置市政
	isUnique(false), // 默认非唯一建筑
	yield({ 0,0,0,0,0 })
{
	// 根据建筑类型初始化属性
	if (name == "Palace") {
		_type = BuildingType::PALACE;
		districtName = "Downtown";
		cost = 1;
		isUnique = true;
		yield.cultureYield = 1;
		yield.productionYield = 2;
		yield.goldYield = 5;
		yield.scienceYield = 2;
	}
	else if (name == "Monument") {
		_type = BuildingType::MONUMENT;
		districtName = "Downtown";
		cost = 60;
		purchaseCost = 240;
		isUnique = false;
		yield.cultureYield = 1; // +1文化
	}
	else if (name == "Granary") {
		_type = BuildingType::GRANARY;
		districtName = "Downtown";
		cost = 65;
		purchaseCost = 260;
		yield.foodYield = 1;
	}
	else if (name == "Library") {
		_type = BuildingType::LIBRARY;
		districtName = "Campus";
		cost = 90;
		purchaseCost = 360;
		maintenanceCost = 1;
		prereqTechID = 4; // 前置科技：书写(ID 4)
		yield.scienceYield = 2; // +2科技
	}
	else if (name == "University") {
		_type = BuildingType::UNIVERSITY;
		districtName = "Campus";
		cost = 250;
		purchaseCost = 1000;
		maintenanceCost = 2;
		prereqTechID = 10; // 前置科技：数学(ID 10)
		yield.scienceYield = 4; // +4科技
	}
	else if (name == "Laboratory") {
		_type = BuildingType::LABORATORY;
		districtName = "Campus";
		cost = 440;
		purchaseCost = 1760;
		maintenanceCost = 3;
		prereqTechID = 14; // 前置科技：火药(ID 14)
		yield.scienceYield = 5; // +5科技
	}
	else if (name == "Market") {
		_type = BuildingType::MARKET;
		districtName = "CommercialHub";
		cost = 120;
		purchaseCost = 480;
		prereqTechID = 6; // 前置科技：货币(ID 6)
		yield.goldYield = 2; // +2金币
	}
	else if (name == "Bank") {
		_type = BuildingType::BANK;
		districtName = "CommercialHub";
		cost = 290;
		purchaseCost = 1160;
		prereqTechID = 13; // 前置科技：银行(ID 13)
		yield.goldYield = 5; // +5金币
	}
	else if (name == "StockExchange") {
		_type = BuildingType::STOCK_EXCHANGE;
		districtName = "CommercialHub";
		cost = 330;
		purchaseCost = 1320;
		prereqTechID = 19; // 前置科技：经济学(ID 19)
		yield.goldYield = 7; // +7金币
	}
	else if (name == "Workshop") {
		_type = BuildingType::WORKSHOP;
		districtName = "IndustryZone";
		cost = 195;
		purchaseCost = 780;
		maintenanceCost = 1;
		prereqTechID = 12; // 前置科技：学徒制(ID 12)
		yield.productionYield = 3; // +3生产力
	}
	else if (name == "Factory") {
		_type = BuildingType::FACTORY;
		districtName = "IndustryZone";
		cost = 330;
		purchaseCost = 1320;
		maintenanceCost = 2;
		prereqTechID = 17; // 前置科技：工业化(ID 17)
		yield.productionYield = 6; // +6生产力
	}
	else if (name == "PowerPlant") {
		_type = BuildingType::POWER_PLANT;
		districtName = "IndustryZone";
		cost = 300;
		purchaseCost = 1200;
		maintenanceCost = 3;
		prereqTechID = 17; // 前置科技：工业化(ID 17)
		citizenBenefit.productionYield = 1; // +1生产力给每位市民
	}
	else if (name == "RhurValley") {
		_type = BuildingType::RHUR_VALLEY;
		districtName = "IndustryZone";
		cost = 1240;
		canPurchase = false; // 不能用黄金购买
		isUnique = true;
		prereqTechID = 17; // 前置科技：工业化(ID 17)
		yield.productionYield = 8; // +8生产力
	}
	else if (name == "Amphitheater") {
		_type = BuildingType::AMPHITHEATER;
		districtName = "TheaterSquare";
		cost = 150;
		purchaseCost = 600;
		maintenanceCost = 1;
		prereqCivicID = 102; // 前置市政：技艺(ID 102)
		yield.cultureYield = 2; // +2文化
	}
	else if (name == "Museum") {
		_type = BuildingType::MUSEUM;
		districtName = "TheaterSquare";
		cost = 290;
		purchaseCost = 1160;
		maintenanceCost = 2;
		prereqCivicID = 107; // 前置市政：人文主义(ID 107)
		yield.cultureYield = 2; // +2文化
	}
	else if (name == "BroadcastCenter") {
		_type = BuildingType::BROADCAST_CENTER;
		districtName = "TheaterSquare";
		cost = 440;
		purchaseCost = 1760;
		maintenanceCost = 3;
		prereqCivicID = 109; // 前置市政：意识形态(ID 109)
		yield.cultureYield = 4; // +4文化
		citizenBenefit.cultureYield = 1; // +1文化给每位市民
	}
	else if (name == "Lighthouse") {
		_type = BuildingType::LIGHTHOUSE;
		districtName = "Harbor";
		cost = 120;
		purchaseCost = 480;
		prereqTechID = 5; // 前置科技：航海(ID 5)
		yield.foodYield = 1; // +1食物
	}
	else if (name == "Dockyard") {
		_type = BuildingType::DOCKYARD;
		districtName = "Harbor";
		cost = 290;
		purchaseCost = 1160;
		maintenanceCost = 1;
		prereqTechID = 9; // 前置科技：造船术(ID 9)
		yield.foodYield = 2; // +2食物
	}
	else if (name == "Docks") {
		_type = BuildingType::DOCKS;
		districtName = "Harbor";
		cost = 440;
		purchaseCost = 1760;
		prereqTechID = 16; // 前置科技：制图学(ID 16)
		yield.foodYield = 3; // +3食物
		yield.goldYield = 2; // +2金币
		citizenBenefit.foodYield = 1; // +1食物给每位市民
	}
	else if (name == "LaunchSatellite") {
		_type = BuildingType::LAUNCH_SATELLITE;
		districtName = "Spaceport";
		cost = 1000;
		purchaseCost = 4000;
		isUnique = true;
		prereqTechID = 21; // 前置科技：火箭学(ID 21)
		yield.scienceYield = 10; // +10科技
	}
	else {
		// 默认情况，处理未知建筑名称
		_type = BuildingType::TO_BE_DEFINED;
	}
}
District::BuildingCategory convertFromBuildingType(Building::BuildingType type) {
	switch (type) {
	case Building::BuildingType::PALACE:           return District::BuildingCategory::PALACE;
	case Building::BuildingType::MONUMENT:         return District::BuildingCategory::MONUMENT;
	case Building::BuildingType::GRANARY:          return District::BuildingCategory::GRANARY;
	case Building::BuildingType::LIBRARY:          return District::BuildingCategory::LIBRARY;
	case Building::BuildingType::UNIVERSITY:       return District::BuildingCategory::UNIVERSITY;
	case Building::BuildingType::LABORATORY:       return District::BuildingCategory::LABORATORY;
	case Building::BuildingType::MARKET:           return District::BuildingCategory::MARKET;
	case Building::BuildingType::BANK:             return District::BuildingCategory::BANK;
	case Building::BuildingType::STOCK_EXCHANGE:   return District::BuildingCategory::STOCK_EXCHANGE;
	case Building::BuildingType::WORKSHOP:         return District::BuildingCategory::WORKSHOP;
	case Building::BuildingType::FACTORY:          return District::BuildingCategory::FACTORY;
	case Building::BuildingType::POWER_PLANT:      return District::BuildingCategory::POWER_PLANT;
	case Building::BuildingType::RHUR_VALLEY:      return District::BuildingCategory::RHUR_VALLEY;
	case Building::BuildingType::AMPHITHEATER:     return District::BuildingCategory::AMPHITHEATER;
	case Building::BuildingType::MUSEUM:           return District::BuildingCategory::MUSEUM;
	case Building::BuildingType::BROADCAST_CENTER: return District::BuildingCategory::BROADCAST_CENTER;
	case Building::BuildingType::LIGHTHOUSE:       return District::BuildingCategory::LIGHTHOUSE;
	case Building::BuildingType::DOCKYARD:         return District::BuildingCategory::DOCKYARD;
	case Building::BuildingType::DOCKS:            return District::BuildingCategory::DOCKS;
	case Building::BuildingType::LAUNCH_SATELLITE: return District::BuildingCategory::LAUNCH_SATELLITE;
	default:                                       return District::BuildingCategory::TO_BE_DEFINED;
	}
}

District::BuildingCategory Building::getType() const { return convertFromBuildingType(_type); }


// 检查是否可以建造该建筑
bool Building::canErectBuilding()
{
	// 前置建筑检查在District类的addBuilding中处理
	// 检查前置科技和市政
	if ((prereqTechID == -1 || GameManager::getInstance()->getPlayer(playerID)->getTechTree()->isActivated(prereqTechID)) &&
		(prereqCivicID == -1 || GameManager::getInstance()->getPlayer(playerID)->getTechTree()->isActivated(prereqCivicID)))
		return true;

	return false;
}