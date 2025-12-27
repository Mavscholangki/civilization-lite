#ifndef __UNIT_FACTORY_H__
#define __UNIT_FACTORY_H__
#include "Core/Player.h"
#include "AllKindsOfUnits.h"

// UnitFactory.h
class UnitFactory {
public:
    static AbstractUnit* createUnit(const std::string& name, int owner, const Hex& position) {
        if (name == "Settler") {
            Settler* unit = new Settler();
            unit->initUnit(owner, position);
            return unit;
        }
        else if (name == "Builder") {
            Builder* unit = new Builder();
            unit->initUnit(owner, position);
            return unit;
        }
        else if (name == "Warrior") {
            Warrior* unit = new Warrior();
            unit->initUnit(owner, position);
            return unit;
        }
		else if (name == "Swordsman")
		{
			Swordsman* unit = new Swordsman();
			unit->initUnit(owner, position);
			return unit;
		}
		else if (name == "LineInfantry")
		{
			LineInfantry* unit = new LineInfantry();
			unit->initUnit(owner, position);
			return unit;
		}
		else if (name == "Archer")
		{
			Archer* unit = new Archer();
			unit->initUnit(owner, position);
			return unit;
		}
		else if (name == "Crossbowman")
		{
			Crossbowman* unit = new Crossbowman();
			unit->initUnit(owner, position);
			return unit;
		}
		else if (name == "Musketeers")
		{
			Musketeers* unit = new Musketeers();
			unit->initUnit(owner, position);
			return unit;
		}
		else if (name == "Catapult")
		{
			Catapult* unit = new Catapult();
			unit->initUnit(owner, position);
			return unit;
		}
		else if (name == "Cannon")
		{
			Cannon* unit = new Cannon();
			unit->initUnit(owner, position);
			return unit;
		}
		else if (name == "Biplane")
		{
			Biplane* unit = new Biplane();
			unit->initUnit(owner, position);
			return unit;
		}
		else if (name == "JetFighter")
		{
			JetFighter* unit = new JetFighter();
			unit->initUnit(owner, position);
			return unit;
		}
        // ... 其他单位
        else {
            return nullptr;
        }
    }
};

#endif // __UNIT_FACTORY__