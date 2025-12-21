#include "Harbor.h"
USING_NS_CC;
int Harbor::harborCount = 0;

Harbor::Harbor(Hex pos, std::string name)
	: District(pos, DistrictType(District::DistrictType::HARBOR), name)
{
	isConstructed = false;
	productionCost = 54; // 港口建造成本
	currentProgress = 0;
	turnsRemaining = productionCost; // 默认初始剩余回合数等于生产力成本
	prereqTech = u8"航海"; // 需要“航海”科技解锁
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
	if (!isConstructed)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 港口的加成产出计算逻辑
	// 例如：每个相邻的海洋地块增加2点金币产出
	auto gameScene = static_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return;
	std::vector<Hex> neighbors = getHexNeighbors(_pos);
	int districtCount = 0;
	for (const auto& neighbor : neighbors)
	{
		// 相邻的每两个区域+1点金币
		District* adjacentDistrict = gameScene->getDistrictAtHex(neighbor);
		if (adjacentDistrict && adjacentDistrict->getType() == District::DistrictType::THEATER_SQUARE)
		{
			districtCount++;
		}
	}
	adjacencyBonus.goldYield += (districtCount / 2); // 每两个相邻区域+1金币
}

bool Harbor::canErectDistrict(Hex where)
{
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return false;
	TileData tileData = gameScene->getTileData(where);
	// 港口只能建在海岸上
	if (tileData.type == TerrainType::OCEAN)
	{
		if (prereqTech.empty() || gameScene->getPlayer()->isTechResearched(prereqTech))
			return true;
	}
	return false;
}

bool Harbor::addBuilding(Building::BuildingType buildingType)
{
	if (buildingType != Building::BuildingType::LIGHTHOUSE &&
		buildingType != Building::BuildingType::DOCKYARD &&
		buildingType != Building::BuildingType::DOCKS)
	{
		return false; // 港口只能建造特定建筑
	}
	if (!isConstructed)
	{
		return false; // 港口未建造完成，不能添加建筑
	}
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
	// 添加建筑物到区域
	buildings.push_back(newBuilding);
	// 建筑物可能会影响区域产出,调用更新函数
	updateGrossYield();
	return true;
}