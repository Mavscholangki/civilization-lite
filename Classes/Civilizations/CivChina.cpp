// CivChina.cpp
#include "CivChina.h"
#include "../Development/TechSystem.h"
#include <City/Yield.h>

bool CivChina::init() {
    if (!BaseCiv::init()) {
        return false;
    }

    // 设置中国特性
    m_traits.name = "始皇帝";
    m_traits.description = "尤里卡和灵感加成提升75%，建造者有5次使用次数，拥有特殊单位虎蹲炮";
    m_traits.eurekaBoost = 0.75f; // 75%尤里卡加成
    m_traits.inspirationBoost = 0.75f; // 75%灵感加成
    m_traits.builderCharges = 5;
    m_traits.initialTiles = 3; // 默认值
    m_traits.scienceBonus = 1.0f;
    m_traits.cultureBonus = 1.0f;
    m_traits.halfCostIndustrial = false;
    m_traits.extraDistrictSlot = false;
    m_traits.militaryProductionBonus = 1.0f;

    // 添加特殊单位到列表
    m_uniqueUnits.push_back("虎蹲炮");

    return true;
}

CivilizationTrait CivChina::getTraits() const {
    return m_traits;
}

bool CivChina::hasUniqueUnit(const std::string& unitName) const {
    for (const auto& unit : m_uniqueUnits) {
        if (unit == unitName) {
            return true;
        }
    }
    return false;
}

bool CivChina::isUniqueUnitUnlocked(const std::string& unitName) const {
    if (unitName != "虎蹲炮") {
        return false;
    }

    // 虎蹲炮需要机械科技（ID:11）解锁
    // 注意：这里需要访问科技树，但文明类通常不直接持有科技树
    // 这个检查应该在Player类中完成，使用Player的科技树
    // 这里暂时返回true，实际应在Player中判断
    return true;
}

cocos2d::Ref* CivChina::createUniqueUnit(const std::string& unitName, void* position) {
    if (unitName == "虎蹲炮") {
        // 这里应该创建虎蹲炮单位
        // TigerCannonUnit::create(position);
        // 暂时返回nullptr，待单位系统完善
        return nullptr;
    }
    return nullptr;
}

Yield CivChina::calculateDistrictBonus(const District* district) const {
    // 中国没有特殊区域加成，使用基类实现
    return BaseCiv::calculateDistrictBonus(district);
}