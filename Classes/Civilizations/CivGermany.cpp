// CivGermany.cpp
#include "CivGermany.h"

bool CivGermany::init() {
    if (!BaseCiv::init()) {
        return false;
    }

    // 设置德国特性
    m_traits.name = "神圣罗马皇帝";
    m_traits.description = "工业区建设成本减半，每个城市可建造的区域上限+1";
    m_traits.halfCostIndustrial = true;
    m_traits.extraDistrictSlot = true;
    m_traits.initialTiles = 3;
    m_traits.eurekaBoost = 0.5f;
    m_traits.inspirationBoost = 0.5f;
    m_traits.builderCharges = 3;
    m_traits.scienceBonus = 1.0f;
    m_traits.cultureBonus = 1.0f;
    m_traits.militaryProductionBonus = 1.0f;

    return true;
}

CivilizationTrait CivGermany::getTraits() const {
    return m_traits;
}

float CivGermany::calculateDistrictCost(const std::string& districtType) const {
    // 检查是否为工业区（需要根据实际项目中的区域类型标识调整）
    if (districtType == "INDUSTRY_ZONE" ||
        districtType == "IndustrialZone" ||
        districtType == "工业区") {
        return 0.5f; // 半价
    }

    // 其他区域按原价
    return 1.0f;
}

int CivGermany::calculateMaxDistricts(int population) const {
    // 德国特殊能力：区域上限+1
    // 默认公式：人口除以3（向下取整）再加1，德国额外+1
    int baseDistricts = BaseCiv::calculateMaxDistricts(population);
    return baseDistricts + 1; // 德国额外+1
}