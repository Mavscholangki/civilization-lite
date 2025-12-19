#ifndef __TILE_DATA_H__
#define __TILE_DATA_H__

enum class TerrainType {
    OCEAN,      // 深海
    COAST,      // 浅海
    DESERT,     // 沙漠 (热+干)
    PLAINS,     // 平原 (温+干)
    GRASSLAND,  // 草原 (温+湿)
    JUNGLE,     // 雨林 (热+湿) - 新增
    TUNDRA,     // 冻土 (冷) - 新增
    SNOW,       // 雪地 (极冷) - 新增
    MOUNTAIN    // 山脉
};

struct TileData {
    TerrainType type;
    float height;           // 高度值 0~1
    float moisture;         // 湿度值 0~1
    float temperature;      // 温度值 0~1 (由纬度决定)
    
    // 【移动相关】
    int movementCost;       // 移动力消耗：1/2/999(不可)
    bool canCrossWater;     // 是否可以下水
    
    // 【产出属性】基础值 + 柏林噪声随机
    int food;               // 粮食产出
    int production;         // 生产力产出
    int gold;               // 金币产出
    int science;            // 科技产出（仅特殊地方）
    int culture;            // 文化产出（仅特殊地方）

    TileData() 
        : type(TerrainType::OCEAN), height(0), moisture(0), temperature(0),
          movementCost(1), canCrossWater(false),
          food(0), production(0), gold(0), science(0), culture(0) {}
};

#endif