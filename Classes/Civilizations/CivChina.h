// CivChina.h
#ifndef __CIV_CHINA_H__
#define __CIV_CHINA_H__

#include "BaseCiv.h"
#include <vector>
#include <string>

// 前向声明
class TigerCannonUnit;

class CivChina : public BaseCiv {
public:
    // 创建方法
    static CivChina* create() {
        CivChina* pRet = new(std::nothrow) CivChina();
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
    virtual float getEurekaBoost() const override { return 0.75f; } // 尤里卡75%
    virtual int getBuilderCharges() const override { return 5; }    // 建造者5次

    // ==================== 特殊单位接口 ====================
    virtual bool hasUniqueUnit(const std::string& unitName) const override;
    virtual bool isUniqueUnitUnlocked(const std::string& unitName) const override;
    virtual cocos2d::Ref* createUniqueUnit(const std::string& unitName, void* position) override;

    // ==================== 区域加成计算 ====================
    virtual Yield calculateDistrictBonus(const District* district) const override;

private:
    std::vector<std::string> m_uniqueUnits;
};

#endif // __CIV_CHINA_H__
