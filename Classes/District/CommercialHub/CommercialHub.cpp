#include "CommercialHub.h"
#include "Scene/GameScene.h"
USING_NS_CC;


int CommercialHub::commercialHubCount = 0;
CommercialHub::CommercialHub(Hex pos, std::string name)
	: District(pos, DistrictType(District::DistrictType::COMMERCIAL_HUB), name)
{
	isConstructed = false;
	productionCost = 54; // 商业中心建造成本
	currentProgress = 0;
	turnsRemaining = productionCost; // 默认初始剩余回合数等于生产力成本
	prereqTech = u8"货币"; // 需要“货币”科技解锁
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
	if (!isConstructed)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 商业中心的加成产出计算逻辑
	// 例如：每个相邻的河流地块增加1点金币产出
	//auto gameScene = static_cast<GameScene*>(Director::getInstance()->getRunningScene());
	//if (!gameScene) return;
	//std::vector<Hex> neighbors = getHexNeighbors(_pos);
	//// 暂不考虑河流加成
	///*int riverCount = 0;
	//for (const auto& neighbor : neighbors)
	//{
	//	TileData tileData = gameScene->getTileData(neighbor);
	//	if (tileData.hasRiver)
	//	{
	//		riverCount++;
	//	}
	//}
	//adjacencyBonus.goldYield += riverCount * 1;*/ // 每个河流+1金币
	//// 每两个相邻区域+1点金币
	//// 每个相邻港口+2点金币
	//int districtCount = 0;
	//for (const auto& neighbor : neighbors)
	//{
	//	// 相邻的每两个区域+1点金币
	//	District* adjacentDistrict = gameScene->getDistrictAtHex(neighbor);
	//	if (adjacentDistrict)
	//	{
	//		districtCount++;
	//		if (adjacentDistrict->getType() == District::DistrictType::HARBOR)
	//		{
	//			adjacencyBonus.goldYield += 2; // 每个相邻港口+2金币
	//		}
	//	}
	//}
	// 计算建筑加成
	for (auto building : buildings)
	{
		buildingBonus += building->getYield();
	}
	adjacencyBonus += buildingBonus;
}

bool CommercialHub::canErectDistrict(Hex where)
{
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return false;
	TileData tileData = gameScene->getTileData(where);
	// 简单示例: 只能在陆地上建造商业中心
	if (tileData.type != TerrainType::OCEAN &&
		tileData.type != TerrainType::COAST &&
		tileData.type != TerrainType::MOUNTAIN)
	{
		// if (prereqTech.empty() || gameScene->getPlayer()->isTechResearched(prereqTech))
			return true;
	}
	return true;
}

bool CommercialHub::addBuilding(Building::BuildingType buildingType)
{
	if (buildingType != Building::BuildingType::MARKET &&
		buildingType != Building::BuildingType::BANK &&
		buildingType != Building::BuildingType::STOCK_EXCHANGE)
	{
		return false; // 商业中心只能建造特定建筑
	}
	// 创建建筑实例
	Building* newBuilding = new Building(buildingType);
	if (!newBuilding)
		return false; // 创建失败
	if (!newBuilding->canErectBuilding())
	{
		delete newBuilding;
		newBuilding = nullptr;
		return false; // 不满足建造条件
	}
	// 添加建筑物
	buildings.push_back(newBuilding);
	// 建筑物可能会影响区域产出,调用更新函数
	updateGrossYield();
	return true;
}