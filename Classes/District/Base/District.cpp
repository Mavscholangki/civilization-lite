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
	calculateBonus();
	grossYield = baseYield + adjacencyBonus;

    // 这里可以放置区域的绘制方案(暂未确定)
}

// 提供一定的生产力后,更新建造状态,返回建造是否成功
bool District::addProduction(int production)
{
	if (production <= 0)
		return isConstructed; // 无效生产力输入
	currentProgress += production;
	if (currentProgress >= productionCost) // 建造完成
	{
		isConstructed = true;
		currentProgress = productionCost;
		updateGrossYield(); // 更新区域产出
		return true;
	}
	else
		return false;
}

void District::calculateBonus()
{
	adjacencyBonus = { 0, 0, 0, 0, 0 }; // 重置加成产出
	// 待地块数据结构完善后补充
	/*for (auto building : buildings)
	{
		Yield buildingBonus = building->getAdjacencyBonus();
		adjacencyBonus += buildingBonus;
	}*/
}

// 区域总产出更新(后续可能会加上对周围6地块的状态监听/周围地块状态改变时调用)
void District::updateGrossYield()
{
	if (isConstructed)
		baseYield = _type.baseYield;
	calculateBonus(); // 更新加成产出
	grossYield = baseYield + adjacencyBonus;
}

//bool District::addBuilding(Building* addedBuilding)
//{
//	// 简单示例: 直接添加建筑物
//	buildings.push_back(addedBuilding);
//	// 建筑物可能会影响区域产出,调用更新函数
//	updateGrossYield();
//	return true;
//}

bool District::canErectDistrict(Hex where)
{
	// 简单示例: 只能在陆地上建造区域
	// 待地块数据结构完善后补充具体逻辑
	return true;
}

