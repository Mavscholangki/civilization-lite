// CivGermany.h
#ifndef __CIV_GERMANY_H__
#define __CIV_GERMANY_H__

#include "BaseCiv.h"

class CivGermany : public BaseCiv {
public:
    CivGermany();
    virtual ~CivGermany() {}

    // 重写特性获取
    virtual CivilizationTrait getTraits() const override;

    // 特殊能力 - 完全按照文档实现
    virtual bool hasHalfCostIndustrial() const override { return true; }    // 工业区半价
    virtual bool hasExtraDistrictSlot() const override { return true; }     // 城市区域上限+1

    // 德国没有特殊单位，不需要重写创建特殊单位的方法

    // 区域建设成本计算
    virtual float calculateDistrictCost(const std::string& districtType) const override;

    // 城市区域容量计算
    virtual int calculateMaxDistricts(int population) const override;

private:
    // 德国特色建筑：汉萨商站（工业区特色建筑）
};

#endif