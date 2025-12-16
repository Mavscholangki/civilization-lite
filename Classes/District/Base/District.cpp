#include "cocos2d.h"
#include "District.h"
#include "Scene/GameScene.h"

USING_NS_CC;

int max3(int a, int b, int c)
{
	return (a > b && a > c) ? a : (b > c ? b : c);
}
int District::count = 0;

District::District(Hex pos, DistrictType type, std::string name)
{
	// 属性初始化
	_id = count;
	count++;
	_type = type; 
	_name = name; 

	_pos = pos; 

	isConstructed = false;
	productionCost = type.cost;
	currentProgress = 0;

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