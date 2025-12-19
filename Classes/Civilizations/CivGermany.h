// CivGermany.h
#ifndef __CIV_GERMANY_H__
#define __CIV_GERMANY_H__

#include "BaseCiv.h"

class CivGermany : public BaseCiv {
public:
    // 创建方法
    static CivGermany* create() {
        CivGermany* pRet = new(std::nothrow) CivGermany();
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    // 初始化
    virtual bool init() override;

    // ==================== 文明特性获取 ====================
    virtual CivilizationTrait getTraits() const override;

    // ==================== 区域相关接口 ====================
    virtual bool hasHalfCostIndustrial() const override { return true; }
    virtual bool hasExtraDistrictSlot() const override { return true; }

    // ==================== 成本计算接口 ====================
    virtual float calculateDistrictCost(const std::string& districtType) const override;

    // ==================== 区域容量计算 ====================
    virtual int calculateMaxDistricts(int population) const override;

private:
    // 德国特色建筑：汉萨商站（工业区特色建筑）
    // 可以在后续添加相关方法
};

#endif // __CIV_GERMANY_H__
