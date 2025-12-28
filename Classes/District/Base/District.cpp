#include "cocos2d.h"
#include "District.h"
#include "District/Building/Building.h"
#include "Scene/GameScene.h"
#include "Core/GameManager.h"
USING_NS_CC;


int District::count = 0;
std::vector<Hex> District::districtPositions(0);

bool District::isThereDistrictAt(Hex where)
{
	for (const auto& pos : districtPositions)
	{
		if (pos == where)
			return true;
	}
	return false;
}



District::District(int player, Hex pos, DistrictType type, std::string name):
	ProductionProgram(ProductionType::DISTRICT, name, pos, 0, false), // 区域不可以用黄金购买完成
	playerID(player),
	_id(count++),
	_pos(pos),
	_name(name),
	_type(type),
	prereqTechID(-1),
	prereqCivicID(-1),
	prereqTerrains({
		TerrainType::DESERT,
		TerrainType::GRASSLAND,
		TerrainType::JUNGLE,
		TerrainType::PLAINS,
		TerrainType::SNOW,
		TerrainType::TUNDRA
	}),
	possibleBuildings(),
	baseMaintenanceCost(0),
	buildingMaintenanceCost(0),
	maintanenceCost(0),
	buildingBonus({ 0,0,0,0,0 }),
	adjacencyBonus({ 0,0,0,0,0 }),
	grossYield({ 0,0,0,0,0 }),
	baseBenefit({ 0,0,0,0,0 }),
	buildingBenefit({ 0,0,0,0,0 }),
	citizenBenefit({ 0,0,0,0,0 })
{
	// 记录区域位置
	districtPositions.push_back(pos);
	if (name == "Downtown")
	{
		_type = District::DistrictType::DOWNTOWN;
		cost = 54;
	}
	else if (name == "Campus")
	{
		_type = District::DistrictType::CAMPUS;
		cost = 54;
	}
	else if (name == "IndustryZone")
	{
		_type = District::DistrictType::INDUSTRY_ZONE;
		cost = 54;
	}
	else if (name == "CommercialHub")
	{
		_type = District::DistrictType::COMMERCIAL_HUB;
		cost = 110;
	}
	else if (name == "TheaterSquare")
	{
		_type = District::DistrictType::THEATER_SQUARE;
		cost = 54;
	}
	else if (name == "Harbor")
	{
		_type = District::DistrictType::HARBOR;
		cost = 54;
	}
	else if (name == "Spaceport")
	{
		_type = District::DistrictType::SPACEPORT;
		cost = 1800;
	}
}

District::~District()
{
	// 释放建筑对象
	for (auto building : buildings) {
		delete building;
	}
}

// 提供一定的生产力后,更新建造状态,返回建造是否成功
bool District::updateProduction(int productionYield)
{
	if (productionYield <= 0)
		return status == ProductionStatus::COMPLETED; // 无效生产力输入
	progress += productionYield;
	if (progress >= cost) // 建造完成
	{
		turnsRemaining = 0;
		status = ProductionStatus::COMPLETED;
		progress = cost;
		updateGrossYield(); // 更新区域产出
		return true;
	}
	else
	{
		turnsRemaining = (cost - progress + productionYield - 1) / productionYield; // 向上取整
		return false;
	}
}

// 计算区域加成产出
void District::calculateBonus()
{
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	adjacencyBonus += buildingBonus;
}

void District::updateMaintenanceCost()
{
	// 计算建筑维护费用
	buildingMaintenanceCost = 0;
	for (auto building : buildings)
	{
		buildingMaintenanceCost += building->getMaintenanceCost();
	}
	maintanenceCost = baseMaintenanceCost + buildingMaintenanceCost;
}

// 区域总产出更新(后续可能会加上对周围6地块的状态监听/周围地块状态改变时调用)
void District::updateGrossYield()
{
	calculateBonus(); // 更新加成产出
	grossYield = buildingBonus + adjacencyBonus;
}

void District::updateCitizenBenefit()
{
	buildingBenefit = { 0,0,0,0,0 };
	for (auto building : buildings)
	{
		buildingBenefit += building->getCitizenBenefit();
	}
	citizenBenefit = baseBenefit + buildingBenefit;
}

