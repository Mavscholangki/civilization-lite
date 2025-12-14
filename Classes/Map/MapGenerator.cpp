/*
* 散布式细长山脉生成器 (Scattered Strip Mountains)
* 特点：山脉呈细条状，但分布稀疏且不连续，被随机打断
*/

#include "MapGenerator.h"
#include "Utils/PerlinNoise.h"
#include <cmath>
#include <ctime>
#include <algorithm>
#include <queue>
#include <vector>
#include <random>
#include <set>

USING_NS_CC;

// ----------------------------------------------------------------------------
// 辅助函数
// ----------------------------------------------------------------------------

std::vector<Hex> GetNeighbors(const Hex& center) {
    std::vector<Hex> neighbors;
    const int directions[6][2] = {
        {1, 0}, {1, -1}, {0, -1},
        {-1, 0}, {-1, 1}, {0, 1}
    };
    for (int i = 0; i < 6; i++) {
        neighbors.push_back(Hex(center.q + directions[i][0], center.r + directions[i][1]));
    }
    return neighbors;
}

float SmoothStep(float edge0, float edge1, float x) {
    x = std::max(0.0f, std::min(1.0f, (x - edge0) / (edge1 - edge0)));
    return x * x * (3 - 2 * x);
}

// ----------------------------------------------------------------------------
// 主生成逻辑
// ----------------------------------------------------------------------------

