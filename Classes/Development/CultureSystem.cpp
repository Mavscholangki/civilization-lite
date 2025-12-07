#include "CultureSystem.h"
#include <iostream>

CultureNode::CultureNode(int id, const std::string& name, const std::vector<int>& prereqs, const std::string& effect)
    : id(id), name(name), progress(0), activated(false), srcCultureList(prereqs),
    effectDescription(effect) {
    std::fill(policySlotCount, policySlotCount + 4, 0);
}

// 初始化市政树函数
void CultureTree::initializeCultureTree() {
    // 清空现有数据
    cultureList.clear();
    activatedCultureList.clear();

    // 第1层：远古时代市政
    cultureList.emplace(101, CultureNode(101, "法典", {}, "解锁基础法律系统，建立社会秩序"));
    cultureList[101].policySlotCount[0] = 1; // 1个军事政策槽

    // 第2层：古典时代市政
    cultureList.emplace(102, CultureNode(102, "技艺", { 101 }, "解锁艺术和手工艺，提升文化产出"));
    cultureList[102].policySlotCount[1] = 1; // 1个经济政策槽

    cultureList.emplace(103, CultureNode(103, "政治哲学", { 101 }, "建立政治理论，解锁新政府形式"));
    cultureList[103].unlockedGovernmentList = { GovernmentType::AUTOCRACY,
                                               GovernmentType::OLIGARCHY };
    cultureList[103].policySlotCount[3] = 1; // 1个通用政策槽

    // 第3层：中世纪市政
    cultureList.emplace(104, CultureNode(104, "公会", { 102 }, "解锁行会制度，促进经济发展"));
    cultureList[104].policySlotCount[1] = 2; // 2个经济政策槽

    cultureList.emplace(105, CultureNode(105, "封建主义", { 103 }, "建立封建制度，强化地方统治"));
    cultureList[105].unlockedGovernmentList = { GovernmentType::MONARCHY };

    cultureList.emplace(106, CultureNode(106, "历史记录", { 103 }, "建立历史档案，提升文化传承"));

    // 第4层：文艺复兴市政
    cultureList.emplace(107, CultureNode(107, "人文主义", { 106 }, "强调人文价值，促进艺术发展"));
    cultureList[107].policySlotCount[2] = 1; // 1个外交政策槽

    // 第5层：工业时代市政
    cultureList.emplace(108, CultureNode(108, "启蒙运动", { 107 }, "提倡理性主义，开启科学革命"));
    cultureList[108].policySlotCount[0] = 2; // 2个军事政策槽
    cultureList[108].policySlotCount[1] = 2; // 2个经济政策槽

    cultureList.emplace(109, CultureNode(109, "意识形态", { 107 }, "形成明确的政治意识形态"));
    cultureList[109].unlockedGovernmentList = { GovernmentType::DEMOCRACY,
                                                GovernmentType::COMMUNISM,
                                                GovernmentType::FASCISM };

    // 第6层：现代市政
    cultureList.emplace(110, CultureNode(110, "城市化", { 108 }, "促进城市发展，提高人口容量"));
    cultureList[110].policySlotCount[3] = 2; // 2个通用政策槽

    // 第7层：信息时代市政
    cultureList.emplace(111, CultureNode(111, "太空竞赛", { 110 }, "开启太空探索，追求科技进步"));
    cultureList[111].unlockedGovernmentList = { GovernmentType::CORPORATE_LIBERTY };

    cultureList.emplace(112, CultureNode(112, "极端主义", { 109 }, "极端政治思想，激进行动"));

    cultureList.emplace(113, CultureNode(113, "全球化", { 110 }, "促进全球合作，加强国际关系"));
    cultureList[113].policySlotCount[2] = 2; // 2个外交政策槽
    cultureList[113].unlockedGovernmentList = { GovernmentType::DIGITAL_DEMOCRACY };

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

// 增加文化进度函数
bool CultureTree::updateProgress(int cultureId, int progress) {
    if (progress <= 0) return false;

    auto currentNode = cultureList.find(cultureId);
    if (currentNode == cultureList.end() || currentNode->second.activated) {
        return false;
    }

    // 前置条件必须满足才能解锁
    if (!isUnlockable(cultureId)) {
        return false;
    }

    // 更新进度
    currentNode->second.progress += progress;

    // 通知进度更新
    notifyCultureProgress(cultureId, currentNode->second.progress);

    // 检查是否完成解锁
    if (currentNode->second.progress >= 100) {
        currentNode->second.activated = true;
        activatedCultureList.push_back(cultureId);

        // 更新政策槽位
        for (int i = 0; i < 4; i++) {
            activePolicySlots[i] = std::max(activePolicySlots[i],
                currentNode->second.policySlotCount[i]);
        }

        // 通知激活
        notifyCultureUnlocked(cultureId, currentNode->second.name, currentNode->second.effectDescription);

        // 处理连锁激活
        onCultureUnlocked_internal(cultureId);
    }

    return true;
}

// 增加文化进度函数[鼓舞特化版]
bool CultureTree::updateProgress_Inspiration(int cultureId) {
    auto it = cultureList.find(cultureId);
    if (it == cultureList.end() || it->second.activated) {
        return false;
    }

    // 立即增加50%进度
    it->second.progress += 50;

    // 通知监听器
    notifyInspirationTriggered(cultureId, it->second.name);

    // 检查是否可解锁（进度>=100且前置条件满足）
    if (it->second.progress >= 100 && isUnlockable(cultureId)) {
        it->second.activated = true;
        activatedCultureList.push_back(cultureId);
        notifyCultureUnlocked(cultureId, it->second.name, it->second.effectDescription);
        onCultureUnlocked_internal(cultureId);
    }

    return true;
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
                    dependentIt->second.progress >= 100) {

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
void CultureTree::notifyCultureProgress(int cultureId, int progress) {
    for (auto listener : listeners) {
        listener->onCultureProgress(cultureId, progress);
    }
}

// 通知监听器：灵感触发
void CultureTree::notifyInspirationTriggered(int cultureId, const std::string& cultureName) {
    for (auto listener : listeners) {
        listener->onInspirationTriggered(cultureId, cultureName);
    }
}