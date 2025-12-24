#include "Harbor.h"
USING_NS_CC;
int Harbor::harborCount = 0;

Harbor::Harbor(int player, Hex pos, std::string name)
	: District(player, pos, DistrictType(District::DistrictType::HARBOR), name)
{
	cost = 54; // 港口建造成本
	progress = 0;
	turnsRemaining = cost; // 默认初始剩余回合数等于生产力成本
	prereqTechID = 5; // 前置科技: 航海(Tech ID 5)
	possibleBuildings = std::vector<BuildingCategory>({
		BuildingCategory::LIGHTHOUSE,
		BuildingCategory::DOCKYARD,
		BuildingCategory::DOCKS,
		});
	prereqTerrains = { TerrainType::COAST, TerrainType::OCEAN }; // 港口只能建在海岸或海洋上
	baseBenefit.foodYield = 1;    // 基础产出: +1粮食
	baseBenefit.goldYield = 2;    // 基础产出: +2金币
	updateCitizenBenefit();
	updateGrossYield();
	if (canErectDistrict(pos))
	{
		harborCount++;
	}
	// 绘制港口 (蓝色六边形)
	auto draw = DrawNode::create();
	Vec2 vertices[6] = {
		Vec2(0, 20),
		Vec2(17.32f, 10),
		Vec2(17.32f, -10),
		Vec2(0, -20),
		Vec2(-17.32f, -10),
		Vec2(-17.32f, 10)
	};
	draw->drawSolidPoly(vertices, 6, Color4F::BLUE);
	// 加个白色轮廓
	draw->drawPoly(vertices, 6, false, Color4F::WHITE);
	_harborVisual = draw;
}

void Harbor::calculateBonus()
{
	// 如果未建造完成,则无加成产出
	if (status != ProductionStatus::COMPLETED)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 港口的加成产出计算逻辑
	std::vector<Hex> neighbors = getHexNeighbors(_pos);
	int districtCount = 0;
	for (const auto& neighbor : neighbors)
	{
		// 相邻的每两个区域+1点金币
		if (District::isThereDistrictAt(neighbor))
		{
			districtCount++;
		}
	}
	adjacencyBonus.goldYield += (districtCount / 2); // 每两个相邻区域+1金币

	// 计算建筑加成
	for (const auto& building : buildings)
	{
		buildingBonus += building->getYield();
	}
}