#include "TechSystem.h"

TechNode::TechNode(int id, const std::string& name, int cost,
	const std::vector<int>& prereqs, const std::string& effect)
	: id(id), name(name), cost(cost), progress(0), activated(false),
	srcTechList(prereqs), effectDescription(effect) {
}

// 初始化科技树函数：目前采用硬编码
void TechTree::initializeTechTree() {
	// 清空现有数据
	techList.clear();
	activatedTechList.clear();
	currentResearchTech = -1;

	// 第1层：基础科技（成本较低）
	techList.emplace(1, TechNode(1, "畜牧", 25, {}, "解锁牧场，允许训练骑兵单位"));
	techList.emplace(2, TechNode(2, "采矿", 25, {}, "解锁矿山，允许开采矿产资源"));

	// 第2层：早期科技（成本增加）
	techList.emplace(3, TechNode(3, "弓箭手", 50, { 1, 2 }, "解锁弓箭手单位，增强远程攻击能力"));
	techList.emplace(4, TechNode(4, "书写", 50, { 1 }, "解锁图书馆，开启文化系统"));

	// 第3层：古典时代科技
	techList.emplace(5, TechNode(5, "航海", 80, { 4 }, "解锁船只，开启海上探索和贸易"));
	techList.emplace(6, TechNode(6, "货币", 80, { 4 }, "解锁市场，增加金币收入"));
	techList.emplace(7, TechNode(7, "铁器", 80, { 2 }, "解锁铁剑士，增强近战能力"));

	// 第4层：中世纪科技
	techList.emplace(8, TechNode(8, "工程学", 120, { 7 }, "解锁攻城武器，增强攻城能力"));
	techList.emplace(9, TechNode(9, "造船术", 120, { 5 }, "解锁更强大的船只，增强海军"));
	techList.emplace(10, TechNode(10, "数学", 120, { 4 }, "解锁大学，提升科研效率"));
	techList.emplace(11, TechNode(11, "机械", 120, { 7 }, "解锁弩兵和投石机"));

	// 第5层：文艺复兴科技
	techList.emplace(12, TechNode(12, "学徒制", 200, { 10, 11 }, "解锁工坊，提升生产力"));
	techList.emplace(13, TechNode(13, "银行", 200, { 6 }, "解锁银行，大幅增加金币收入"));

	// 第6层：工业时代科技
	techList.emplace(14, TechNode(14, "火药", 300, { 12 }, "解锁火枪手，开启热兵器时代"));
	techList.emplace(15, TechNode(15, "金属铸造", 300, { 12 }, "解锁工厂，大幅提升生产力"));

	// 第7层：启蒙时代科技
	techList.emplace(16, TechNode(16, "制图学", 400, { 5, 9 }, "解锁更精确的地图，提升探索效率"));
	techList.emplace(17, TechNode(17, "工业化", 400, { 15 }, "解锁工业区，开启工业革命"));
	techList.emplace(18, TechNode(18, "飞行", 400, { 16 }, "解锁飞机，开启空中战斗"));
	techList.emplace(19, TechNode(19, "经济学", 400, { 13 }, "解锁证券交易所，经济大幅增长"));

	// 第8层：现代科技
	techList.emplace(20, TechNode(20, "高级飞行", 600, { 18 }, "解锁喷气式飞机，掌握制空权"));
	techList.emplace(21, TechNode(21, "火箭学", 600, { 20 }, "解锁火箭，开启太空时代"));

	// 第9层：信息时代科技
	techList.emplace(22, TechNode(22, "卫星", 800, { 21 }, "解锁卫星，获得全球视野"));
	techList.emplace(23, TechNode(23, "核裂变", 800, { 17 }, "解锁核电站和原子弹"));
	techList.emplace(24, TechNode(24, "核聚变", 1000, { 23 }, "解锁聚变反应堆，近乎无限的能源"));

	// 设置解锁关系（构建有向图）
	// 第1层解锁第2层
	techList[1].dstTechList = { 3, 4 };
	techList[2].dstTechList = { 3 };

	// 第2层解锁第3层
	techList[3].dstTechList = { 8 };
	techList[4].dstTechList = { 5, 6, 10 };

	// 第3层解锁第4层
	techList[5].dstTechList = { 9, 16 };
	techList[6].dstTechList = { 13 };
	techList[7].dstTechList = { 8, 11 };

	// 第4层解锁第5层
	techList[8].dstTechList = { 12, 14 };
	techList[9].dstTechList = { 16 };
	techList[10].dstTechList = { 12 };
	techList[11].dstTechList = { 12 };

	// 第5层解锁第6层
	techList[12].dstTechList = { 14, 15 };
	techList[13].dstTechList = { 19 };

	// 第6层解锁第7层
	techList[14].dstTechList = { 17 };
	techList[15].dstTechList = { 17 };

	// 第7层解锁第8层
	techList[16].dstTechList = { 18 };
	techList[17].dstTechList = { 20, 23 };
	techList[18].dstTechList = { 20 };
	techList[19].dstTechList = { 20 };

	// 第8层解锁第9层
	techList[20].dstTechList = { 21 };
	techList[21].dstTechList = { 22 };
	techList[23].dstTechList = { 24 };

	// 添加一些跨时代的直接连接，让科技树更丰富
	techList[10].dstTechList.push_back(16); // 数学 -> 制图学
	techList[14].dstTechList.push_back(21); // 火药 -> 火箭学
	techList[17].dstTechList.push_back(21); // 工业化 -> 火箭学
}

