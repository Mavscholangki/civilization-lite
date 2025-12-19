// CivGermany.cpp
#include "CivGermany.h"

// 构造函数
CivGermany::CivGermany() : BaseCiv("Germany", "Barbarossa") {
    civType = CivilizationType::GERMANY;

    // 设置德国特性
    traits.name = "神圣罗马皇帝";
    traits.description = "工业区建设成本减半，每个城市可建造的区域上限+1";
    traits.halfCostIndustrial = true;
    traits.extraDistrictSlot = true;
    traits.initialTiles = 7;      // 默认值
    traits.eurekaBoost = 0.5f;    // 默认值
}

// 获取特性
CivilizationTrait CivGermany::getTraits() const {
    return traits;
}

// 区域建设成本计算
float CivGermany::calculateDistrictCost(const std::string& districtType) const {
    // 检查是否为工业区（需要根据实际项目中的区域类型标识调整）
    if (districtType == "INDUSTRY_ZONE" || districtType == "IndustrialZone") {
        return 0.5f; // 半价
    }

    // 其他区域按原价
    return 1.0f;
}

// 城市区域容量计算
int CivGermany::calculateMaxDistricts(int population) const {
    // 德国特殊能力：区域上限+1
    // 默认公式：人口除以3（向下取整）再加1，德国额外+1
    int baseDistricts = std::max(1, population / 3 + 1);
    return baseDistricts + 1; // 德国额外+1
}