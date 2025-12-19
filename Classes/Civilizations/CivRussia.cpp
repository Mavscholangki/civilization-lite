// CivRussia.cpp
#include "CivRussia.h"
#include "District/Base/District.h"

bool CivRussia::init() {
    if (!BaseCiv::init()) {
        return false;
    }

    // 设置俄罗斯特性
    m_traits.name = "彼得大帝";
    m_traits.description = "初始领土扩大（+8地块），学院和剧院广场产出+20%，军事单位生产成本降低";
    m_traits.initialTiles = 8;               // 初始地块+8
    m_traits.scienceBonus = 1.2f;           // 科研加成20%
    m_traits.cultureBonus = 1.2f;           // 文化加成20%
    m_traits.militaryProductionBonus = 0.8f; // 军事生产力成本减少20%
    m_traits.halfCostIndustrial = false;
    m_traits.extraDistrictSlot = false;
    m_traits.eurekaBoost = 0.5f;
    m_traits.inspirationBoost = 0.5f;
    m_traits.builderCharges = 3;
    m_traits.productionBonus = 1.0f;
    m_traits.goldBonus = 1.0f;
    m_traits.faithBonus = 1.0f;

    return true;
}

CivilizationTrait CivRussia::getTraits() const {
    return m_traits;
}

Yield CivRussia::calculateDistrictBonus(const District* district) const {
    Yield bonus = { 0, 0, 0, 0, 0 };

    if (district == nullptr) {
        return bonus;
    }

    // 获取区域类型
    District::DistrictType districtType = district->getType();

    // 如果是学院区域，提供额外的科研加成
    if (districtType.typeName == District::DistrictTypeName::CAMPUS) {
        // 获取基础产出
        Yield baseYield = district->getYield();
        bonus.scienceYield = static_cast<int>(baseYield.scienceYield * 0.2f); // +20%
    }

    // 如果是剧院广场区域，提供额外的文化加成
    if (districtType.typeName == District::DistrictTypeName::THEATER_SQUARE) {
        Yield baseYield = district->getYield();
        bonus.cultureYield = static_cast<int>(baseYield.cultureYield * 0.2f); // +20%
    }

    return bonus;
}