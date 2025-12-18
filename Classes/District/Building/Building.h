#ifndef __BUILDING_H__
#define __BUILDING_H__

#include <string>

class Building {
public:
	enum class BuildingTypeName {
		LIBRARY,
		WORKSHOP,
		TO_BE_DEFINED
	};
	struct BuildingType {
		BuildingTypeName typeName;
		int cost;
		BuildingType() :
			typeName(BuildingTypeName::TO_BE_DEFINED), cost(0)
		{
		};
		BuildingType(BuildingTypeName type) :
			typeName(type), cost(0)
		{
			switch (typeName)
			{
			case BuildingTypeName::LIBRARY:
				cost = 80;
				break;
			case BuildingTypeName::WORKSHOP:
				cost = 100;
				break;
			default:
				cost = 0;
				break;
			}
		}
	};
	Building(BuildingType type, std::string name) :
		_type(type), _name(name)
	{
	};
	inline BuildingType getType() { return _type; }
	inline std::string getName() { return _name; }
private:
	BuildingType _type;
	std::string _name;
};

#endif // __BUILDING_H__