// CivChina.cpp
#include "CivChina.h"
#include "../Units/Civilian/Builder.h"
#include "../Development/TechSystem.h"
#include "District/Base/District.h"

CivChina::CivChina() : BaseCiv("China", "Qin Shi Huang") {
    civType = CivilizationType::CHINA;

    // 设置中国特性
    traits.name = "始皇帝";
    traits.description = "尤里卡和灵感加成提升75%，建造者有5次使用次数，拥有特殊单位虎蹲炮";
    traits.eurekaBoost = 0.75f; // 75%尤里卡加成
    traits.initialTiles = 7;    // 默认值
    traits.halfCostIndustrial = false;
    traits.extraDistrictSlot = false;
    traits.scienceBonus = 1.0f;  // 默认
    traits.cultureBonus = 1.0f;  // 默认
    traits.productionBonus = 1.0f; // 默认
    traits.militaryProductionBonus = 1.0f; // 默认

    // 添加特殊单位到列表
    uniqueUnits.push_back("虎蹲炮");
}

// 获取特性
CivilizationTrait CivChina::getTraits() const {
    return traits;
}

// 检查是否有特殊单位
bool CivChina::hasUniqueUnit(const std::string& unitName) const {
    for (const auto& unit : uniqueUnits) {
        if (unit == unitName) return true;
    }
    return false;
}

// 检查虎蹲炮是否解锁
bool CivChina::isUniqueUnitUnlocked(const std::string& unitName) const {
    if (unitName != "虎蹲炮") {
        return false;
    }

    if (techTree == nullptr) {
        return false;
    }

    // 检查机械科技（ID:11）是否解锁
    return techTree->isActivated(11); // 机械科技ID
}

// 创建特殊单位
AbstractUnit* CivChina::createUniqueUnit(const std::string& unitName, Hex pos) {
    if (unitName == "虎蹲炮") {
        // 检查是否解锁
        if (isUniqueUnitUnlocked(unitName)) {
            return TigerCannonUnit::create(pos);
        }
    }

    return nullptr;
}

// 区域建设成本计算 - 字符串版本
float CivChina::calculateDistrictCost(const std::string& districtType) const {
    return 1.0f; // 原价
}

// 区域建设成本计算 - DistrictType版本
float CivChina::calculateDistrictCost(District::DistrictType type) const {
    return 1.0f; // 原价
}

// 城市区域容量计算
int CivChina::calculateMaxDistricts(int population) const {
    return BaseCiv::calculateMaxDistricts(population);
}

// 区域加成计算
Yield CivChina::calculateDistrictBonus(const District* district) const {
    return BaseCiv::calculateDistrictBonus(district);
}