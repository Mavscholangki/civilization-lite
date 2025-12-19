// BaseCiv.cpp
#include "BaseCiv.h"
#include "District/Base/District.h"

bool BaseCiv::init() {
    // 初始化默认特性
    m_traits.name = "Default Civilization";
    m_traits.description = "Standard civilization with no special bonuses";
    m_traits.initialTiles = 3;
    m_traits.eurekaBoost = 0.5f;
    m_traits.inspirationBoost = 0.5f;
    m_traits.scienceBonus = 1.0f;
    m_traits.cultureBonus = 1.0f;
    m_traits.productionBonus = 1.0f;
    m_traits.goldBonus = 1.0f;
    m_traits.faithBonus = 1.0f;
    m_traits.halfCostIndustrial = false;
    m_traits.extraDistrictSlot = false;
    m_traits.militaryProductionBonus = 1.0f;
    m_traits.builderCharges = 3;

    return true;
}

Yield BaseCiv::calculateDistrictBonus(const District* district) const {
    // 默认文明没有区域加成
    Yield bonus;
    bonus.foodYield = 0;
    bonus.productionYield = 0;
    bonus.scienceYield = 0;
    bonus.goldYield = 0;
    bonus.cultureYield = 0;
    return bonus;
}

int BaseCiv::calculateMaxDistricts(int population) const {
    // 默认公式：人口除以3（向下取整）再加1
    if (population <= 0) return 0;
    return std::max(1, population / 3 + 1);
}

int BaseCiv::applyEurekaBonus(int techId, const TechTree* techTree) const {
    const TechNode* techNode = techTree->getTechInfo(techId);
    if (techNode) {
        int baseBonus = techNode->cost / 2; // 默认50%
        float boostFactor = getEurekaBoost() / 0.5f; // 相对于默认50%的比例
        return static_cast<int>(baseBonus * boostFactor);
    }
    return 0;
}

int BaseCiv::applyInspirationBonus(int cultureId, const CultureTree* cultureTree) const {
    const CultureNode* cultureNode = cultureTree->getCultureInfo(cultureId);
    if (cultureNode) {
        int baseBonus = cultureNode->cost / 2; // 默认50%
        float boostFactor = getInspirationBoost() / 0.5f; // 相对于默认50%的比例
        return static_cast<int>(baseBonus * boostFactor);
    }
    return 0;
}