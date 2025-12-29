#include "CultureSystem.h"
#include <iostream>

CultureNode::CultureNode(int id, const std::string& name, int cost, const std::vector<int>& prereqs, const std::string& effect)
	: id(id), name(name), cost(cost), progress(0), activated(false), srcCultureList(prereqs),
	effectDescription(effect) {
	std::fill(policySlotCount, policySlotCount + 4, 0);
}

// 初始化市政树函数
void CultureTree::initializeCultureTree() {
	// 清空现有数据
	cultureList.clear();
	activatedCultureList.clear();
	currentResearchCulture = -1;

	// 第1层：远古时代市政
	cultureList.emplace(101, CultureNode(101, u8"法典", 25, {}, u8"解锁基础法律系统，建立社会秩序"));
	cultureList[101].policySlotCount[0] = 1; // 1个军事政策槽
	cultureList[101].unlockedPolicyIds = { 1001, 2001 }; // 征兵, 城市规划

	// 第2层：古典时代市政
	cultureList.emplace(102, CultureNode(102, u8"技艺", 50, { 101 }, u8"解锁艺术和手工艺，提升文化产出"));
	cultureList[102].policySlotCount[1] = 1; // 1个经济政策槽
	cultureList[102].unlockedPolicyIds = { 2002 }; // 贸易联盟

	cultureList.emplace(103, CultureNode(103, u8"政治哲学", 50, { 101 }, u8"建立政治理论，解锁新政府形式"));
	cultureList[103].unlockedGovernmentList = { GovernmentType::AUTOCRACY, GovernmentType::OLIGARCHY };
	cultureList[103].policySlotCount[3] = 1; // 1个通用政策槽
	cultureList[103].unlockedPolicyIds = { 3001, 4001 }; // 外交联盟, 启示录

	// 第3层：中世纪市政
	cultureList.emplace(104, CultureNode(104, u8"公会", 80, { 102 }, u8"解锁行会制度，促进经济发展"));
	cultureList[104].policySlotCount[1] = 2; // 2个经济政策槽
	cultureList[104].unlockedPolicyIds = { 2003 }; // 市场经济

	cultureList.emplace(105, CultureNode(105, u8"封建主义", 80, { 103 }, u8"建立封建制度，强化地方统治"));
	cultureList[105].unlockedGovernmentList = { GovernmentType::MONARCHY };
	cultureList[105].unlockedPolicyIds = { 1002 }; // 军团

	cultureList.emplace(106, CultureNode(106, u8"历史记录", 80, { 103 }, u8"建立历史档案，提升文化传承"));
	cultureList[106].unlockedPolicyIds = { 4002 }; // 哲学

	// 第4层：文艺复兴市政
	cultureList.emplace(107, CultureNode(107, u8"人文主义", 120, { 106 }, u8"强调人文价值，促进艺术发展"));
	cultureList[107].policySlotCount[2] = 1; // 1个外交政策槽
	cultureList[107].unlockedPolicyIds = { 3002 }; // 启蒙思想

	// 第5层：工业时代市政
	cultureList.emplace(108, CultureNode(108, u8"启蒙运动", 200, { 107 }, u8"提倡理性主义，开启科学革命"));
	cultureList[108].policySlotCount[0] = 2; // 2个军事政策槽
	cultureList[108].policySlotCount[1] = 2; // 2个经济政策槽
	cultureList[108].unlockedPolicyIds = { 1004, 1005 }; // 纪律, 佣兵制

	cultureList.emplace(109, CultureNode(109, u8"意识形态", 200, { 107 }, u8"形成明确的政治意识形态"));
	cultureList[109].unlockedGovernmentList = { GovernmentType::DEMOCRACY, GovernmentType::COMMUNISM, GovernmentType::FASCISM };
	cultureList[109].unlockedPolicyIds = { 4003 }; // 黄金时代

	// 第6层：现代市政
	cultureList.emplace(110, CultureNode(110, u8"城市化", 300, { 108 }, u8"促进城市发展，提高人口容量"));
	cultureList[110].policySlotCount[3] = 2; // 2个通用政策槽
	cultureList[110].unlockedPolicyIds = { 2004 }; // 工业革命

	// 第7层：信息时代市政
	cultureList.emplace(111, CultureNode(111, u8"太空竞赛", 400, { 110 }, u8"开启太空探索，追求科技进步"));
	cultureList[111].unlockedGovernmentList = { GovernmentType::CORPORATE_LIBERTY };
	cultureList[111].unlockedPolicyIds = { 4004 }; // 科学革命

	cultureList.emplace(112, CultureNode(112, u8"极端主义", 400, { 109 }, u8"极端政治思想，激进行动"));
	// 112不分配政策卡，代表负面效应

	cultureList.emplace(113, CultureNode(113, u8"全球化", 400, { 110 }, u8"促进全球合作，加强国际关系"));
	cultureList[113].policySlotCount[2] = 2; // 2个外交政策槽
	cultureList[113].unlockedGovernmentList = { GovernmentType::DIGITAL_DEMOCRACY };
	cultureList[113].unlockedPolicyIds = { 3003 }; // 国际合作

	// 设置解锁关系
	// 第1层解锁第2层
	cultureList[101].dstCultureList = { 102, 103 };

	// 第2层解锁第3层
	cultureList[102].dstCultureList = { 104 };
	cultureList[103].dstCultureList = { 105, 106 };

	// 第3层解锁第4层
	cultureList[106].dstCultureList = { 107 };

	// 第4层解锁第5层
	cultureList[107].dstCultureList = { 108, 109 };

	// 第5层解锁第6层
	cultureList[108].dstCultureList = { 110 };
	cultureList[109].dstCultureList = { 112 };

	// 第6层解锁第7层
	cultureList[110].dstCultureList = { 111, 113 };

	// 添加一些额外的连接，让市政树更丰富
	cultureList[104].dstCultureList.push_back(108); // 公会 -> 启蒙运动
	cultureList[105].dstCultureList.push_back(109); // 封建主义 -> 意识形态
}

