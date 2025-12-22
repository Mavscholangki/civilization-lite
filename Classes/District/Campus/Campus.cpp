#include "cocos2d.h"
#include "Scene/GameScene.h"
#include "District/Base/District.h"
#include "Campus.h"

USING_NS_CC;

int Campus::campusCount = 0;

Campus::Campus(Hex pos, std::string name)
	: District(pos, DistrictType(District::DistrictType::CAMPUS), name)
{
	isConstructed = false;
	productionCost = 54; // 校园区建造成本
	currentProgress = 0;
	turnsRemaining = productionCost; // 默认初始剩余回合数等于生产力成本
	prereqTech = u8"写作"; // 需要“写作”科技解锁
	baseMaintenanceCost = 1; // 基础维护费用
	updateMaintenanceCost();
	baseBenefit.scienceYield = 2; // 基础产出: +2科技
	updateCitizenBenefit();
	updateGrossYield();
	
	if (canErectDistrict(pos))
	{
		campusCount++;
	}

	// 绘制学院 (绿色六边形)
	auto draw = DrawNode::create();
	Vec2 vertices[6] = {
		Vec2(0, 20),
		Vec2(17.32f, 10),
		Vec2(17.32f, -10),
		Vec2(0, -20),
		Vec2(-17.32f, -10),
		Vec2(-17.32f, 10)
	};
	draw->drawSolidPoly(vertices, 6, Color4F::GREEN);
	// 加个白色轮廓
	draw->drawPoly(vertices, 6, false, Color4F::WHITE);
	_campusVisual = draw;
}

void Campus::calculateBonus()
{
	// 如果未建造完成,则无加成产出
	if (!isConstructed)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}

	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 校园区的加成产出计算逻辑
	// 例如：每个相邻的山脉地块增加1点科技产出
	auto gameScene = static_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return;
	std::vector<Hex> neighbors = getHexNeighbors(_pos);
	int mountainCount = 0;
	int jungleCount = 0;
	for (const auto& neighbor : neighbors)
	{
		TileData tileData = gameScene->getTileData(neighbor);
		if (tileData.type == TerrainType::MOUNTAIN)
		{
			mountainCount++;
		}
		else if (tileData.type == TerrainType::JUNGLE)
		{
			jungleCount++;
		}
	}
	adjacencyBonus.scienceYield += mountainCount * 1; // 每个山脉+1科技
	adjacencyBonus.scienceYield += jungleCount / 2; // 每两个雨林+1科技
	//// 每两个相邻区域+1点科技
	//int districtCount = 0;
	//for (const auto& neighbor : neighbors)
	//{
	//	// 相邻的每两个区域+1点科技
	//	District* adjacentDistrict = gameScene->getDistrictAtHex(neighbor);
	//	if (adjacentDistrict)
	//	{
	//		districtCount++;
	//	}
	//}
	//adjacencyBonus.scienceYield += (districtCount / 2); // 每两个相邻区域+1科技


	// 计算建筑加成
	for (auto building : buildings)
	{
		buildingBonus += building->getYield();
	}
	adjacencyBonus += buildingBonus;

	// 这里放置政策加成等其他加成计算
}

bool Campus::canErectDistrict(Hex where)
{
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return false;
	TileData tileData = gameScene->getTileData(where);
	// 校园区不能建在海洋、海岸和山脉上
	if (tileData.type != TerrainType::OCEAN && 
		tileData.type != TerrainType::COAST &&
		tileData.type != TerrainType::MOUNTAIN)
	{
		// if (prereqTech.empty() || gameScene->getPlayer()->isTechResearched(prereqTech))
			return true;
	}
	return false;
}

bool Campus::addBuilding(Building::BuildingType buildingType)
{
	if (buildingType != Building::BuildingType::LIBRARY &&
		buildingType != Building::BuildingType::UNIVERSITY &&
		buildingType != Building::BuildingType::LABORATORY)
	{
		return false; // 只能添加校园区相关建筑
	}
	if (!isConstructed)
		return false; // 未建成不能添加建筑

	// 创建建筑实例
	Building* newBuilding = new Building(buildingType);
	if (!newBuilding)
		return false; // 创建失败
	if (!newBuilding->canErectBuilding())
	{
		delete newBuilding;
		newBuilding = nullptr;
		return false; // 建筑前置条件不满足
	}
	buildings.push_back(newBuilding);
	// 建筑物可能会影响区域产出,调用更新函数
	updateGrossYield();
	return true;
}