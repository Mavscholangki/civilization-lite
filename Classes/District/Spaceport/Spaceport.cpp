#include "Spaceport.h"

USING_NS_CC;

int Spaceport::spaceportCount = 0;

Spaceport::Spaceport(Hex pos, std::string name)
	: District(pos, DistrictType(District::DistrictType::SPACEPORT), name)
{
	isConstructed = false;
	productionCost = 1800; // 航天中心建造成本
	currentProgress = 0;
	turnsRemaining = productionCost; // 默认初始剩余回合数等于生产力成本
	prereqTech = u8"火箭技术"; // 需要“火箭技术”科技解锁
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

bool Spaceport::canErectDistrict(Hex where)
{
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return false;
	TileData tileData = gameScene->getTileData(where);
	// 航天中心不能建在海洋、海岸和山脉上
	if (tileData.type != TerrainType::OCEAN &&
		tileData.type != TerrainType::COAST &&
		tileData.type != TerrainType::MOUNTAIN)
	{
		// if (prereqTech.empty() || gameScene->getPlayer()->isTechResearched(prereqTech))
			return true;
	}
	return false;
}

bool Spaceport::addBuilding(Building::BuildingType buildingType)
{
	if (buildingType != Building::BuildingType::LAUNCH_SATELLITE)
	{
		return false; // 航天中心只能建造特定建筑, 注意发射卫星简化为一种建筑(原版是一个项目)
	}
	if (!isConstructed)
	{
		return false; // 未建造完成不能添加建筑
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
	// 添加建筑到区域
	buildings.push_back(newBuilding);
	updateGrossYield(); // 建筑物可能会影响区域产出,调用更新函数
	return true;
}