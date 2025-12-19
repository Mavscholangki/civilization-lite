// CivRussia.cpp
#include "CivRussia.h"
#include "District/Base/District.h"

CivRussia::CivRussia() : BaseCiv("Russia", "Peter") {
    civType = CivilizationType::RUSSIA;

    // 设置俄罗斯特性
    traits.name = "彼得大帝";
    traits.description = "初始领土扩大（+8地块），学院和剧院广场产出+20%，军事单位生产成本降低";
    traits.initialTiles = 8;               // 初始地块+8
    traits.scienceBonus = 1.2f;           // 科研加成20%
    traits.cultureBonus = 1.2f;           // 文化加成20%
    traits.militaryProductionBonus = 0.8f; // 军事生产力成本减少20%
    traits.halfCostIndustrial = false;
    traits.extraDistrictSlot = false;
    traits.eurekaBoost = 0.5f;            // 默认值
    traits.productionBonus = 1.0f;        // 默认值
}

// 获取特性
CivilizationTrait CivRussia::getTraits() const {
    return traits;
}

// 区域加成计算
Yield CivRussia::calculateDistrictBonus(const District* district) const {
    Yield bonus = { 0, 0, 0, 0, 0 };

    if (district == nullptr) {
        return bonus;
    }

    auto districtType = district->getType();

    // 如果是学院区域，提供额外的科研加成
    if (districtType.typeName == District::DistrictTypeName::CAMPUS) {
        int baseScience = district->getYield().scienceYield;
        bonus.scienceYield = static_cast<int>(baseScience * 0.2f); // +20%
    }

    // 如果是剧院广场区域，提供额外的文化加成
    if (districtType.typeName == District::DistrictTypeName::THEATER_SQUARE) {
        int baseCulture = district->getYield().cultureYield;
        bonus.cultureYield = static_cast<int>(baseCulture * 0.2f); // +20%
    }

    return bonus;
}

// 区域建设成本计算
float CivRussia::calculateDistrictCost(const std::string& districtType) const {
    return 1.0f;
}

float CivRussia::calculateDistrictCost(District::DistrictType type) const {
    return 1.0f;
}

// 城市区域容量计算
int CivRussia::calculateMaxDistricts(int population) const {
    return BaseCiv::calculateMaxDistricts(population);
}