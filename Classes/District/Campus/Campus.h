#ifndef __CAMPUS_H__
#define __CAMPUS_H__

#include "District/Base/District.h"
#include "City/Yield.h"
class Campus : public District {
public:
	static Campus* create(Hex pos);
	bool initCampus(Hex pos);
	// 校园特有属性和方法
	Yield getCampusYield(); // 获取校园的产出
private:
	cocos2d::Node* _campusVisual;
};
#endif