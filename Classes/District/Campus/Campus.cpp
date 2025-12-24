#include "cocos2d.h"
#include "Scene/GameScene.h"
#include "District/Base/District.h"
#include "District/Building/Building.h"
#include "Campus.h"

USING_NS_CC;

int Campus::campusCount = 0;

Campus::Campus(int player, Hex pos, std::string name): 
	District(player, pos, DistrictType(District::DistrictType::CAMPUS), name)
{
	cost = 54; // 校园区建造成本
	progress = 0;
	turnsRemaining = cost; // 默认初始剩余回合数等于生产力成本
	prereqTechID = 4; // 前置科技: 书写(Tech ID 4)
	possibleBuildings = std::vector<BuildingCategory>({
		BuildingCategory::LIBRARY, 
		BuildingCategory::UNIVERSITY, 
		BuildingCategory::LABORATORY
	});
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
	if (status != ProductionStatus::COMPLETED)
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
	// 每两个相邻区域+1点科技
	int districtCount = 0;
	for (const auto& neighbor : neighbors)
	{
		if (District::isThereDistrictAt(neighbor))
		{
			districtCount++;
		}
	}
	adjacencyBonus.scienceYield += (districtCount / 2); // 每两个相邻区域+1科技


	// 计算建筑加成
	for (auto building : buildings)
	{
		buildingBonus += building->getYield();
	}
}