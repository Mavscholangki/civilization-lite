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
	int progress;                   // 研究进度
	bool activated;					// 激活状态
	std::vector<int> srcTechList;	// 前置科技ID
	std::vector<int> dstTechList;	// 后继科技ID
	std::string effectDescription;  // 效果描述

	TechNode() = default;// 防止系统调用出错
	TechNode(int id, const std::string& name, const std::vector<int>& prereqs, const std::string& effect = "");
};

// 科技事件监听器接口
class TechEventListener {
public:
	virtual ~TechEventListener() {}

	// 科技被激活
	virtual void onTechActivated(int techId, const std::string& techName, const std::string& effect) = 0;

	// 研究进度更新
	virtual void onResearchProgress(int techId, int currentProgress) = 0;

	// 尤里卡被触发
	virtual void onEurekaTriggered(int techId, const std::string& techName) = 0;
};

// 科技树管理类
class TechTree {
private:
	std::unordered_map<int, TechNode> techList; // ID到节点的映射
	std::vector<int> activatedTechList;         // 已激活节点列表

	std::vector<TechEventListener*> listeners;

public:
	TechTree() {
		initializeTechTree();
	}

	~TechTree() {
		listeners.clear();
	}

	// 写操作
	void initializeTechTree();
	void updateProgress(int techId, int progress);
	void updateProgress_Eureka(int techId);

	// 是非查询
	bool isUnlockable(int techId) const;
	bool isActivated(int techId) const;

	// 读操作
	std::vector<int> getUnlockableTechList() const;
	std::vector<int> getActivatedTechList() const;
	const TechNode* getTechInfo(int techId) const;
	int getTechProgress(int techId) const;

	// 事件监听器管理
	void addEventListener(TechEventListener* listener);
	void removeEventListener(TechEventListener* listener);

private:
	void onTechActivated_intern(int prereqTechId);

	// 通知监听器
	void notifyTechActivated(int techId, const std::string& techName, const std::string& effect);
	void notifyResearchProgress(int techId, int progress);
	void notifyEurekaTriggered(int techId, const std::string& techName);
};

#endif // TECH_SYSTEM_H