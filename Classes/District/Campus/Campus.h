#ifndef __CAMPUS_H__
#define __CAMPUS_H__

#include "District/Base/District.h"
#include "City/Yield.h"
class Campus : public District {
public:
	Campus(int player, Hex pos, std::string name);
	virtual void calculateBonus();
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