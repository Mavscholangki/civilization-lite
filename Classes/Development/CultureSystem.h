#ifndef CULTURE_SYSTEM_H
#define CULTURE_SYSTEM_H

#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

// 政体枚举
enum class GovernmentType {
	CHIEFDOM,           // 酋邦
	AUTOCRACY,          // 独裁统治  
	OLIGARCHY,          // 寡头政体
	CLASSICAL_REPUBLIC, // 古典共和
	MONARCHY,           // 君主制
	THEOCRACY,          // 神权政治
	MERCHANT_REPUBLIC,  // 商人共和国
	DEMOCRACY,          // 民主制
	COMMUNISM,          // 共产主义
	FASCISM,            // 法西斯主义
	CORPORATE_LIBERTY,  // 公司自由制
	DIGITAL_DEMOCRACY   // 数字民主
};

// 市政节点结构
struct CultureNode {
	int id;
	std::string name;
	int cost;                       // 所需文化值（总成本）- 新增字段
	int progress;					// 已投入文化值（从百分比改为具体数值）
	bool activated;					// 解锁状态
	std::vector<int> srcCultureList;	// 前置市政ID
	std::vector<int> dstCultureList;	// 后继市政ID
	std::string effectDescription;		// 效果描述

	// 市政解锁的政体
	std::vector<GovernmentType> unlockedGovernmentList;
	// 市政解锁的政策卡槽位
	int policySlotCount[4]; // 4个索引依次是[军事,经济,外交,通用]
	// 新增：解锁的政策卡ID列表
	std::vector<int> unlockedPolicyIds;

	// 新增：政策槽位类型枚举
	enum class SlotType { MILITARY, ECONOMIC, DIPLOMATIC, WILDCARD };

	CultureNode() = default;// 防止系统调用出错
	CultureNode(int id, const std::string& name, int cost, const std::vector<int>& prereqs, const std::string& effect = "");
};

// 市政事件监听器接口
class CultureEventListener {
public:
	virtual ~CultureEventListener() {}

	// 市政被解锁
	virtual void onCultureUnlocked(int cultureId, const std::string& cultureName,
		const std::string& effect) = 0;

	// 文化进度更新
	virtual void onCultureProgress(int cultureId, int currentProgress, int totalCost) = 0; // 修改参数

	// 灵感被触发
	virtual void onInspirationTriggered(int cultureId, const std::string& cultureName) = 0;
};

// 市政树管理类
class CultureTree {
private:
	std::unordered_map<int, CultureNode> cultureList;	// ID到节点的映射
	std::vector<int> activatedCultureList;				// 已激活市政列表
	GovernmentType currentGovernment;					// 当前政体
	int activePolicySlots[4];							// 当前激活的政策槽位
	int currentResearchCulture;							// 当前正在研究的市政ID

	std::vector<CultureEventListener*> listeners;

public:
	CultureTree() {
		initializeCultureTree();
		currentGovernment = GovernmentType::CHIEFDOM;
		std::fill(activePolicySlots, activePolicySlots + 4, 0);
		currentResearchCulture = -1; // 初始没有研究
	}

	~CultureTree() {
		listeners.clear();
	}

	// 写操作
	void initializeCultureTree();

	// 设置当前研究的市政
	bool setCurrentResearch(int cultureId);
	int getCurrentResearch() const { return currentResearchCulture; }

	// 增加文化点数（与科技树接口一致）
	void updateProgress(int points);

	// 触发灵感（与科技树接口一致）
	void updateProgress_Inspiration(int cultureId);

	// 政体相关
	bool switchGovernment(GovernmentType newGovernment);
	GovernmentType getCurrentGovernment() const { return currentGovernment; }

	// 是非查询
	bool isUnlockable(int cultureId) const;
	bool isActivated(int cultureId) const;
	bool isGovernmentUnlocked(GovernmentType government) const;

	// 获取研究进度百分比（与科技树接口一致）
	int getResearchProgressPercent(int cultureId) const;

	// 读操作
	std::vector<int> getUnlockableCultureList() const;
	std::vector<int> getActivatedCultureList() const;
	const CultureNode* getCultureInfo(int cultureId) const;
	int getCultureProgress(int cultureId) const;
	int getCultureCost(int cultureId) const; // 新增：获取文化成本
	const int* getActivePolicySlots() const { return activePolicySlots; }

	// 新增：获取已解锁的政策ID（供政策系统调用）
	std::vector<int> getUnlockedPolicyIds() const;

	// 新增：根据文化ID获取解锁的政策ID
	std::vector<int> getPoliciesUnlockedByCulture(int cultureId) const;

	// 新增：获取当前可用的政策槽位信息
	struct PolicySlotInfo {
		int military;
		int economic;
		int diplomatic;
		int wildcard;
	};

	PolicySlotInfo getPolicySlotInfo() const {
		return { activePolicySlots[0], activePolicySlots[1],
				activePolicySlots[2], activePolicySlots[3] };
	}

	// 事件监听器管理
	void addEventListener(CultureEventListener* listener);
	void removeEventListener(CultureEventListener* listener);

private:
	void onCultureUnlocked_internal(int prereqCultureId);

	// 内部函数
	void addProgressToCulture(int cultureId, int points);
	void activateCulture(int cultureId);

	// 通知监听器
	void notifyCultureUnlocked(int cultureId, const std::string& cultureName, const std::string& effect);
	void notifyCultureProgress(int cultureId, int progress, int totalCost); // 修改参数
	void notifyInspirationTriggered(int cultureId, const std::string& cultureName);
};

#endif // CULTURE_SYSTEM_H