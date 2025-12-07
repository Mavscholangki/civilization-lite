#include "TechSystem.h"

TechNode::TechNode(int id, const std::string& name, const std::vector<int>& prereqs, const std::string& effect)
	: id(id), name(name), progress(0), activated(false), srcTechList(prereqs), effectDescription(effect) {

}

// 初始化科技树函数：目前采用硬编码
void TechTree::initializeTechTree() {
	// 远古时代科技
	techList.emplace(1, TechNode(1, "农业", {}, "解锁建造者，允许建造农场"));
	techList.emplace(2, TechNode(2, "畜牧", { 1 }, "允许建造牧场，提供骑兵单位"));
	techList.emplace(3, TechNode(3, "采矿", { 1 }, "允许开采资源，建造矿场"));
	techList.emplace(4, TechNode(4, "制陶", { 1 }, "解锁圣地区域"));
	techList.emplace(5, TechNode(5, "弓箭", { 2 }, "解锁弓箭手单位"));

	// 古典时代科技  
	techList.emplace(6, TechNode(6, "文字", { 4 }, "解锁图书馆，开启文化树"));
	techList.emplace(7, TechNode(7, "青铜术", { 3 }, "解锁剑士单位"));
	techList.emplace(8, TechNode(8, "骑术", { 2, 5 }, "解锁骑兵单位"));
	techList.emplace(9, TechNode(9, "数学", { 6 }, "解锁区域，提高建造效率"));

	// 设置解锁关系
	techList[1].dstTechList = { 2, 3, 4, 5 };
	techList[2].dstTechList = { 5, 8 };
	techList[3].dstTechList = { 7 };
	techList[4].dstTechList = { 6 };
	techList[5].dstTechList = { 8 };
	techList[6].dstTechList = { 9 };
}

// 检查是否可研究函数：检查前置条件是否满足
bool TechTree::isUnlockable(int techId) const {
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end() || currentNode->second.activated) {
		return false; // 如果节点不存在或已激活，一定不可研究
	}

	// 检查所有前置条件
	for (int srcId : currentNode->second.srcTechList) {
		auto prereqNode = techList.find(srcId);
		if (prereqNode == techList.end() || !prereqNode->second.activated) {
			return false; // 前置条件不存在或前置条件自身未激活，则不允许研究
		}
	}

	return true;
}

// 增加科技进度函数：如果条件足够研究科技，更新进度；否则什么也不做
void TechTree::updateProgress(int techId, int progress) {
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end() || currentNode->second.activated) {
		return;
	}

	// 前置条件必须满足才能研究
	if (!isUnlockable(techId)) {
		return;
	}

	// 更新进度（可能已经有尤里卡的50%）
	currentNode->second.progress += progress;
	// 通知监听器
	notifyResearchProgress(techId, currentNode->second.progress);

	// 检查是否完成研究
	if (currentNode->second.progress >= 100) {
		currentNode->second.activated = true;
		activatedTechList.push_back(techId);

		// 通知监听器
		notifyTechActivated(techId, currentNode->second.name, currentNode->second.effectDescription);

		// 对内更新后续进度已满但前置条件现在才满足的科技
		onTechActivated_intern(techId);
	}
}

// 增加科技进度函数[尤里卡特化版本]
void TechTree::updateProgress_Eureka(int techId)
{
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end() || currentNode->second.activated) {
		return;
	}

	// 立即增加50%进度，无论前置条件是否满足
	currentNode->second.progress += 50;

	// 通知监听器
	notifyEurekaTriggered(techId, currentNode->second.name);

	// 如果进度达到100%，但前置条件不满足，暂时不能激活
	if (currentNode->second.progress >= 100 && isUnlockable(techId)) {
		// 如果前置条件满足，立即激活
		currentNode->second.activated = true;
		activatedTechList.push_back(techId);

		// 通知监听器
		notifyTechActivated(techId, currentNode->second.name, currentNode->second.effectDescription);

		onTechActivated_intern(techId);
	}
}

// 检查科技是否激活函数
bool TechTree::isActivated(int techId) const {
	auto currentNode = techList.find(techId);

	// 确认已激活的条件：找到节点，且该节点已激活
	return (currentNode != techList.end() && currentNode->second.activated);
}

// 获取可研究的科技列表函数
std::vector<int> TechTree::getUnlockableTechList() const {
	std::vector<int> currentUnlockable;

	for (const auto& currentTech : techList) {
		if (!currentTech.second.activated && isUnlockable(currentTech.first)) {
			currentUnlockable.push_back(currentTech.first);
		}
	}

	return currentUnlockable;
}

// 获取已解锁的科技列表函数
std::vector<int> TechTree::getActivatedTechList() const
{
	return activatedTechList;
}

// 获取科技信息函数
const TechNode* TechTree::getTechInfo(int techId) const {
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end())
	{
		return nullptr;
	}
	else
	{
		return &currentNode->second;
	}
}

int TechTree::getTechProgress(int techId) const
{
	auto currentNode = techList.find(techId);
	if (currentNode == techList.end()) {
		return -1;
	}
	return currentNode->second.progress;
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
void TechTree::onTechActivated_intern(int prereqTechId) {
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
					dependentIt->second.progress >= 100) {

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
						notifyTechActivated(dependentTechId, dependentIt->second.name, dependentIt->second.effectDescription);
						
						newlyActivated.push_back(dependentTechId);
					}
				}
			}
		}
	}
}

// 通知监听器：科技激活
void TechTree::notifyTechActivated(int techId, const std::string& techName, const std::string& effect) {
	for (auto listener : listeners) {
		listener->onTechActivated(techId, techName, effect);
	}
}

// 通知监听器：研究进度更新
void TechTree::notifyResearchProgress(int techId, int progress) {
	for (auto listener : listeners) {
		listener->onResearchProgress(techId, progress);
	}
}

// 通知监听器：尤里卡触发
void TechTree::notifyEurekaTriggered(int techId, const std::string& techName) {
	for (auto listener : listeners) {
		listener->onEurekaTriggered(techId, techName);
	}
}