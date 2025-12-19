// CivRussia.h
#ifndef __CIV_RUSSIA_H__
#define __CIV_RUSSIA_H__

#include "BaseCiv.h"
#include "../District/Base/District.h"

class CivRussia : public BaseCiv {
public:
    CivRussia();
    virtual ~CivRussia() {}

    // 重写特性获取
    virtual CivilizationTrait getTraits() const override;

    // 特殊能力
    virtual int getInitialTiles() const override { return 8; }    // 初始地块+8
    virtual float getMilitaryProductionCost() const override { return 0.8f; } // 军事单位成本减少20%

    // 学院与文化加成（通过特性实现，不需要额外方法）
    virtual float calculateScienceBonus() const override { return 1.2f; }
    virtual float calculateCultureBonus() const override { return 1.2f; }

    // 区域加成计算（学院和剧院广场）
    virtual Yield calculateDistrictBonus(const District* district) const override;

    // 区域建设成本计算（俄罗斯没有特殊折扣）
    virtual float calculateDistrictCost(const std::string& districtType) const override;
    virtual float calculateDistrictCost(District::DistrictType type) const override;

    // 城市区域容量计算（俄罗斯没有额外区域槽位）
    virtual int calculateMaxDistricts(int population) const override;
};

#endif