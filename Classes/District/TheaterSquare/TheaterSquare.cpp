#include "TheaterSquare.h"

USING_NS_CC;

int TheaterSquare::theaterSquareCount = 0;

TheaterSquare::TheaterSquare(int player, Hex pos, std::string name)
	: District(player, pos, DistrictType(District::DistrictType::THEATER_SQUARE), name)
{
	cost = 54; // 剧院广场建造成本
	turnsRemaining = cost; // 默认初始剩余回合数等于生产力成本
	prereqCivicID = 102; // 前置市政: 技艺(Tech ID 102)
	possibleBuildings = std::vector<BuildingCategory>({
		BuildingCategory::AMPHITHEATER,
		BuildingCategory::MUSEUM,
		BuildingCategory::BROADCAST_CENTER
		});
	baseMaintenanceCost = 1; // 基础维护费用
	updateMaintenanceCost();
	baseBenefit.cultureYield = 2; // 区域公民收益: +2文化
	updateCitizenBenefit();
	updateGrossYield();
	if (canErectDistrict(pos))
	{
		theaterSquareCount++;
	}
	// 绘制剧院广场 (紫色六边形)
	auto draw = DrawNode::create();
	Vec2 vertices[6] = {
		Vec2(0, 20),
		Vec2(17.32f, 10),
		Vec2(17.32f, -10),
		Vec2(0, -20),
		Vec2(-17.32f, -10),
		Vec2(-17.32f, 10)
	};
	draw->drawSolidPoly(vertices, 6, Color4F::MAGENTA);
	// 加个白色轮廓
	draw->drawPoly(vertices, 6, false, Color4F::WHITE);
	_theaterSquareVisual = draw;
}

void TheaterSquare::calculateBonus()
{
	// 如果未建造完成,则无加成产出
	if (status != ProductionStatus::COMPLETED)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 剧院广场的加成产出计算逻辑
	// 例如：每个相邻的区域增加2点文化产出
	std::vector<Hex> neighbors = getHexNeighbors(_pos);
	for (const auto& neighbor : neighbors)
	{
		// 相邻的每个区域+2点文化产出
		if (District::isThereDistrictAt(neighbor))
		{
			adjacencyBonus.cultureYield += 2;
		}
	}
}
