/*
* 这是一个地图生成器。
* 你不需要修改。
*/

#include "MapGenerator.h"
#include "Utils/PerlinNoise.h"
#include <cmath>
#include <ctime>
#include <algorithm>
#include <queue>
#include <vector>
#include <random>

USING_NS_CC;

// 辅助函数：获取周围 6 个邻居
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

// 辅助函数：平滑插值
float SmoothStep(float edge0, float edge1, float x) {
    x = std::max(0.0f, std::min(1.0f, (x - edge0) / (edge1 - edge0)));
    return x * x * (3 - 2 * x);
}

// Map Generation: The Compromise Solution (Balanced Continents)
// 地图生成：折中方案 (平衡的大陆布局)
// 结合了“宏观轮廓”和“微观细节”，生成 2-3 块形状自然的大陆。
std::map<Hex, TileData> MapGenerator::generate(int width, int height) {
    std::map<Hex, TileData> map_data;

    // 1. Init Random Engine / 初始化随机引擎
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_seed(0, 10000);
    // Offset range / 偏移范围
    std::uniform_real_distribution<> dist_offset(-50000.0, 50000.0);

    // Initialize Noise / 初始化噪声
    PerlinNoise macro_noise(dist_seed(gen));   // [核心] 宏观噪声：决定大陆位置
    PerlinNoise detail_noise(dist_seed(gen));  // [核心] 细节噪声：决定海岸线形状
    PerlinNoise warp_noise(dist_seed(gen));
    PerlinNoise moisture_noise(dist_seed(gen));

    // --------------------------------------------------------------------------
    // 2. Parameters for "The Compromise" / 折中方案的参数
    // --------------------------------------------------------------------------

    // [Macro Scale] 宏观缩放：非常小 (3.0~4.5)。
    // 作用：在 120 宽的地图上，这只会生成 2-3 个巨大的白色波峰（即大陆雏形）。
    std::uniform_real_distribution<> dist_macro_scale(3.0, 4.5);
    const float kMacroScale = dist_macro_scale(gen);

    // [Detail Scale] 细节缩放：中等 (10.0~14.0)。
    // 作用：在宏观轮廓上叠加纹理，防止大陆变成圆球。
    const float kDetailScale = 12.0f;

    // [Sea Level] 海平面：适中 (0.38 ~ 0.45)。
    // 保证陆地占比在 40%~60% 之间。
    std::uniform_real_distribution<> dist_sea_level(0.38, 0.45);
    const float kSeaLevel = dist_sea_level(gen);

    // Offsets / 偏移量
    const float kOffsetX = dist_offset(gen);
    const float kOffsetY = dist_offset(gen);

    // Aspect Ratio / 纵横比修正 (120/50 = 2.4)
    float aspect_ratio = (float)width / height;

    // Temperature Bias / 气温偏差
    std::uniform_real_distribution<> dist_temp_bias(-0.15, 0.15);
    const float kTempBias = dist_temp_bias(gen);

    // --------------------------------------------------------------------------
    // 3. Pass 1: Terrain Generation / 第一遍：地形生成
    // --------------------------------------------------------------------------
    for (int r = 0; r < height; r++) {
        for (int q = 0; q < width; q++) {
            int ax_r = r;
            int ax_q = q - (r >> 1);
            Hex hex(ax_q, ax_r);
            TileData tile;

            float nx = static_cast<float>(q) / width;
            float ny = static_cast<float>(r) / height;

            // --- Domain Warping / 域扭曲 ---
            float q_warp = warp_noise.octaveNoise(nx * 3.0f + kOffsetX, ny * 3.0f + kOffsetY, 2, 0.5f);
            float r_warp = warp_noise.octaveNoise(nx * 3.0f + 5.2f + kOffsetX, ny * 3.0f + 1.3f + kOffsetY, 2, 0.5f);

            // --- Coordinate Calculation / 坐标计算 ---
            // Macro coordinates (Low Frequency)
            float u_macro = (nx * kMacroScale * aspect_ratio) + kOffsetX + (q_warp * 0.1f);
            float v_macro = (ny * kMacroScale) + kOffsetY + (r_warp * 0.1f);

            // Detail coordinates (High Frequency)
            float u_detail = (nx * kDetailScale * aspect_ratio) + kOffsetX;
            float v_detail = (ny * kDetailScale) + kOffsetY;

            // --- Height Blending / 高度混合 (折中核心) ---
            float h_macro = macro_noise.octaveNoise(u_macro, v_macro, 4, 0.5f);
            float h_detail = detail_noise.octaveNoise(u_detail, v_detail, 4, 0.5f);

            // Blend: 70% Macro Shape + 30% Detail Ruggedness
            // 混合公式：70% 的宏观形状 + 30% 的细节粗糙度
            float e = h_macro * 0.70f + h_detail * 0.30f;

            // --- Vignette / 边缘遮罩 ---
            // Rectangular mask to utilize the 120 width fully.
            // 矩形遮罩，只切除最边缘。
            float x_dist = std::abs(nx - 0.5f) * 2.0f;
            float y_dist = std::abs(ny - 0.5f) * 2.0f;
            float x_mask = SmoothStep(1.0f, 0.92f, x_dist); // Only fade last 8%
            float y_mask = SmoothStep(1.0f, 0.88f, y_dist);
            e = e * x_mask * y_mask;

            // --- Redistribution / 重分布 ---
            // Push down valleys to separate continents clearly.
            // 稍微加强对比度，让海峡更清晰。
            e = std::pow(e, 1.3f);

            // --- Moisture & Temperature / 湿度与温度 ---
            // (Same logic as before, works well)
            float m = moisture_noise.octaveNoise(u_macro * 0.8f + 500, v_macro * 0.8f + 500, 3, 0.5f);
            m += (1.0f - e) * 0.25f; // Oceans add moisture
            m = std::max(0.0f, std::min(1.0f, m));

            float temp = 1.0f - std::abs(ny - 0.5f) * 2.0f; // Latitude
            temp += kTempBias;
            temp -= e * 0.45f; // Height cooling
            temp += warp_noise.octaveNoise(nx * 5 + kOffsetX, ny * 5 + kOffsetY, 1, 0.5f) * 0.15f;
            temp = std::max(0.0f, std::min(1.0f, temp));

            tile.height = e;
            tile.moisture = m;
            tile.temperature = temp;

            // --- Classification / 地形分类 ---
            if (e < kSeaLevel) {
                tile.type = TerrainType::OCEAN; // BFS will fix coasts
            }
            else {
                // Land logic
                if (e > 0.90f) {
                    tile.type = TerrainType::MOUNTAIN;
                }
                else {
                    bool is_cold = (temp < 0.18f);
                    bool is_dry = (m < 0.22f);

                    if (is_cold) {
                        tile.type = (temp < 0.08f) ? TerrainType::SNOW : TerrainType::TUNDRA;
                    }
                    else if (is_dry) {
                        tile.type = TerrainType::DESERT;
                    }
                    else {
                        // Greenery Balance
                        if (m < 0.52f) {
                            tile.type = TerrainType::PLAINS;
                        }
                        else if (temp > 0.72f) {
                            tile.type = TerrainType::JUNGLE;
                        }
                        else {
                            tile.type = TerrainType::GRASSLAND;
                        }
                    }
                }
            }
            map_data[hex] = tile;
        }
    }

    // --------------------------------------------------------------------------
    // 4. Pass 2: BFS Coastline (No changes needed, this part is solid)
    // --------------------------------------------------------------------------
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
                    next_to_land = true; break;
                }
            }
        }
        if (next_to_land) {
            data.type = TerrainType::COAST;
            visited[current] = true;
            frontier.push({ current, 1 });
        }
    }

    const int kMaxCoastWidth = 2; // 2 tiles wide shallow water
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

    return map_data;
}