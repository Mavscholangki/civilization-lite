#ifndef __PRODUCTION_PROGRAM_H__
#define __PRODUCTION_PROGRAM_H__
#include <vector>
#include "City/Yield.h"
#include "Utils/HexUtils.h"
#include <string>
#include <map>
class ProductionProgram {
public:
	enum class ProductionType {
		UNIT,
		BUILDING,
		DISTRICT,
		TO_BE_DEFINED
	};

	struct UnlockCondition {
		std::string name;
		int prereqTechID;
		int prereqCivicID;
		UnlockCondition(std::string n, int tc, int cv) : name(n), prereqTechID(tc), prereqCivicID(cv) {}
	};

	static std::map<int, UnlockCondition> programs;
	
	static std::map<std::string, int> ids;

	static std::vector<std::pair<int, UnlockCondition>> findProgram(int techID, int civicID)
	{
		std::vector<std::pair<int, UnlockCondition>> unlockedPrograms;
		for (auto program : programs)
		{
			if (program.second.prereqTechID == -1 && program.second.prereqCivicID == -1)
				unlockedPrograms.push_back(program);
		}

		if (techID == -1 && civicID != -1)
		{
			for (auto program : programs)
			{
				if (program.second.prereqCivicID == civicID)
					unlockedPrograms.push_back(program);
			}
		}
		else if (civicID == -1 && techID != -1)
		{
			for (auto program : programs)
			{
				if (program.second.prereqTechID == techID)
					unlockedPrograms.push_back(program);
			}
		}
		return unlockedPrograms;
	}

	enum class ProductionStatus {
		IN_PROGRESS,
		COMPLETED,
		PAUSED
	}; 

	ProductionProgram(ProductionType type, std::string name, Hex pos, int cost, bool canPurchase, int purchaseCost = 0);
	~ProductionProgram()
	{}

	void			setPosOnCreated(Hex pos) { posOnCreated = pos; }


	ProductionType	getType() const { return type; }
	ProductionStatus getStatus() const { return status; }
	int				getID() const { return _id; }
	std::string		getName() const { return _name; }
	Hex				getPosOnCreated() const { return posOnCreated; }
	int				getCost() const { return cost; }
	int				getProgress() const { return progress; }
	int 			getTurnsRemaining() const { return turnsRemaining; }
	bool 			getCanPurchase() const { return canPurchase; }
	int				getPurchaseCost() const { return purchaseCost; }

	void pauseProduction() { status = ProductionStatus::PAUSED; } // 暂停生产
	void resumeProduction() { status = ProductionStatus::IN_PROGRESS; } // 恢复生产
	bool purchaseCompletion(); // 用黄金购买完成生产
	virtual void addProgress(int amount); // 增加生产进度(需在派生类中实现具体逻辑)
	bool isCompleted() const { return progress >= cost; } // 是否完成生产
protected:
	void completeProduction() { status = ProductionStatus::COMPLETED; progress = cost; } // 完成生产
	ProductionType type;
	ProductionStatus status;
	int _id;
	std::string _name;
	Hex posOnCreated;
	int cost;
	int progress;
	int turnsRemaining; // 剩余回合数
	bool canPurchase; // 是否可以用黄金购买完成
	int purchaseCost; // 黄金购买成本
};

#endif // __PRODUCTION_PROGRAM_H__
