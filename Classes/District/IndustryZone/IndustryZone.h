#ifndef __INDUSTRY_ZONE_H__
#define __INDUSTRY_ZONE_H__
#include "District/Base/District.h"
class IndustryZone : public District {
public:
	IndustryZone(Hex pos, std::string name);
	virtual void calculateBonus();
	virtual bool canErectDistrict(Hex where);
	virtual bool addBuilding(Building::BuildingType buildingType);
	cocos2d::Node* _industryZoneVisual;
private:
	static int industryZoneCount; // 工业区计数器
};

#endif // __INDUSTRY_ZONE_H__