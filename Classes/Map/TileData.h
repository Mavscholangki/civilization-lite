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
    float height;    // 高度值 0~1
    float moisture;  // 湿度值 0~1
    float temperature;// 温度值 0~1 (由纬度决定)

    TileData() : type(TerrainType::OCEAN), height(0), moisture(0), temperature(0) {}
};

#endif