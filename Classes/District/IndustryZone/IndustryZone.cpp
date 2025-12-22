#include "IndustryZone.h"

USING_NS_CC;
int IndustryZone::industryZoneCount = 0;

IndustryZone::IndustryZone(Hex pos, std::string name)
	: District(pos, DistrictType(District::DistrictType::INDUSTRY_ZONE), name)
{
	isConstructed = false;
	productionCost = 110; // 工业区建造成本
	currentProgress = 0;
	turnsRemaining = productionCost; // 默认初始剩余回合数等于生产力成本
	prereqTech = u8"工艺"; // 需要“工艺”科技解锁
	baseMaintenanceCost = 1; // 基础维护费用
	updateMaintenanceCost();
	baseBenefit.productionYield = 2; // 基础产出: +2生产力
	updateCitizenBenefit();
	updateGrossYield();
	if (canErectDistrict(pos))
	{
		industryZoneCount++;
	}
	// 绘制工业区 (灰色六边形)
	auto draw = DrawNode::create();
	Vec2 vertices[6] = {
		Vec2(0, 20),
		Vec2(17.32f, 10),
		Vec2(17.32f, -10),
		Vec2(0, -20),
		Vec2(-17.32f, -10),
		Vec2(-17.32f, 10)
	};
	draw->drawSolidPoly(vertices, 6, Color4F::GRAY);
	// 加个白色轮廓
	draw->drawPoly(vertices, 6, false, Color4F::WHITE);
	_industryZoneVisual = draw;
}

void IndustryZone::calculateBonus()
{
	// 如果未建造完成,则无加成产出
	if (!isConstructed)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 工业区的加成产出计算逻辑
	// 例如：每个相邻的森林地块增加1点生产力产出
	//auto gameScene = static_cast<GameScene*>(Director::getInstance()->getRunningScene());
	//if (!gameScene) return;
	//std::vector<Hex> neighbors = getHexNeighbors(_pos);
	//for (const auto& neighbor : neighbors)
	//{
	//	// 相邻的每个区域+1点生产力
	//	District* adjacentDistrict = gameScene->getDistrictAtHex(neighbor);
	//	if (adjacentDistrict && adjacentDistrict->getType() == District::DistrictType::THEATER_SQUARE)
	//	{
	//		adjacencyBonus.productionYield += 1;
	//	}
	//}
}

bool IndustryZone::canErectDistrict(Hex where)
{
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return false;
	TileData tileData = gameScene->getTileData(where);
	// 工业区不能建在海洋、海岸和山脉上
	if (tileData.type != TerrainType::OCEAN &&
		tileData.type != TerrainType::COAST &&
		tileData.type != TerrainType::MOUNTAIN)
	{
		// if (prereqTech.empty() || gameScene->getPlayer()->isTechResearched(prereqTech))
			return true;
	}
	return false;
}

bool IndustryZone::addBuilding(Building::BuildingType buildingType)
{
	if (buildingType != Building::BuildingType::WORKSHOP &&
		buildingType != Building::BuildingType::FACTORY &&
		buildingType != Building::BuildingType::POWER_PLANT)
	{
		return false; // 工业区只能建造特定建筑
	}
	if (!isConstructed)
	{
		return false; // 未建造完成不能添加建筑
	}

	// 创建建筑实例
	Building* newBuilding = new Building(buildingType);
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