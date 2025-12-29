// CivRussia.h
#ifndef __CIV_RUSSIA_H__
#define __CIV_RUSSIA_H__

#include "BaseCiv.h"

class CivRussia : public BaseCiv {
public:
    // 创建方法
    static CivRussia* create() {
        CivRussia* pRet = new(std::nothrow) CivRussia();
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

    // ==================== 通用加成接口 ====================
    virtual int getInitialTiles() const override { return 8; } // 初始地块+8
    virtual float getMilitaryProductionCost() const override { return 0.8f; } // 军事单位成本减少20%

    // ==================== 资源加成接口 ====================
    virtual float getScienceBonus() const override { return 1.2f; }
    virtual float getCultureBonus() const override { return 1.2f; }

    // ==================== 区域加成计算 ====================
    virtual Yield calculateDistrictBonus(const District* district) const override;

private:
};

#endif // __CIV_RUSSIA_H__
