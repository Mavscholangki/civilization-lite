#ifndef __YIELD_H__
#define __YIELD_H__

// 区域产出结构体: 食物, 生产力, 科技值, 金币, 文化值
struct Yield {
    int foodYield; // 食物产出
    int productionYield; // 生产力产出
    int scienceYield; // 科技值产出
    int goldYield; // 金币产出
    int cultureYield; // 文化值产出

    Yield operator+(const Yield& other)
    {
        Yield ret;
        ret.foodYield = foodYield + other.foodYield;
        ret.productionYield = productionYield + other.productionYield;
        ret.scienceYield = scienceYield + other.scienceYield;
        ret.goldYield = goldYield + other.goldYield;
        ret.cultureYield = cultureYield + other.cultureYield;
        return ret;
    }
};

#endif