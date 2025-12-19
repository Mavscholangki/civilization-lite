// BaseCiv.cpp
#include "BaseCiv.h"
#include "../Units/Melee/Warrior.h"
// BaseCiv.cpp
#include "BaseCiv.h"
#include "../Units/Melee/Warrior.h"
#include "../Units/Civilian/Settler.h"
#include "../Units/Civilian/Builder.h"

BaseCiv::BaseCiv(const std::string& name, const std::string& leader)
    : civType(CivilizationType::BASIC), civName(name), leaderName(leader) {

    // 设置基本特性
    traits.name = "基本文明";
    traits.description = "没有特殊能力的文明";
    traits.initialTiles = 7;
    traits.eurekaBoost = 0.5f; // 默认50%
    traits.scienceBonus = 1.0f;
    traits.cultureBonus = 1.0f;
    traits.productionBonus = 1.0f;
    traits.militaryProductionBonus = 1.0f;
}

bool BaseCiv::hasUniqueUnit(const std::string& unitName) const {
    for (const auto& unit : uniqueUnits) {
        if (unit == unitName) return true;
    }
    return false;
}

bool BaseCiv::isUniqueUnitUnlocked(const std::string& unitName) const {
    return false;
}

AbstractUnit* BaseCiv::createUniqueUnit(const std::string& unitName, Hex pos) {
    // 默认创建基本单位
    if (unitName == "战士") return Warrior::create(pos);
    if (unitName == "开拓者") return Settler::create(pos);
    if (unitName == "建造者") return Builder::create(pos);

    return nullptr;
}

// 区域建设成本计算（默认实现）
float BaseCiv::calculateDistrictCost(const std::string& districtType) const {
    // 默认返回1.0f，表示原价
    return 1.0f;
}

float BaseCiv::calculateDistrictCost(District::DistrictType type) const {
    // 默认返回1.0f，表示原价
    return 1.0f;
}

// 城市区域容量计算（默认实现）
int BaseCiv::calculateMaxDistricts(int population) const {
    // 默认规则：人口除以3（向下取整）再加1
    return std::max(1, population / 3 + 1);
}

// 区域加成计算（默认实现）- 返回空加成
Yield BaseCiv::calculateDistrictBonus(const District* district) const {
    return Yield{ 0, 0, 0, 0, 0 };
}