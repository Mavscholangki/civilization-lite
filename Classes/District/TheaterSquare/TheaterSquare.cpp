#include "TheaterSquare.h"

USING_NS_CC;

int TheaterSquare::theaterSquareCount = 0;

TheaterSquare::TheaterSquare(Hex pos, std::string name)
	: District(pos, DistrictType(District::DistrictType::THEATER_SQUARE), name)
{
	isConstructed = false;
	productionCost = 54; // 剧院广场建造成本
	currentProgress = 0;
	turnsRemaining = productionCost; // 默认初始剩余回合数等于生产力成本
	prereqCivic = u8"戏剧与诗歌"; // 需要“戏剧与诗歌”市政解锁
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
	if (!isConstructed)
	{
		adjacencyBonus = { 0,0,0,0,0 };
		return;
	}
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 剧院广场的加成产出计算逻辑
	// 例如：每个相邻的区域增加2点文化产出
	//auto gameScene = static_cast<GameScene*>(Director::getInstance()->getRunningScene());
	//if (!gameScene) return;
	//std::vector<Hex> neighbors = getHexNeighbors(_pos);
	//for (const auto& neighbor : neighbors)
	//{
	//	// 相邻的每个区域+2点文化产出
	//	District* adjacentDistrict = gameScene->getDistrictAtHex(neighbor);
	//	if (adjacentDistrict && adjacentDistrict->getType() == District::DistrictType::THEATER_SQUARE)
	//	{
	//		adjacencyBonus.cultureYield += 2; // 每个相邻区域+2文化
	//	}
	//}
}

bool TheaterSquare::canErectDistrict(Hex where)
{
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return false;
	TileData tileData = gameScene->getTileData(where);
	// 剧院广场不能建在海洋、海岸和山脉上
	if (tileData.type != TerrainType::OCEAN &&
		tileData.type != TerrainType::COAST &&
		tileData.type != TerrainType::MOUNTAIN)
	{
		// if (prereqCivic.empty() || gameScene->getPlayer()->isCivicCompleted(prereqCivic))
			return true;
	}
	return false;
}

bool TheaterSquare::addBuilding(Building::BuildingType building)
{
	if (building != Building::BuildingType::AMPHITHEATER &&
		building != Building::BuildingType::MUSEUM &&
		building != Building::BuildingType::BROADCAST_CENTER)
	{
		return false; // 剧院广场只能建造特定建筑
	}
	if (!isConstructed)
	{
		return false; // 未建造完成不能添加建筑
	}

	// 创建建筑实例
	Building* newBuilding = new Building(building);
	if (!newBuilding)
		return false; // 创建失败
	if (!newBuilding->canErectBuilding())
	{
		delete newBuilding;
		newBuilding = nullptr;
		return false; // 不满足建造条件
	}
	// 添加建筑到区域
	buildings.push_back(newBuilding);
	updateGrossYield(); // 更新区域总产出
	return true;
}