#ifndef __SPACEPORT_H__
#define __SPACEPORT_H__
#include "District/Base/District.h"
class Spaceport : public District {
public:
	Spaceport(int player, Hex pos, std::string name);
	virtual void calculateBonus();
	cocos2d::Node* _spaceportVisual;
private:
	static int spaceportCount; // 航天中心计数器
};
#endif // __SPACEPORT_H__