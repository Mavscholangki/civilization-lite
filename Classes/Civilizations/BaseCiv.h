// BaseCiv.h
#ifndef __BASIC_CIV_H__
#define __BASIC_CIV_H__

#include <string>
#include <vector>
#include <memory>
#include "../Development/CultureSystem.h"
#include "../Development/TechSystem.h"
#include "../Units/Base/AbstractUnit.h"
#include "../District/Base/District.h"

// 文明类型枚举
enum class CivilizationType {
    BASIC,      // 基本文明
    GERMANY,    // 德国
    CHINA,      // 中国
    RUSSIA      // 俄罗斯
};

// 文明特性结构
struct CivilizationTrait {
    std::string name;
    std::string description;

    // 数值加成
    float scienceBonus = 1.0f;        // 科研加成
    float cultureBonus = 1.0f;        // 文化加成
    float productionBonus = 1.0f;     // 生产力加成
    float militaryProductionBonus = 1.0f; // 军事生产力加成

    // 特殊能力
    bool halfCostIndustrial = false;  // 工业区半价
    bool extraDistrictSlot = false;   // 额外区域槽位
    bool enhancedBuilder = false;     // 增强建造者
    int initialTiles = 7;             // 初始地块数量
    float eurekaBoost = 0.5f;         // 尤里卡加成
};

// 基本文明类
class BaseCiv {
protected:
    CivilizationType civType;
    std::string civName;
    std::string leaderName;

    CivilizationTrait traits;

    // 特殊单位列表
    std::vector<std::string> uniqueUnits;

    // 科技和文化系统指针
    TechTree* techTree;
    CultureTree* cultureTree;

public:
    BaseCiv(const std::string& name, const std::string& leader);
    virtual ~BaseCiv() {}

    // 获取基本信息
    CivilizationType getCivilizationType() const { return civType; }
    std::string getCivilizationName() const { return civName; }
    std::string getLeaderName() const { return leaderName; }

    // 获取特性
    virtual CivilizationTrait getTraits() const { return traits; }

    // 特殊能力判定
    virtual bool hasHalfCostIndustrial() const { return false; }    // 工业区半价
    virtual bool hasExtraDistrictSlot() const { return false; }     // 城市区域上限+1

    // 特殊能力数值
    virtual float getEurekaBoost() const { return traits.eurekaBoost; }
    virtual int getBuilderCharges() const { return 3; } // 默认建造者3次
    virtual int getInitialTiles() const { return traits.initialTiles; }
    virtual float getMilitaryProductionCost() const { return 1.0f; } // 默认100%

    // 数值加成计算
    virtual float calculateScienceBonus() const { return traits.scienceBonus; }
    virtual float calculateCultureBonus() const { return traits.cultureBonus; }
    virtual float calculateProductionBonus() const { return traits.productionBonus; }

    // 区域建设成本计算
    virtual float calculateDistrictCost(const std::string& districtType) const;
    virtual float calculateDistrictCost(District::DistrictType type) const;

    // 城市区域容量计算
    virtual int calculateMaxDistricts(int population) const;

    // 区域加成计算
    virtual Yield calculateDistrictBonus(const District* district) const;

    // 设置系统指针
    void setTechTree(TechTree* tree) { techTree = tree; }
    void setCultureTree(CultureTree* tree) { cultureTree = tree; }

    // 检查是否有特殊单位
    virtual bool hasUniqueUnit(const std::string& unitName) const;
    virtual bool isUniqueUnitUnlocked(const std::string& unitName) const;

    // 创建特殊单位
    virtual AbstractUnit* createUniqueUnit(const std::string& unitName, Hex pos);

    // 事件回调
    virtual void onCityFounded() {}
    virtual void onTechResearched(int techId) {}
    virtual void onCultureUnlocked(int cultureId) {}
};

#endif