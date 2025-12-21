#ifndef __THEATER_SQUARE_H__
#define __THEATER_SQUARE_H__

#include "District/Base/District.h"

class TheaterSquare : public District {
public:
	TheaterSquare(Hex pos, std::string name);
	virtual void calculateBonus();
	virtual bool canErectDistrict(Hex where);
	virtual bool addBuilding(Building::BuildingType buildingType);
	cocos2d::Node* _theaterSquareVisual;
private:
	static int theaterSquareCount; // 剧院广场计数器
};

#endif // __THEATER_SQUARE_H__