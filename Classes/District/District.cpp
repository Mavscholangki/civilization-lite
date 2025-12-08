#include "cocos2d.h"
#include "District.h"
#include "Scene/GameScene.h"

USING_NS_CC;

int max3(int a, int b, int c)
{
	return (a > b && a > c) ? a : (b > c ? b : c);
}

bool District::canErectDistrict()
{
	if (_city->population / 3 <= count)
		return false;

	if (max3(abs(_city->gridPos.q - _pos.q), abs(_city->gridPos.r - _pos.r), abs(_city->gridPos.s - _pos.s)) > 3)
		return false;

	// 这里放置地形检测
	// auto gameMapLayer = 
	// if(_type.requirement != GameScene::)

	return true;
}

District::District(Hex pos, DistrictType type, std::string name, BaseCity* city)
{
	// 属性初始化
	_id = count;
	count++;
	_type = type; 
	_name = name; 

	_pos = pos; 
	_city = city; 

	isConstructed = false;
	productionCost = type.cost;
	currentProgress = 0;
	calculateTurnsRemaining();

	baseYield = type.baseYield; 
	calculateAdjacencyBonus();
	grossYield = baseYield + adjacencyBonus;

    // 这里可以放置区域的绘制方案(暂未确定)
}

// 提供一定的生产力后,更新建造状态,返回建造是否成功
bool District::addProduction(int production)
{
	if (production <= 0)
		return isConstructed;
	currentProgress += production;
	if (currentProgress >= productionCost)
	{
		isConstructed = true;
		currentProgress = productionCost;
		updateGrossYield();
		return true;
	}
	else
		return false;
}

// 计算建造剩余回合数,每回合可以进行的进度即为城市的生产力
void District::calculateTurnsRemaining()
{
	if (_city->population <= 0)
		turnsRemaining = INT_MAX;
	else
	{
		int remainingCost = productionCost - currentProgress;
		turnsRemaining = (remainingCost / _city->population) > 0 ? (remainingCost / _city->population + 1) : 1;
	}
}

// 计算相邻区域加成(由于和具体的区域相关,暂时按下不表)
void District::calculateAdjacencyBonus()
{
	adjacencyBonus = {0, 0, 0, 0, 0};
}

// 区域总产出更新(后续可能会加上对周围6地块的状态监听/周围地块状态改变时调用)
void District::updateGrossYield()
{
	if (isConstructed)
		baseYield = _type.baseYield;
	calculateAdjacencyBonus(); // 更新加成产出
	grossYield = baseYield + adjacencyBonus;
}

#ifdef __BUILDING_H__
bool District::addBuilding(Building addedBuilding)
{
	
}
#endif