// CivChina.h
#ifndef __CIV_CHINA_H__
#define __CIV_CHINA_H__

#include "BaseCiv.h"
#include "../Units/Base/AbstractUnit.h"

// 虎蹲炮单位 - 适应现有AbstractUnit接口
class TigerCannonUnit : public AbstractUnit {
public:
    static TigerCannonUnit* create(Hex pos) {
        TigerCannonUnit* pRet = new TigerCannonUnit();
        if (pRet && pRet->initUnit(pos)) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    // 实现AbstractUnit的纯虚函数
    virtual std::string getUnitName() override {
        return "虎蹲炮";
    }
    virtual int getBaseAttack() override {
        return 30;
    }
    virtual int getMaxMoves() override {
        return 2;
    }

    // 虎蹲炮特有属性（通过成员变量实现）
    int getSiegeBonus() const { return 15; } // 攻城加成
    bool isRangedUnit() const { return true; } // 是远程单位
    int getRange() const { return 1; } // 攻击距离1

private:
    int experience = 0;
};

class CivChina : public BaseCiv {
public:
    CivChina();
    virtual ~CivChina() {}

    // 重写特性获取
    virtual CivilizationTrait getTraits() const override;

    // 特殊能力
    virtual float getEurekaBoost() const override { return 0.75f; } // 尤里卡75%
    virtual int getBuilderCharges() const override { return 5; }    // 建造者5次

    // 特殊单位相关
    virtual bool hasUniqueUnit(const std::string& unitName) const override;
    virtual AbstractUnit* createUniqueUnit(const std::string& unitName, Hex pos) override;
    virtual bool isUniqueUnitUnlocked(const std::string& unitName) const override;

    // 区域建设成本计算（中国没有特殊折扣）
    virtual float calculateDistrictCost(const std::string& districtType) const override;
    virtual float calculateDistrictCost(District::DistrictType type) const override;

    // 城市区域容量计算（中国没有额外区域槽位）
    virtual int calculateMaxDistricts(int population) const override;

    // 区域加成计算
    virtual Yield calculateDistrictBonus(const District* district) const override;
};

#endif