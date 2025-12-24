#ifndef __COMMERCIAL_HUB_H__
#define __COMMERCIAL_HUB_H__
#include "District/Base/District.h"
#include "District/Building/Building.h"


class CommercialHub : public District {
public:
	CommercialHub(int player, Hex pos, std::string name);
	virtual void calculateBonus();
	static int commercialHubCount; // 商业中心计数器
	cocos2d::Node* _commercialHubVisual;
};


#endif // __COMMERCIAL_HUB_H__