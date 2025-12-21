#include "cocos2d.h"
#include "District.h"
#include "Scene/GameScene.h"

USING_NS_CC;


int District::count = 0;

District::District(Hex pos, DistrictType type, std::string name):
	_id(count++),
	_pos(pos),
	_name(name),
	_type(type),
	isConstructed(false),
	productionCost(0),
	currentProgress(0),
	turnsRemaining(0),
	prereqTech(""),
	prereqCivic(""),
	baseMaintenanceCost(0),
	buildingMaintenanceCost(0),
	maintanenceCost(0),
	buildingBonus({ 0,0,0,0,0 }),
	adjacencyBonus({ 0,0,0,0,0 }),
	grossYield({ 0,0,0,0,0 }),
	baseBenefit({ 0,0,0,0,0 }),
	buildingBenefit({ 0,0,0,0,0 }),
	citizenBenefit({ 0,0,0,0,0 })
{}

// 提供一定的生产力后,更新建造状态,返回建造是否成功
bool District::updateProduction(int productionYield)
{
	if (productionYield <= 0)
		return isConstructed; // 无效生产力输入
	currentProgress += productionYield;
	if (currentProgress >= productionCost) // 建造完成
	{
		turnsRemaining = 0;
		isConstructed = true;
		currentProgress = productionCost;
		updateGrossYield(); // 更新区域产出
		return true;
	}
	else
	{
		turnsRemaining = (productionCost - currentProgress + productionYield - 1) / productionYield; // 向上取整
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

bool District::addBuilding(Building::BuildingType buildingType)
{
	// 创建建筑实例
	Building* newBuilding = new Building(buildingType);
	if (!newBuilding)
		return false; // 创建失败
	// 简单示例: 直接添加建筑物
	buildings.push_back(newBuilding);
	// 建筑物可能会影响区域产出,调用更新函数
	updateGrossYield();
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
	// 校园区不能建在海洋、海岸和山脉上
	if (tileData.type != TerrainType::OCEAN &&
		tileData.type != TerrainType::COAST &&
		tileData.type != TerrainType::MOUNTAIN)
	{
		if (prereqTech.empty() || gameScene->getPlayer()->isTechResearched(prereqTech))
			return true;
	}
	return false;
}

// ==================== 市中心区 ====================

Downtown::Downtown(Hex pos, std::string name)
	: District(pos, DistrictType(District::DistrictType::DOWNTOWN), name)
{
	isConstructed = false;
	productionCost  = 0; // 市中心区建造成本为0
	currentProgress = 0;
	turnsRemaining  = 0; // 默认初始剩余回合数为0
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
	if (!isConstructed)
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

bool Downtown::canErectDistrict(Hex where)
{
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return false;
	TileData tileData = gameScene->getTileData(where);
	// 市中心不能建在海洋、海岸和山脉上
	if (tileData.type != TerrainType::OCEAN &&
		tileData.type != TerrainType::COAST &&
		tileData.type != TerrainType::MOUNTAIN)
	{
		return true;
	}
	return false;
}

bool Downtown::addBuilding(Building::BuildingType buildingType)
{
	if (buildingType != Building::BuildingType::MONUMENT &&
		buildingType != Building::BuildingType::GRANARY &&
		buildingType != Building::BuildingType::PALACE)
	{
		return false; // 市中心区只能建造特定建筑
	}
	// 创建建筑实例
	Building* newBuilding = new Building(buildingType);
	if (!newBuilding)
		return false; // 创建失败
	if (!newBuilding->canErectBuilding())
	{
		delete newBuilding;
		newBuilding = nullptr;
		return false; // 建造条件不满足
	}
	buildings.push_back(newBuilding);
	// 建筑物可能会影响区域产出,调用更新函数
	updateGrossYield();
	return true;
}