std::map<Hex, TileData> MapGenerator::generate(int width, int height) {
    std::map<Hex, TileData> map_data;

    // 1. 初始化随机数
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_seed(0, 10000);
    std::uniform_real_distribution<> dist_offset(-50000.0, 50000.0);

    // 2. 初始化噪声生成器
    PerlinNoise continent_noise(dist_seed(gen)); // 大陆轮廓
    PerlinNoise ridge_noise1(dist_seed(gen));    // 山脉层1 (横向)
    PerlinNoise ridge_noise2(dist_seed(gen));    // 山脉层2 (纵向)
    PerlinNoise warp_noise(dist_seed(gen));      // 坐标扭曲
    PerlinNoise breakup_noise(dist_seed(gen));   // 打断噪声 (用于切断山脉)
    PerlinNoise climate_noise(dist_seed(gen));   // 气候
    PerlinNoise river_noise(dist_seed(gen));     // 河流

    const float kOffsetX = dist_offset(gen);
    const float kOffsetY = dist_offset(gen);

    std::uniform_real_distribution<> dist_sea_level(0.36, 0.42);
    const float kSeaLevel = dist_sea_level(gen);

    // ========================================================================
    // 步骤 1：生成海陆分布 (Continents)
    // ========================================================================
    std::map<Hex, bool> is_land;

    for (int r = 0; r < height; r++) {
        for (int q = 0; q < width; q++) {
            int ax_r = r;
            int ax_q = q - (r >> 1);
            Hex hex(ax_q, ax_r);
            TileData tile;

            float nx = static_cast<float>(q) / width;
            float ny = static_cast<float>(r) / height;

            // 生成大陆噪声
            float continent = continent_noise.octaveNoise(
                nx * 3.6f * 2.4f + kOffsetX,
                ny * 3.6f + kOffsetY,
                4, 0.5f
            );

            // 边缘衰减
            float dx = 2.0f * nx - 1.0f;
            float dy = 2.0f * ny - 1.0f;
            float edge_dist = std::max(std::abs(dx), std::abs(dy));
            float edge_fade = SmoothStep(1.0f, 0.87f, edge_dist);

            continent = continent * edge_fade;
            continent = std::pow(continent, 1.18f);

            tile.height = continent;

            if (continent < kSeaLevel) {
                tile.type = TerrainType::OCEAN;
                is_land[hex] = false;
            }
            else {
                tile.type = TerrainType::GRASSLAND;
                is_land[hex] = true;
            }

            map_data[hex] = tile;
        }
    }

    // ========================================================================
    // 步骤 2：生成散开的条状山脉 (Scattered Strip Ridges)
    // ========================================================================
    std::map<Hex, float> ridge_value_map;

    for (auto& pair : map_data) {
        Hex hex = pair.first;
        if (!is_land[hex]) {
            ridge_value_map[hex] = 0.0f;
            continue;
        }

        // --- 1. 强力坐标扭曲 (Stronger Warping) ---
        // 增加扭曲力度，让山脉分布更不规则，模拟板块挤压的随机性
        float nx = static_cast<float>(hex.q + (hex.r >> 1)) / width;
        float ny = static_cast<float>(hex.r) / height;

        float warp_strength = 0.12f;
        float wx = nx + warp_noise.noise(nx * 2.5f, ny * 2.5f) * warp_strength;
        float wy = ny + warp_noise.noise(nx * 2.5f + 100.0f, ny * 2.5f + 100.0f) * warp_strength;

        // --- 2. 稀疏的各向异性生成 ---

        // 组 A：倾向于横向延伸
        // 频率降到 0.8 -> 山脉链之间的距离会非常远
        // 压缩比 10.0 -> 保持条状
        float ridge_h = ridge_noise1.noise(wx * 0.8f + kOffsetX, wy * 10.0f + kOffsetY);
        ridge_h = 1.0f - std::abs(ridge_h - 0.5f) * 2.0f;
        ridge_h = std::pow(ridge_h, 12.0f); // 极细锐化

        // 组 B：倾向于纵向延伸
        float ridge_v = ridge_noise2.noise(wx * 10.0f + kOffsetX + 500.0f, wy * 0.8f + kOffsetY + 500.0f);
        ridge_v = 1.0f - std::abs(ridge_v - 0.5f) * 2.0f;
        ridge_v = std::pow(ridge_v, 12.0f);

        float final_ridge = std::max(ridge_h, ridge_v);

        // --- 3. 关键：打断连续性 (The "Breakup" Pass) ---
        // 生成一个高频噪声，如果在低值区，就强行把山脉抹掉
        // 频率 4.0 意味着每隔一段距离就切一刀
        float breakup = breakup_noise.noise(nx * 4.0f + kOffsetX, ny * 4.0f + kOffsetY);

        // breakup < 0.4 的地方山脉会被切断
        float breakup_mask = SmoothStep(0.35f, 0.55f, breakup);

        final_ridge *= breakup_mask;

        // --- 4. 区域限制 ---
        // 依然保留大区域的空白，防止满屏都是碎山
        float region_mask = continent_noise.noise(nx * 1.5f + kOffsetX, ny * 1.5f + kOffsetY);
        region_mask = SmoothStep(0.2f, 0.6f, region_mask);

        ridge_value_map[hex] = final_ridge * region_mask;
    }

    // ========================================================================
    // 步骤 3：严格阈值化 (Thresholding)
    // ========================================================================
    for (auto& pair : map_data) {
        if (!is_land[pair.first]) continue;

        // 阈值设定：0.18
        // 配合 pow(12) 和打断噪声，这会留下断断续续的细线
        if (ridge_value_map[pair.first] > 0.18f) {
            pair.second.type = TerrainType::MOUNTAIN;
            pair.second.height = 0.85f + (static_cast<float>(dist_seed(gen)) / 10000.0f) * 0.15f;
        }
    }

    // ========================================================================
    // 步骤 4：形态学细化与去噪 (Morphological Cleaning)
    // ========================================================================
    std::map<Hex, TerrainType> temp_types;
    for (auto& pair : map_data) temp_types[pair.first] = pair.second.type;

    // --- 第一轮：腐蚀 (Erosion) ---
    // 去除团块，保持条状
    for (auto& pair : map_data) {
        Hex hex = pair.first;
        if (pair.second.type == TerrainType::MOUNTAIN) {
            int mountain_neighbors = 0;
            for (const auto& n : GetNeighbors(hex)) {
                if (map_data.find(n) != map_data.end() && map_data[n].type == TerrainType::MOUNTAIN) {
                    mountain_neighbors++;
                }
            }

            // 如果被 >=5 个山脉包围，说明它是团块的中心，变成平地
            if (mountain_neighbors >= 5) {
                temp_types[hex] = TerrainType::GRASSLAND;
            }
        }
    }
    for (auto& pair : map_data) pair.second.type = temp_types[pair.first];

    // --- 第二轮：去孤点 (Despeckle) ---
    // 因为我们要散开的山脉，但不要单个格子的噪点
    for (auto& pair : map_data) temp_types[pair.first] = pair.second.type;

    for (auto& pair : map_data) {
        Hex hex = pair.first;
        if (pair.second.type == TerrainType::MOUNTAIN) {
            int mountain_neighbors = 0;
            for (const auto& n : GetNeighbors(hex)) {
                if (map_data.find(n) != map_data.end() && map_data[n].type == TerrainType::MOUNTAIN) {
                    mountain_neighbors++;
                }
            }
            // 如果完全孤立，删掉
            if (mountain_neighbors == 0) {
                temp_types[hex] = TerrainType::GRASSLAND;
            }
        }
        // 注意：这次我不做“填补断点”的操作了，因为你的需求是“散开”
        // 所以我们允许断裂存在
    }
    for (auto& pair : map_data) pair.second.type = temp_types[pair.first];

    // ========================================================================
    // 步骤 5：生成气候 (Climate: Moisture & Temperature)
    // ========================================================================
    for (auto& pair : map_data) {
        Hex hex = pair.first;
        TileData& tile = pair.second;

        if (!is_land[hex] || tile.type == TerrainType::MOUNTAIN) continue;

        int offset_r = hex.r;
        int offset_q = hex.q + (hex.r >> 1);
        float nx = static_cast<float>(offset_q) / width;
        float ny = static_cast<float>(offset_r) / height;

        // 1. 湿度
        float moisture = climate_noise.octaveNoise(
            nx * 5.5f * 2.4f + kOffsetX + 500.0f,
            ny * 5.5f + kOffsetY + 500.0f,
            3, 0.5f
        );

        // 山脉阻挡效应
        int mountain_neighbors = 0;
        for (const auto& n : GetNeighbors(hex)) {
            if (map_data.find(n) != map_data.end() && map_data[n].type == TerrainType::MOUNTAIN) {
                mountain_neighbors++;
            }
        }
        moisture += mountain_neighbors * 0.10f;
        moisture += 0.30f;
        moisture = std::max(0.0f, std::min(1.0f, moisture));

        // 2. 温度
        float temperature = 1.0f - std::abs(ny - 0.5f) * 2.0f;
        temperature += climate_noise.octaveNoise(
            nx * 4.0f * 2.4f + kOffsetX,
            ny * 4.0f + kOffsetY,
            2, 0.5f
        ) * 0.15f - 0.075f;
        temperature = std::max(0.0f, std::min(1.0f, temperature));

        tile.moisture = moisture;
        tile.temperature = temperature;

        // 3. 确定地貌
        if (temperature < 0.20f) {
            tile.type = (temperature < 0.10f) ? TerrainType::SNOW : TerrainType::TUNDRA;
        }
        else if (moisture < 0.26f) {
            tile.type = TerrainType::DESERT;
        }
        else if (moisture < 0.42f) {
            tile.type = TerrainType::PLAINS;
        }
        else if (temperature > 0.82f && moisture > 0.78f) {
            tile.type = TerrainType::JUNGLE;
        }
        else {
            tile.type = TerrainType::GRASSLAND;
        }
    }

    // ========================================================================
    // 步骤 6：生成海岸线
    // ========================================================================
    std::queue<std::pair<Hex, int>> frontier;
    std::map<Hex, bool> visited;

    for (auto& pair : map_data) {
        Hex current = pair.first;
        TileData& data = pair.second;
        if (data.type != TerrainType::OCEAN) continue;

        bool next_to_land = false;
        for (const auto& n : GetNeighbors(current)) {
            if (map_data.find(n) != map_data.end()) {
                if (map_data[n].type != TerrainType::OCEAN && map_data[n].type != TerrainType::COAST) {
                    next_to_land = true;
                    break;
                }
            }
        }
        if (next_to_land) {
            data.type = TerrainType::COAST;
            visited[current] = true;
            frontier.push({ current, 1 });
        }
    }

    const int kMaxCoastWidth = 2;
    while (!frontier.empty()) {
        auto t = frontier.front();
        frontier.pop();
        if (t.second >= kMaxCoastWidth) continue;

        for (const auto& n : GetNeighbors(t.first)) {
            if (map_data.find(n) != map_data.end() && !visited[n]) {
                if (map_data[n].type == TerrainType::OCEAN) {
                    map_data[n].type = TerrainType::COAST;
                    visited[n] = true;
                    frontier.push({ n, t.second + 1 });
                }
            }
        }
    }

    // ========================================================================
    // 步骤 7：生成河流
    // ========================================================================
    std::vector<Hex> all_mountains;
    std::set<Hex> river_tiles;

    for (auto& pair : map_data) {
        if (pair.second.type == TerrainType::MOUNTAIN) {
            all_mountains.push_back(pair.first);
        }
    }

    std::uniform_real_distribution<> dist_river_chance(0.0, 1.0);

    for (const auto& mountain_hex : all_mountains) {
        if (dist_river_chance(gen) > 0.35f) continue;

        std::vector<Hex> start_candidates;
        for (const auto& n : GetNeighbors(mountain_hex)) {
            if (map_data.find(n) != map_data.end()) {
                if (map_data[n].type != TerrainType::MOUNTAIN &&
                    map_data[n].type != TerrainType::OCEAN &&
                    map_data[n].type != TerrainType::COAST) {
                    start_candidates.push_back(n);
                }
            }
        }

        if (start_candidates.empty()) continue;

        std::uniform_int_distribution<> dist_start(0, start_candidates.size() - 1);
        Hex current = start_candidates[dist_start(gen)];

        std::set<Hex> path_visited;
        int max_steps = 50;

        for (int step = 0; step < max_steps; step++) {
            if (path_visited.count(current)) break;
            path_visited.insert(current);

            if (map_data[current].type == TerrainType::OCEAN ||
                map_data[current].type == TerrainType::COAST) {
                break;
            }

            if (map_data[current].type != TerrainType::MOUNTAIN) {
                river_tiles.insert(current);
            }

            Hex next = current;
            float min_height = map_data[current].height;

            auto neighbors = GetNeighbors(current);
            for (const auto& n : neighbors) {
                if (map_data.find(n) != map_data.end() && !path_visited.count(n)) {
                    float height_with_noise = map_data[n].height +
                        river_noise.noise(n.q * 0.1f, n.r * 0.1f) * 0.05f;

                    if (height_with_noise < min_height) {
                        min_height = height_with_noise;
                        next = n;
                    }
                }
            }

            if (next == current) break;
            current = next;
        }
    }

    // 河流效果
    for (const auto& river_hex : river_tiles) {
        if (map_data[river_hex].type == TerrainType::DESERT) map_data[river_hex].type = TerrainType::GRASSLAND;
        map_data[river_hex].moisture = std::min(1.0f, map_data[river_hex].moisture + 0.22f);

        for (const auto& n : GetNeighbors(river_hex)) {
            if (map_data.find(n) != map_data.end()) {
                if (map_data[n].type == TerrainType::DESERT) map_data[n].type = TerrainType::GRASSLAND;
                else if (map_data[n].type == TerrainType::PLAINS) map_data[n].type = TerrainType::GRASSLAND;

                map_data[n].moisture = std::min(1.0f, map_data[n].moisture + 0.16f);
            }
        }
    }

    // ========================================================================
    // 步骤 8：初始化地形产出
    // ========================================================================
    for (auto& pair : map_data) {
        TileData& tile = pair.second;

        // 根据地形类型初始化产出
        // 【关键】山脉和深海没有产出
        switch(tile.type) {
            case TerrainType::OCEAN:
                // 深海：没有产出
                tile.food = 0;
                tile.gold = 0;
                tile.production = 0;
                tile.science = 0;
                tile.culture = 0;
                break;
                
            case TerrainType::COAST:
                // 浅海：有产出
                tile.food = 2;
                tile.gold = 1;
                tile.production = 0;
                tile.science = 0;
                tile.culture = 0;
                break;
                
            case TerrainType::DESERT:
                // 沙漠：少量产出
                tile.food = 0;
                tile.gold = 1;
                tile.production = 1;
                tile.science = 0;
                tile.culture = 0;
                break;
                
            case TerrainType::PLAINS:
                // 平原：标准产出
                tile.food = 2;
                tile.gold = 1;
                tile.production = 2;
                tile.science = 0;
                tile.culture = 0;
                break;
                
            case TerrainType::GRASSLAND:
                // 草原：粮食丰富
                tile.food = 3;
                tile.gold = 0;
                tile.production = 1;
                tile.science = 0;
                tile.culture = 0;
                break;
                
            case TerrainType::JUNGLE:
                // 雨林：多样化产出
                tile.food = 2;
                tile.gold = 0;
                tile.production = 1;
                tile.science = 1;
                tile.culture = 1;
                break;
                
            case TerrainType::TUNDRA:
                // 冻土：极少产出
                tile.food = 1;
                tile.gold = 0;
                tile.production = 0;
                tile.science = 0;
                tile.culture = 0;
                break;
                
            case TerrainType::SNOW:
                // 雪地：没有产出
                tile.food = 0;
                tile.gold = 0;
                tile.production = 0;
                tile.science = 0;
                tile.culture = 0;
                break;
                
            case TerrainType::MOUNTAIN:
                // 山脉：没有产出
                tile.food = 0;
                tile.gold = 0;
                tile.production = 0;
                tile.science = 0;
                tile.culture = 0;
                break;
                
            default:
                break;
        }
    }

    return map_data;
}