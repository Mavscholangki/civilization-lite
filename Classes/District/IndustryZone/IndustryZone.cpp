#include "IndustryZone.h"
#include "Scene/GameScene.h"
#include "District/Building/Building.h"
USING_NS_CC;
int IndustryZone::industryZoneCount = 0;

IndustryZone::IndustryZone(int player, Hex pos, std::string name)
	: District(player, pos, DistrictType(District::DistrictType::INDUSTRY_ZONE), name)
{
	cost = 110; // 工业区建造成本
	turnsRemaining = cost; // 默认初始剩余回合数等于生产力成本
	prereqTechID = 12; // 前置科技: 学徒制(Tech ID 12)
	possibleBuildings = std::vector<BuildingCategory>({
		BuildingCategory::WORKSHOP,
		BuildingCategory::FACTORY,
		BuildingCategory::POWER_PLANT,
		BuildingCategory::RHUR_VALLEY
		});
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
	if (status != ProductionStatus::COMPLETED)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 工业区的加成产出计算逻辑
	// 例如：每个相邻的森林地块增加1点生产力产出
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return;
	std::vector<Hex> neighbors = getHexNeighbors(_pos);
	for (const auto& neighbor : neighbors)
	{
		// 相邻的每个区域+1点生产力
		if (District::isThereDistrictAt(neighbor))
		{
			adjacencyBonus.productionYield += 1;
		}
	}
	// 计算建筑加成
	for (auto building : buildings)
	{
		buildingBonus += building->getYield();
	}
}