Building::BuildingType convertToBuildingType(District::BuildingCategory category) {
	switch (category) {
	case District::BuildingCategory::PALACE:           return Building::BuildingType::PALACE;
	case District::BuildingCategory::MONUMENT:         return Building::BuildingType::MONUMENT;
	case District::BuildingCategory::GRANARY:          return Building::BuildingType::GRANARY;
	case District::BuildingCategory::LIBRARY:          return Building::BuildingType::LIBRARY;
	case District::BuildingCategory::UNIVERSITY:       return Building::BuildingType::UNIVERSITY;
	case District::BuildingCategory::LABORATORY:       return Building::BuildingType::LABORATORY;
	case District::BuildingCategory::MARKET:           return Building::BuildingType::MARKET;
	case District::BuildingCategory::BANK:             return Building::BuildingType::BANK;
	case District::BuildingCategory::STOCK_EXCHANGE:   return Building::BuildingType::STOCK_EXCHANGE;
	case District::BuildingCategory::WORKSHOP:         return Building::BuildingType::WORKSHOP;
	case District::BuildingCategory::FACTORY:          return Building::BuildingType::FACTORY;
	case District::BuildingCategory::POWER_PLANT:      return Building::BuildingType::POWER_PLANT;
	case District::BuildingCategory::RHUR_VALLEY:      return Building::BuildingType::RHUR_VALLEY;
	case District::BuildingCategory::AMPHITHEATER:     return Building::BuildingType::AMPHITHEATER;
	case District::BuildingCategory::MUSEUM:           return Building::BuildingType::MUSEUM;
	case District::BuildingCategory::BROADCAST_CENTER: return Building::BuildingType::BROADCAST_CENTER;
	case District::BuildingCategory::LIGHTHOUSE:       return Building::BuildingType::LIGHTHOUSE;
	case District::BuildingCategory::DOCKYARD:         return Building::BuildingType::DOCKYARD;
	case District::BuildingCategory::DOCKS:            return Building::BuildingType::DOCKS;
	case District::BuildingCategory::LAUNCH_SATELLITE: return Building::BuildingType::LAUNCH_SATELLITE;
	default:                                 return Building::BuildingType::TO_BE_DEFINED;
	}
}

bool District::addBuilding(std::string buildingName)
{
	bool hasThisBuilding = false;
	Building* newBuilding = new Building(this->playerID, buildingName);
	for (auto possibleBuilding : possibleBuildings)
	{
		if (possibleBuilding == newBuilding->getType())
		{
			hasThisBuilding = true;
			break;
		}
	}
	if (!hasThisBuilding)
	{
		delete newBuilding;
		return false;
	}
	// 创建建筑实例
	if (!newBuilding)
		return false; // 创建失败
	if (!newBuilding->canErectBuilding())
	{
		delete newBuilding;
		return false; // 不满足建造条件
	}
	buildings.push_back(newBuilding);
	updateGrossYield(); // 建筑物可能会影响区域产出,调用更新函数
	return true;
}

std::vector<Hex> District::getHexNeighbors(Hex center)
{
	std::vector<Hex> neighbors;
	std::vector<Hex> directions = {
		Hex(1, 0), Hex(1, -1), Hex(0, -1),
		Hex(-1, 0), Hex(-1, 1), Hex(0, 1)
	};
	for (const auto& dir : directions)
	{
		neighbors.push_back(Hex(center.q + dir.q, center.r + dir.r));
	}
	return neighbors;
}

bool District::canErectDistrict(Hex where)
{
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return false;
	TileData tileData = gameScene->getTileData(where);
	bool tileMatch = false;
	for (auto terrain : prereqTerrains)
	{
		if (tileData.type == terrain)
		{
			tileMatch = true;
			break;
		}
	}
	// 校园区不能建在海洋、海岸和山脉上
	if (tileMatch)
	{
		if ((prereqTechID == -1 || GameManager::getInstance()->getPlayer(playerID)->getTechTree()->isActivated(prereqTechID)) &&
			(prereqCivicID == -1 || GameManager::getInstance()->getPlayer(playerID)->getTechTree()->isActivated(prereqCivicID)))
			return true;
	}
	return false;
}

// ==================== 市中心区 ====================

Downtown::Downtown(int player, Hex pos, std::string name)
	: District(player, pos, DistrictType(District::DistrictType::DOWNTOWN), name)
{
	cost = 54; // 市中心区建造成本为54
	progress = 0;
	turnsRemaining = cost; // 默认初始剩余回合数为54
	possibleBuildings = std::vector<BuildingCategory>({
		BuildingCategory::MONUMENT,
		BuildingCategory::GRANARY,
		BuildingCategory::PALACE 
	});
	updateGrossYield();

	// 绘制城市 (蓝色方块)
	auto draw = DrawNode::create();
	draw->drawSolidRect(Vec2(-15, -15), Vec2(15, 15), Color4F::BLUE);
	// 加个城墙轮廓
	draw->drawRect(Vec2(-15, -15), Vec2(15, 15), Color4F::WHITE);
	_downtownVisual = draw;
}
void Downtown::calculateBonus()
{
	// 如果未建造完成,则无加成产出
	if (status != ProductionStatus::COMPLETED)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出

	// 不考虑相邻地块的加成

	// 计算建筑加成
	for (auto building : buildings)
	{
		buildingBonus += building->getYield();
	}
	adjacencyBonus += buildingBonus;
	// 这里放置政策加成等其他加成计算
}