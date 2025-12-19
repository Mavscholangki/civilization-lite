#ifndef TECH_SYSTEM_H
#define TECH_SYSTEM_H

#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

// 科技节点结构
struct TechNode {
	int id;
	std::string name;
	int cost;                       // 所需科技值（总成本）
	int progress;                   // 已投入科技值
	bool activated;					// 激活状态
	std::vector<int> srcTechList;	// 前置科技ID
	std::vector<int> dstTechList;	// 后继科技ID
	std::string effectDescription;  // 效果描述

	TechNode() = default;// 防止系统调用出错
	TechNode(int id, const std::string& name, int cost, const std::vector<int>& prereqs, const std::string& effect = "");
};

// 科技事件监听器接口
class TechEventListener {
public:
	virtual ~TechEventListener() {}

	// 科技被激活
	virtual void onTechActivated(int techId, const std::string& techName, const std::string& effect) = 0;

	// 研究进度更新
	virtual void onResearchProgress(int techId, int currentProgress, int totalCost) = 0;

	// 尤里卡被触发
	virtual void onEurekaTriggered(int techId, const std::string& techName) = 0;
};

// 科技树管理类
class TechTree {
private:
	std::unordered_map<int, TechNode> techList; // ID到节点的映射
	std::vector<int> activatedTechList;         // 已激活节点列表
	int currentResearchTech;                    // 当前正在研究的科技ID

	std::vector<TechEventListener*> listeners;

public:
	TechTree() : currentResearchTech(-1) {
		initializeTechTree();
	}

	~TechTree() {
		listeners.clear();
	}

	// 初始化
	void initializeTechTree();

	// 设置当前研究的科技（由玩家选择）
	bool setCurrentResearch(int techId);
	int getCurrentResearch() const { return currentResearchTech; }

	// 增加科研点数（由游戏回合系统调用）
	void updateProgress(int points);

	// 触发尤里卡（由事件系统调用）
	void updateProgress_Eureka(int techId);

	// 查询函数
	bool isResearchable(int techId) const;
	bool isActivated(int techId) const;
	int getResearchProgressPercent(int techId) const; // 获取研究进度百分比

	// 读操作
	std::vector<int> getResearchableTechList() const;
	std::vector<int> getActivatedTechList() const;
	const TechNode* getTechInfo(int techId) const;
	int getTechProgress(int techId) const;
	int getTechCost(int techId) const;

	// 事件监听器管理
	void addEventListener(TechEventListener* listener);
	void removeEventListener(TechEventListener* listener);

private:
	// 内部更新函数
	void addProgressToTech(int techId, int points);
	void activateTech(int techId);

	// 内部回调函数
	void onTechActivated_internal(int prereqTechId);

	// 通知监听器
	void notifyTechActivated(int techId, const std::string& techName, const std::string& effect);
	void notifyResearchProgress(int techId, int currentProgress, int totalCost);
	void notifyEurekaTriggered(int techId, const std::string& techName);
};

#endif // TECH_SYSTEM_H