#ifndef __CAMPUS_H__
#define __CAMPUS_H__

#include "District/Base/District.h"
#include "City/Yield.h"
class Campus : public District {
public:
	Campus(Hex pos, std::string name);
	virtual void calculateBonus();
	virtual bool canErectDistrict(Hex where);
	virtual bool addBuilding(Building::BuildingType building);
	enum class BuildingType {
		LIBRARY, // 图书馆
		UNIVERSITY, // 大学
		RESEARCH_LAB // 研究实验室
	};
private:
	static int campusCount; // 校园区计数器
	cocos2d::Node* _campusVisual;
};
#endif