// 设置当前研究的市政
bool CultureTree::setCurrentResearch(int cultureId) {
	auto it = cultureList.find(cultureId);
	if (it == cultureList.end()) {
		return false;
	}

	// 检查市政是否已激活或可解锁
	if (it->second.activated || !isUnlockable(cultureId)) {
		return false;
	}

	currentResearchCulture = cultureId;
	return true;
}

// 增加文化点数（与科技树接口一致）
void CultureTree::updateProgress(int points) {
	if (currentResearchCulture <= 0 || points <= 0) {
		return;
	}

	addProgressToCulture(currentResearchCulture, points);
}

// 触发灵感（与科技树接口一致）
void CultureTree::updateProgress_Inspiration(int cultureId) {
	auto it = cultureList.find(cultureId);
	if (it == cultureList.end() || it->second.activated) {
		return;
	}

	// 灵感：增加该市政50%成本的文化值（与科技树尤里卡一致）
	int inspirationPoints = it->second.cost / 2;

	// 通知监听器
	notifyInspirationTriggered(cultureId, it->second.name);

	// 增加进度
	addProgressToCulture(cultureId, inspirationPoints);
}

// 内部函数：向市政添加进度
void CultureTree::addProgressToCulture(int cultureId, int points) {
	auto it = cultureList.find(cultureId);
	if (it == cultureList.end() || it->second.activated) {
		return;
	}

	// 检查前置条件（除非有灵感，否则必须有前置条件）
	if (!isUnlockable(cultureId)) {
		return;
	}

	// 增加进度，但不能超过成本
	int newProgress = it->second.progress + points;
	if (newProgress > it->second.cost) {
		newProgress = it->second.cost;
	}

	it->second.progress = newProgress;

	// 通知进度更新
	notifyCultureProgress(cultureId, newProgress, it->second.cost);

	// 检查是否完成解锁
	if (newProgress >= it->second.cost) {
		activateCulture(cultureId);
	}
}

