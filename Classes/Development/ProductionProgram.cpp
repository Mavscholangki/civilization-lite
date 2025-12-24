#include "ProductionProgram.h"



std::map<int, ProductionProgram::UnlockCondition> ProductionProgram::programs = {
	// Units
	{1, ProductionProgram::UnlockCondition("Settler", -1, -1)},
	{2, ProductionProgram::UnlockCondition("Builder", -1, -1)},

	{11, ProductionProgram::UnlockCondition("Warrior", -1, -1)},
	{12, ProductionProgram::UnlockCondition("Swordsman", 7, -1)},
	{13, ProductionProgram::UnlockCondition("LineInfantry", 12, -1)},

	{21, ProductionProgram::UnlockCondition("Archer", 3, -1)},
	{22, ProductionProgram::UnlockCondition("Crossbowman", 11, -1)},
	{23, ProductionProgram::UnlockCondition("Musketeers", 14, -1)},

	{31, ProductionProgram::UnlockCondition("Catapult", 8, -1)},
	{32, ProductionProgram::UnlockCondition("Cannon", 14, -1)},

	{41, ProductionProgram::UnlockCondition("Biplane", 18, -1)},
	{42, ProductionProgram::UnlockCondition("JetFighter", 20, -1)},

	// Districts and Buildings
	{100, ProductionProgram::UnlockCondition("Downtown", -1, -1)},
	{101, ProductionProgram::UnlockCondition("Palace", -1, -1)},
	{102, ProductionProgram::UnlockCondition("Monument", -1, -1)},
	{103, ProductionProgram::UnlockCondition("Granary", -1, -1)},

	{110, ProductionProgram::UnlockCondition("Campus", 4, -1)},
	{111, ProductionProgram::UnlockCondition("Library", 4, -1)},
	{112, ProductionProgram::UnlockCondition("University", 10, -1)},
	{113, ProductionProgram::UnlockCondition("Laboratory", 14, -1)},

	{120, ProductionProgram::UnlockCondition("CommercialHub", 6, -1)},
	{121, ProductionProgram::UnlockCondition("Market", 6, -1)},
	{122, ProductionProgram::UnlockCondition("Bank", 13, -1)},
	{123, ProductionProgram::UnlockCondition("StockExchange", 19, -1)},

	{130, ProductionProgram::UnlockCondition("Harbor", 5, -1)},
	{131, ProductionProgram::UnlockCondition("Lighthouse", 5, -1)},
	{132, ProductionProgram::UnlockCondition("Dockyard", 9, -1)},
	{133, ProductionProgram::UnlockCondition("Docks", 16, -1)},

	{140, ProductionProgram::UnlockCondition("TheaterSquare", -1, 102)},
	{141, ProductionProgram::UnlockCondition("Amphitheater", -1, 102)},
	{142, ProductionProgram::UnlockCondition("Museum", -1, 107)},
	{143, ProductionProgram::UnlockCondition("BroadcastCenter", -1, 109)},

	{150, ProductionProgram::UnlockCondition("IndustryZone", 12, -1)},
	{151, ProductionProgram::UnlockCondition("Workshop", 12, -1)},
	{152, ProductionProgram::UnlockCondition("Factory", 17, -1)},
	{153, ProductionProgram::UnlockCondition("PowerPlant", 17, -1)},
	{159, ProductionProgram::UnlockCondition("RhurValley", 17, -1)},

	{160, ProductionProgram::UnlockCondition("Spaceport", 21, -1)},
	{161, ProductionProgram::UnlockCondition("LaunchSatellite", 21, -1)}
};

std::map<std::string, int> ProductionProgram::ids = {
	// Units
	{"Settler", 1},
	{"Builder", 2},

	{"Warrior", 11},
	{"Swordsman", 12},
	{"LineInfantry", 13},

	{"Archer", 21},
	{"Crossbowman", 22},
	{"Musketeers", 23},

	{"Catapult", 31},
	{"Cannon", 32},

	{"Biplane", 41},
	{"JetFighter", 42},

	// Districts and Buildings
	{"Downtown", 100},
	{"Palace", 101},
	{"Monument", 102},
	{"Granary", 103},

	{"Campus", 110},
	{"Library", 111},
	{"University", 112},
	{"Laboratory", 113},

	{"CommercialHub", 120},
	{"Market", 121},
	{"Bank", 122},
	{"StockExchange", 123},

	{"Harbor", 130},
	{"Lighthouse", 131},
	{"Dockyard", 132},
	{"Docks", 133},

	{"TheaterSquare", 140},
	{"Amphitheater", 141},
	{"Museum", 142},
	{"BroadcastCenter", 143},

	{"IndustryZone", 150},
	{"Workshop", 151},
	{"Factory", 152},
	{"PowerPlant", 153},
	{"RhurValley", 159},

	{"Spaceport", 160},
	{"LaunchSatellite", 161}
};

ProductionProgram::ProductionProgram(ProductionType type, std::string name, Hex pos, int cost, bool canPurchase, int purchaseCost):
	status(ProductionStatus::IN_PROGRESS),
	type(type),
	_id(ids[name]),
	_name(name),
	posOnCreated(pos),
	cost(cost),
	progress(0),
	canPurchase(canPurchase),
	purchaseCost(purchaseCost),
	turnsRemaining(0)
{}

bool ProductionProgram::purchaseCompletion() 
{
	if (canPurchase && status != ProductionStatus::COMPLETED) {
		completeProduction();
		return true;
	}
	return false;
}

void ProductionProgram::addProgress(int amount)
{
	if (status == ProductionStatus::IN_PROGRESS) {
		progress += amount;
		if (progress >= cost) {
			progress = cost;
			status = ProductionStatus::COMPLETED;
			turnsRemaining = 0;
		}
		else {
			turnsRemaining = (cost - progress + amount - 1) / amount; // 向上取整计算剩余回合数
		}
	}
}