// 设置当前研究的科技
bool TechTree::setCurrentResearch(int techId) {
	auto it = techList.find(techId);
	if (it == techList.end()) {
		return false;
	}

	// 检查科技是否已激活或可研究
	if (it->second.activated || !isResearchable(techId)) {
		return false;
	}

	currentResearchTech = techId;
	return true;
}

// 增加科研点数（由游戏回合系统调用）
void TechTree::addSciencePoints(int points) {
	if (currentResearchTech <= 0 || points <= 0) {
		return;
	}

	addProgressToTech(currentResearchTech, points);
}

// 触发尤里卡（由事件系统调用）
void TechTree::triggerEureka(int techId) {
	auto it = techList.find(techId);
	if (it == techList.end() || it->second.activated) {
		return;
	}

	// 尤里卡：增加该科技50%成本的科技值
	int eurekaPoints = it->second.cost / 2;

	// 通知监听器
	notifyEurekaTriggered(techId, it->second.name);

	// 增加进度
	addProgressToTech(techId, eurekaPoints);
}

// 内部函数：向科技添加进度
void TechTree::addProgressToTech(int techId, int points) {
	auto it = techList.find(techId);
	if (it == techList.end() || it->second.activated) {
		return;
	}

	// 检查前置条件（除非有尤里卡，否则必须有前置条件）
	if (!isResearchable(techId)) {
		return;
	}

	// 增加进度，但不能超过成本
	int newProgress = it->second.progress + points;
	if (newProgress > it->second.cost) {
		newProgress = it->second.cost;
	}

	it->second.progress = newProgress;

	// 通知进度更新
	notifyResearchProgress(techId, newProgress, it->second.cost);

	// 检查是否完成研究
	if (newProgress >= it->second.cost) {
		activateTech(techId);
	}
}

// 激活科技
void TechTree::activateTech(int techId) {
	auto it = techList.find(techId);
	if (it == techList.end() || it->second.activated) {
		return;
	}

	it->second.activated = true;
	activatedTechList.push_back(techId);

	// 如果当前正在研究这个科技，重置当前研究
	if (currentResearchTech == techId) {
		currentResearchTech = -1;
	}

	// 通知监听器
	notifyTechActivated(techId, it->second.name, it->second.effectDescription);

	// 处理连锁激活
	onTechActivated_internal(techId);
}

// 检查是否可研究
bool TechTree::isResearchable(int techId) const {
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end() || currentNode->second.activated) {
		return false;
	}

	// 检查所有前置条件
	for (int srcId : currentNode->second.srcTechList) {
		auto prereqNode = techList.find(srcId);
		if (prereqNode == techList.end() || !prereqNode->second.activated) {
			return false;
		}
	}

	return true;
}

