#ifndef __HARBOR_H__
#define __HARBOR_H__
#include "District/Base/District.h"
#include "City/Yield.h"
#include "District/Building/Building.h"

class Harbor : public District {
public:
	Harbor(int player, Hex pos, std::string name);
	virtual void calculateBonus();
	cocos2d::Node* _harborVisual;
private:
	static int harborCount; // 港口计数器
};

#endif // __HARBOR_H__