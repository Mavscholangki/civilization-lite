#ifndef __DISTRICT_FACTORY_H__
#define __DISTRICT_FACTORY_H__
#include "Core/Player.h"
#include "AllKindsOfDistricts.h"

class DistrictFactory {
public:
	static District* createDistrict(const std::string& name, int owner, const Hex& position, Node*& _visual) {
		if (name == "Downtown") {
			auto newDistrict = new Downtown(owner, position, name);
			_visual = newDistrict->_downtownVisual;
			return newDistrict;
		}
		else if (name == "Campus") {
			auto newDistrict = new Campus(owner, position, name);
			_visual = newDistrict->_campusVisual;
			return newDistrict;
		}
		else if (name == "TheaterSquare") {
			auto newDistrict = new TheaterSquare(owner, position, name);
			_visual = newDistrict->_theaterSquareVisual;
			return newDistrict;
		}
		else if (name == "CommercialHub")
		{
			auto newDistrict = new CommercialHub(owner, position, name);
			_visual = newDistrict->_commercialHubVisual;
			return newDistrict;
		}
		else if (name == "Harbor")
		{
			auto newDistrict = new Harbor(owner, position, name);
			_visual = newDistrict->_harborVisual;
			return newDistrict;
		}
		else if (name == "Spaceport")
		{
			auto newDistrict = new Spaceport(owner, position, name);
			_visual = newDistrict->_spaceportVisual;
			return newDistrict;
		}
		else if (name == "IndustryZone")
		{
			auto newDistrict = new IndustryZone(owner, position, name);
			_visual = newDistrict->_industryZoneVisual;
			return newDistrict;
		}
		else {
			return nullptr;
		}
	}
};

#endif // __DISTRICT_FACTORY_H__