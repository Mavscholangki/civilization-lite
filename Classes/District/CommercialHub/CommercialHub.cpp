#include "CommercialHub.h"
#include "Scene/GameScene.h"
USING_NS_CC;


int CommercialHub::commercialHubCount = 0;
CommercialHub::CommercialHub(int player, Hex pos, std::string name)
	: District(player, pos, DistrictType(District::DistrictType::COMMERCIAL_HUB), name)
{
	cost = 54; // 商业中心建造成本
	turnsRemaining = cost; // 默认初始剩余回合数等于生产力成本
	prereqTechID = 6; // 前置科技: 货币(Tech ID 6)
	possibleBuildings = std::vector<BuildingCategory>({
		BuildingCategory::MARKET,
		BuildingCategory::BANK,
		BuildingCategory::STOCK_EXCHANGE
		});
	baseBenefit.goldYield = 4;    // 基础产出: +4金币
	updateCitizenBenefit();
	updateGrossYield();
	if (canErectDistrict(pos))
	{
		commercialHubCount++;
	}

	// 绘制商业中心 (黄色六边形)
	auto draw = DrawNode::create();
	Vec2 vertices[6] = {
		Vec2(0, 20),
		Vec2(17.32f, 10),
		Vec2(17.32f, -10),
		Vec2(0, -20),
		Vec2(-17.32f, -10),
		Vec2(-17.32f, 10)
	};
	draw->drawSolidPoly(vertices, 6, Color4F::YELLOW);
	// 加个白色轮廓
	draw->drawPoly(vertices, 6, false, Color4F::WHITE);
	_commercialHubVisual = draw;
}

void CommercialHub::calculateBonus()
{
	// 如果未建造完成,则无加成产出
	if (status != ProductionStatus::COMPLETED)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 商业中心的加成产出计算逻辑
	// 例如：每个相邻的河流地块增加1点金币产出
	std::vector<Hex> neighbors = getHexNeighbors(_pos);
	//// 暂不考虑河流加成
	//s*int riverCount = 0;
	//for (const auto& neighbor : neighbors)
	//{
	//	TileData tileData = gameScene->getTileData(neighbor);
	//	if (tileData.hasRiver)
	//	{
	//		riverCount++;
	//	}
	//}
	//adjacencyBonus.goldYield += riverCount * 1;*/ // 每个河流+1金币
	// 每两个相邻区域+1点金币
	// 每个相邻港口+2点金币
	int districtCount = 0;
	for (const auto& neighbor : neighbors)
	{
		// 相邻的每两个区域+1点金币
		if (District::isThereDistrictAt(neighbor))
		{
			districtCount++;
		}
	}
	adjacencyBonus.goldYield += (districtCount / 2) * 1;
	// 计算建筑加成
	for (auto building : buildings)
	{
		buildingBonus += building->getYield();
	}
}