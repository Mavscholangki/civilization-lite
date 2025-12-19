// BaseCiv.h
#ifndef __BASE_CIV_H__
#define __BASE_CIV_H__

#include "cocos2d.h"
#include "../Development/TechSystem.h"
#include "../Development/CultureSystem.h"
#include "../City/Yield.h"  // 包含项目中的 Yield 定义

// 前向声明
class District;
namespace DistrictType {
    struct DistrictTypeInfo;
}

// 文明特性结构体
struct CivilizationTrait {
    std::string name;
    std::string description;

    // 通用加成
    int initialTiles = 3;             // 初始地块数
    float eurekaBoost = 0.5f;         // 尤里卡加成系数（默认50%）
    float inspirationBoost = 0.5f;    // 灵感加成系数（默认50%）

    // 资源加成
    float scienceBonus = 1.0f;        // 科研加成系数
    float cultureBonus = 1.0f;        // 文化加成系数
    float productionBonus = 1.0f;     // 生产力加成系数
    float goldBonus = 1.0f;           // 金币加成系数
    float faithBonus = 1.0f;          // 信仰加成系数

    // 区域相关
    bool halfCostIndustrial = false;  // 工业区半价
    bool extraDistrictSlot = false;   // 额外区域槽位
    float militaryProductionBonus = 1.0f; // 军事单位生产力成本系数

    // 单位相关
    int builderCharges = 3;           // 建造者使用次数

    CivilizationTrait() = default;
};

class BaseCiv : public cocos2d::Ref {
public:
    // 创建方法
    static BaseCiv* create() {
        BaseCiv* pRet = new(std::nothrow) BaseCiv();
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        }
        delete pRet;
        return nullptr;
    }

    // 初始化
    virtual bool init();

    virtual ~BaseCiv() {}

    // ==================== 文明特性获取 ====================
    virtual CivilizationTrait getTraits() const { return m_traits; }

    // ==================== 通用加成接口 ====================
    virtual int getInitialTiles() const { return m_traits.initialTiles; }
    virtual float getEurekaBoost() const { return m_traits.eurekaBoost; }
    virtual float getInspirationBoost() const { return m_traits.inspirationBoost; }
    virtual int getBuilderCharges() const { return m_traits.builderCharges; }

    // ==================== 资源加成接口 ====================
    virtual float getScienceBonus() const { return m_traits.scienceBonus; }
    virtual float getCultureBonus() const { return m_traits.cultureBonus; }
    virtual float getProductionBonus() const { return m_traits.productionBonus; }
    virtual float getGoldBonus() const { return m_traits.goldBonus; }
    virtual float getFaithBonus() const { return m_traits.faithBonus; }
    virtual float getMilitaryProductionCost() const { return m_traits.militaryProductionBonus; }

    // ==================== 区域相关接口 ====================
    virtual bool hasHalfCostIndustrial() const { return m_traits.halfCostIndustrial; }
    virtual bool hasExtraDistrictSlot() const { return m_traits.extraDistrictSlot; }

    // ==================== 特殊单位接口 ====================
    virtual bool hasUniqueUnit(const std::string& unitName) const { return false; }
    virtual bool isUniqueUnitUnlocked(const std::string& unitName) const { return false; }
    virtual cocos2d::Ref* createUniqueUnit(const std::string& unitName, void* position) { return nullptr; }

    // ==================== 区域加成计算 ====================
    virtual Yield calculateDistrictBonus(const District* district) const;

    // ==================== 成本计算接口 ====================
    virtual float calculateDistrictCost(const std::string& districtType) const { return 1.0f; }
    virtual float calculateDistrictCost(const DistrictType::DistrictTypeInfo& type) const { return 1.0f; }

    // ==================== 区域容量计算 ====================
    virtual int calculateMaxDistricts(int population) const;

    // ==================== 科技/文化加成应用 ====================
    virtual int applyScienceBonus(int baseScience) const {
        return static_cast<int>(baseScience * getScienceBonus());
    }

    virtual int applyCultureBonus(int baseCulture) const {
        return static_cast<int>(baseCulture * getCultureBonus());
    }

    virtual int applyEurekaBonus(int techId, const TechTree* techTree) const;
    virtual int applyInspirationBonus(int cultureId, const CultureTree* cultureTree) const;

    // ==================== 事件回调 ====================
    virtual void onTechActivated(int techId) {}
    virtual void onCultureUnlocked(int cultureId) {}
    virtual void onCityFounded(void* city) {}

protected:
    CivilizationTrait m_traits;
};

#endif // __BASE_CIV_H__