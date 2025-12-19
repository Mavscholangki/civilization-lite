// GameConfig.h
#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#include <string>

namespace GameConfig {
    // 玩家起始资源
    const int STARTING_GOLD = 100;
    const int STARTING_SCIENCE = 0;
    const int STARTING_CULTURE = 0;
    const int STARTING_FAITH = 0;
    const int STARTING_AMENITIES = 0;
    const int STARTING_HAPPINESS = 0;

    // 城市相关配置
    const int BASE_CITY_RANGE = 3;  // 城市基础地块范围
    const int MIN_CITY_DISTANCE = 4; // 城市最小间距

    // 人口相关
    const int BASE_POPULATION_GROWTH_COST = 15; // 基础人口增长所需食物
    const float POPULATION_GROWTH_FACTOR = 1.2f; // 人口增长系数

    // 区域相关
    const int BASE_DISTRICT_COST = 30; // 基础区域建设成本
    const float DISTRICT_COST_MULTIPLIER_PER_TECH = 1.05f; // 每项科技增加的成本系数

    // 科技和文化研究
    const int BASE_SCIENCE_PER_POP = 1; // 每个人口提供的基础科研
    const int BASE_CULTURE_PER_POP = 1; // 每个人口提供的基础文化

    // 维护费
    const int BASE_UNIT_MAINTENANCE = 1; // 单位基础维护费
    const int BASE_BUILDING_MAINTENANCE = 1; // 建筑基础维护费

    // 单位属性
    const int DEFAULT_SETTLER_COST = 30; // 移民者基础成本
    const int DEFAULT_BUILDER_COST = 25; // 建造者基础成本

    // 游戏速度调整（回合数）
    const int GAME_SPEED_QUICK = 330;
    const int GAME_SPEED_STANDARD = 500;
    const int GAME_SPEED_EPIC = 750;
    const int GAME_SPEED_MARATHON = 1500;

    // 胜利条件
    const int SCIENCE_VICTORY_TECH_COUNT = 20; // 科技胜利所需科技数
    const int CULTURE_VICTORY_TOURISM_TARGET = 500; // 文化胜利所需旅游业绩

    // 文明特性相关
    namespace Civilization {
        // 中国
        const float CHINA_EUREKA_BOOST = 0.75f; // 尤里卡75%
        const int CHINA_BUILDER_CHARGES = 5;    // 建造者5次

        // 俄罗斯
        const int RUSSIA_EXTRA_TILES = 8;       // 初始地块+8
        const float RUSSIA_MILITARY_COST_REDUCTION = 0.8f; // 军事单位成本减少20%

        // 德国
        const float GERMANY_INDUSTRIAL_DISCOUNT = 0.5f; // 工业区半价
        const int GERMANY_EXTRA_DISTRICT_SLOT = 1;     // 额外区域槽位
    }

    // 政策系统
    namespace Policy {
        const int BASE_POLICY_UNLOCK_COST = 20; // 基础政策解锁所需文化值
        const int POLICY_SWAP_COST = 0;         // 更换政策所需费用（金币）
    }

    // 外交
    const int BASE_WAR_WEARINESS = 0; // 基础战争厌倦度
    const int BASE_DIPLOMATIC_FAVOR = 0; // 基础外交支持

    // 时代系统
    namespace Era {
        const int ANCIENT_ERA_THRESHOLD = 0;   // 远古时代阈值
        const int CLASSICAL_ERA_THRESHOLD = 8;  // 古典时代阈值
        const int MEDIEVAL_ERA_THRESHOLD = 18; // 中世纪时代阈值
        const int RENAISSANCE_ERA_THRESHOLD = 32; // 文艺复兴时代阈值
        const int INDUSTRIAL_ERA_THRESHOLD = 50; // 工业时代阈值
        const int MODERN_ERA_THRESHOLD = 72;    // 现代时代阈值
        const int ATOMIC_ERA_THRESHOLD = 98;    // 原子能时代阈值
        const int INFORMATION_ERA_THRESHOLD = 128; // 信息时代阈值
    }

    // 调试开关
    const bool DEBUG_MODE = true;
    const bool CHEAT_MODE = false;

    // 资源类型
    enum ResourceType {
        FOOD,
        PRODUCTION,
        GOLD,
        SCIENCE,
        CULTURE,
        FAITH
    };

    // 获取资源名称
    inline std::string getResourceName(ResourceType type) {
        switch (type) {
            case FOOD: return "食物";
            case PRODUCTION: return "生产力";
            case GOLD: return "金币";
            case SCIENCE: return "科研";
            case CULTURE: return "文化";
            case FAITH: return "信仰";
            default: return "未知";
        }
    }

    // 计算城市最大区域数（基于人口）
    inline int calculateMaxDistricts(int population) {
        if (population >= 10) return 5;
        if (population >= 7) return 4;
        if (population >= 4) return 3;
        if (population >= 1) return 1;
        return 0;
    }

    // 计算区域建设成本（基于游戏进度）
    inline int calculateDistrictCost(int baseCost, int techCount) {
        float multiplier = 1.0f;
        for (int i = 0; i < techCount; i++) {
            multiplier *= DISTRICT_COST_MULTIPLIER_PER_TECH;
        }
        return static_cast<int>(baseCost * multiplier);
    }
}

#endif // __GAME_CONFIG_H__