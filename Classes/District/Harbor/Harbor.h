#ifndef __HARBOR_H__
#define __HARBOR_H__
#include "District/Base/District.h"
#include "City/Yield.h"
#include "District/Building/Building.h"

class Harbor : public District {
public:
	Harbor(Hex pos, std::string name);
	virtual void calculateBonus();
	virtual bool canErectDistrict(Hex where);
	virtual bool addBuilding(Building::BuildingType buildingType);
	cocos2d::Node* _harborVisual;
private:
	static int harborCount; // 港口计数器
};

#endif // __HARBOR_H__