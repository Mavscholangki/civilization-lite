#include "Spaceport.h"

USING_NS_CC;

int Spaceport::spaceportCount = 0;

Spaceport::Spaceport(int player, Hex pos, std::string name)
	: District(player, pos, DistrictType(District::DistrictType::SPACEPORT), name)
{
	cost = 1800; // 航天中心建造成本
	progress = 0;
	turnsRemaining = cost; // 默认初始剩余回合数等于生产力成本
	prereqTechID = 21; // 前置科技: 火箭学(Tech ID 21)
	possibleBuildings = std::vector<BuildingCategory>({
		BuildingCategory::LAUNCH_SATELLITE
		});
	updateGrossYield();
	if (canErectDistrict(pos))
	{
		spaceportCount++;
	}
	// 绘制航天中心 (白色六边形)
	auto draw = DrawNode::create();
	Vec2 vertices[6] = {
		Vec2(0, 20),
		Vec2(17.32f, 10),
		Vec2(17.32f, -10),
		Vec2(0, -20),
		Vec2(-17.32f, -10),
		Vec2(-17.32f, 10)
	};
	draw->drawSolidPoly(vertices, 6, Color4F::WHITE);
	// 加个黑色轮廓
	draw->drawPoly(vertices, 6, false, Color4F::BLACK);
	_spaceportVisual = draw;
}

void Spaceport::calculateBonus()
{
	// 航天中心没有加成产出
	adjacencyBonus = { 0, 0, 0, 0, 0 };
}