// 检查科技是否激活
bool TechTree::isActivated(int techId) const {
	auto currentNode = techList.find(techId);
	return (currentNode != techList.end() && currentNode->second.activated);
}

// 获取研究进度百分比
int TechTree::getResearchProgressPercent(int techId) const {
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end() || currentNode->second.cost == 0) {
		return 0;
	}

	return static_cast<int>((currentNode->second.progress * 100) / currentNode->second.cost);
}

// 获取可研究的科技列表
std::vector<int> TechTree::getResearchableTechList() const {
	std::vector<int> currentResearchable;

	for (const auto& currentTech : techList) {
		if (!currentTech.second.activated && isResearchable(currentTech.first)) {
			currentResearchable.push_back(currentTech.first);
		}
	}

	return currentResearchable;
}

// 获取已激活的科技列表
std::vector<int> TechTree::getActivatedTechList() const {
	return activatedTechList;
}

// 获取科技信息
const TechNode* TechTree::getTechInfo(int techId) const {
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end()) {
		return nullptr;
	}
	else {
		return &currentNode->second;
	}
}

// 获取科技当前进度（已投入科技值）
int TechTree::getTechProgress(int techId) const {
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end()) {
		return -1;
	}
	return currentNode->second.progress;
}

// 获取科技成本
int TechTree::getTechCost(int techId) const {
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end()) {
		return -1;
	}
	return currentNode->second.cost;
}

// 添加事件监听器
void TechTree::addEventListener(TechEventListener* listener) {
	if (listener && std::find(listeners.begin(), listeners.end(), listener) == listeners.end()) {
		listeners.push_back(listener);
	}
}

// 移除事件监听器
void TechTree::removeEventListener(TechEventListener* listener) {
	auto it = std::find(listeners.begin(), listeners.end(), listener);
	if (it != listeners.end()) {
		listeners.erase(it);
	}
}

// 回调函数：对科技树内影响，即检查后续科技更新
void TechTree::onTechActivated_internal(int prereqTechId) {
	std::vector<int> newlyActivated;
	newlyActivated.push_back(prereqTechId);

	while (!newlyActivated.empty()) {
		int currentTech = newlyActivated.back();
		newlyActivated.pop_back();

		// 查找所有依赖currentTech的科技
		auto currentNode = techList.find(currentTech);
		if (currentNode != techList.end()) {
			for (int dependentTechId : currentNode->second.dstTechList) {
				auto dependentIt = techList.find(dependentTechId);
				if (dependentIt != techList.end() &&
					!dependentIt->second.activated &&
					dependentIt->second.progress >= dependentIt->second.cost) {

					// 检查所有前置条件
					bool allPrereqsMet = true;
					for (int prereqId : dependentIt->second.srcTechList) {
						auto prereqIt = techList.find(prereqId);
						if (prereqIt == techList.end() || !prereqIt->second.activated) {
							allPrereqsMet = false;
							break;
						}
					}

					if (allPrereqsMet) {
						dependentIt->second.activated = true;
						activatedTechList.push_back(dependentTechId);

						// 通知监听器
						notifyTechActivated(dependentTechId, dependentIt->second.name,
							dependentIt->second.effectDescription);

						newlyActivated.push_back(dependentTechId);
					}
				}
			}
		}
	}
}

// 通知监听器：科技激活
void TechTree::notifyTechActivated(int techId, const std::string& techName,
	const std::string& effect) {
	for (auto listener : listeners) {
		listener->onTechActivated(techId, techName, effect);
	}
}

// 通知监听器：研究进度更新
void TechTree::notifyResearchProgress(int techId, int currentProgress, int totalCost) {
	for (auto listener : listeners) {
		listener->onResearchProgress(techId, currentProgress, totalCost);
	}
}

// 通知监听器：尤里卡触发
void TechTree::notifyEurekaTriggered(int techId, const std::string& techName) {
	for (auto listener : listeners) {
		listener->onEurekaTriggered(techId, techName);
	}
}