// 激活市政
void CultureTree::activateCulture(int cultureId) {
	auto it = cultureList.find(cultureId);
	if (it == cultureList.end() || it->second.activated) {
		return;
	}

	it->second.activated = true;
	activatedCultureList.push_back(cultureId);

	// 更新政策槽位
	for (int i = 0; i < 4; i++) {
		activePolicySlots[i] = std::max(activePolicySlots[i],
			it->second.policySlotCount[i]);
	}

	if (currentResearchCulture == cultureId) {
		currentResearchCulture = -1;
	}

	// 通知监听器（包括政策系统的监听器）
	notifyCultureUnlocked(cultureId, it->second.name, it->second.effectDescription);

	onCultureUnlocked_internal(cultureId);
}

// 检查是否可解锁函数
bool CultureTree::isUnlockable(int cultureId) const {
	auto currentNode = cultureList.find(cultureId);
	if (currentNode == cultureList.end() || currentNode->second.activated) {
		return false;
	}

	// 检查所有前置条件
	for (int srcId : currentNode->second.srcCultureList) {
		auto prereqNode = cultureList.find(srcId);
		if (prereqNode == cultureList.end() || !prereqNode->second.activated) {
			return false;
		}
	}

	return true;
}

// 检查市政是否解锁函数
bool CultureTree::isActivated(int cultureId) const {
	auto currentNode = cultureList.find(cultureId);
	return (currentNode != cultureList.end() && currentNode->second.activated);
}

// 获取研究进度百分比
int CultureTree::getResearchProgressPercent(int cultureId) const {
	auto currentNode = cultureList.find(cultureId);
	if (currentNode == cultureList.end() || currentNode->second.cost == 0) {
		return 0;
	}

	return static_cast<int>((currentNode->second.progress * 100) / currentNode->second.cost);
}

// 获取可解锁的市政列表函数
std::vector<int> CultureTree::getUnlockableCultureList() const {
	std::vector<int> currentUnlockable;

	for (const auto& currentCulture : cultureList) {
		if (!currentCulture.second.activated && isUnlockable(currentCulture.first)) {
			currentUnlockable.push_back(currentCulture.first);
		}
	}

	return currentUnlockable;
}

// 获取已解锁的市政列表函数
std::vector<int> CultureTree::getActivatedCultureList() const {
	return activatedCultureList;
}

// 获取市政信息函数
const CultureNode* CultureTree::getCultureInfo(int cultureId) const {
	auto currentNode = cultureList.find(cultureId);
	if (currentNode == cultureList.end()) {
		return nullptr;
	}
	else {
		return &currentNode->second;
	}
}

// 获取市政当前进度
int CultureTree::getCultureProgress(int cultureId) const {
	auto currentNode = cultureList.find(cultureId);
	if (currentNode == cultureList.end()) {
		return -1;
	}
	return currentNode->second.progress;
}

// 获取市政成本
int CultureTree::getCultureCost(int cultureId) const {
	auto currentNode = cultureList.find(cultureId);
	if (currentNode == cultureList.end()) {
		return -1;
	}
	return currentNode->second.cost;
}

// 切换政体函数
bool CultureTree::switchGovernment(GovernmentType newGovernment) {
	// 检查是否解锁了该政体
	if (!isGovernmentUnlocked(newGovernment)) {
		return false;
	}

	// 切换政体
	currentGovernment = newGovernment;
	return true;
}

// 检查政体是否解锁
bool CultureTree::isGovernmentUnlocked(GovernmentType government) const {
	// 遍历所有已解锁的市政，检查是否解锁了该政体
	for (int cultureId : activatedCultureList) {
		auto it = cultureList.find(cultureId);
		if (it != cultureList.end()) {
			for (GovernmentType unlockedGov : it->second.unlockedGovernmentList) {
				if (unlockedGov == government) {
					return true;
				}
			}
		}
	}
	return false;
}

// 获取所有已解锁的政策ID
std::vector<int> CultureTree::getUnlockedPolicyIds() const {
	std::vector<int> allPolicyIds;
	for (int cultureId : activatedCultureList) {
		auto it = cultureList.find(cultureId);
		if (it != cultureList.end()) {
			for (int policyId : it->second.unlockedPolicyIds) {
				allPolicyIds.push_back(policyId);
			}
		}
	}

	// 去重
	std::sort(allPolicyIds.begin(), allPolicyIds.end());
	allPolicyIds.erase(std::unique(allPolicyIds.begin(), allPolicyIds.end()),
		allPolicyIds.end());

	return allPolicyIds;
}

// 根据文化ID获取解锁的政策
std::vector<int> CultureTree::getPoliciesUnlockedByCulture(int cultureId) const {
	auto it = cultureList.find(cultureId);
	if (it != cultureList.end() && it->second.activated) {
		return it->second.unlockedPolicyIds;
	}
	return {};
}

// 添加事件监听器
void CultureTree::addEventListener(CultureEventListener* listener) {
	if (listener && std::find(listeners.begin(), listeners.end(), listener) == listeners.end()) {
		listeners.push_back(listener);
	}
}

// 移除事件监听器
void CultureTree::removeEventListener(CultureEventListener* listener) {
	auto it = std::find(listeners.begin(), listeners.end(), listener);
	if (it != listeners.end()) {
		listeners.erase(it);
	}
}

// 回调函数：对市政树内影响
void CultureTree::onCultureUnlocked_internal(int prereqCultureId) {
	std::vector<int> newlyUnlocked;
	newlyUnlocked.push_back(prereqCultureId);

	while (!newlyUnlocked.empty()) {
		int currentCulture = newlyUnlocked.back();
		newlyUnlocked.pop_back();

		// 查找所有依赖currentCulture的市政
		auto currentNode = cultureList.find(currentCulture);
		if (currentNode != cultureList.end()) {
			for (int dependentCultureId : currentNode->second.dstCultureList) {
				auto dependentIt = cultureList.find(dependentCultureId);
				if (dependentIt != cultureList.end() &&
					!dependentIt->second.activated &&
					dependentIt->second.progress >= dependentIt->second.cost) {

					// 检查所有前置条件
					bool allPrereqsMet = true;
					for (int prereqId : dependentIt->second.srcCultureList) {
						auto prereqIt = cultureList.find(prereqId);
						if (prereqIt == cultureList.end() || !prereqIt->second.activated) {
							allPrereqsMet = false;
							break;
						}
					}

					if (allPrereqsMet) {
						dependentIt->second.activated = true;
						activatedCultureList.push_back(dependentCultureId);

						// 更新政策槽位
						for (int i = 0; i < 4; i++) {
							activePolicySlots[i] = std::max(activePolicySlots[i],
								dependentIt->second.policySlotCount[i]);
						}

						// 如果当前正在研究这个市政，重置当前研究
						if (currentResearchCulture == dependentCultureId) {
							currentResearchCulture = -1;
						}

						notifyCultureUnlocked(dependentCultureId,
							dependentIt->second.name,
							dependentIt->second.effectDescription);
						newlyUnlocked.push_back(dependentCultureId);
					}
				}
			}
		}
	}
}

// 通知监听器：市政解锁
void CultureTree::notifyCultureUnlocked(int cultureId, const std::string& cultureName, const std::string& effect) {
	for (auto listener : listeners) {
		listener->onCultureUnlocked(cultureId, cultureName, effect);
	}
}

// 通知监听器：文化进度更新
void CultureTree::notifyCultureProgress(int cultureId, int progress, int totalCost) {
	for (auto listener : listeners) {
		listener->onCultureProgress(cultureId, progress, totalCost);
	}
}

// 通知监听器：灵感触发
void CultureTree::notifyInspirationTriggered(int cultureId, const std::string& cultureName) {
	for (auto listener : listeners) {
		listener->onInspirationTriggered(cultureId, cultureName);
	}
}