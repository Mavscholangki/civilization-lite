#ifndef __PERLIN_NOISE_H__
#define __PERLIN_NOISE_H__

#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
/**
* @brief Perlin Noise 实现类
* @details 提供 2D Perlin Noise 生成和分形布朗运动 (FBM) 功能
*/

class PerlinNoise {
public:
    // 初始化种子
    PerlinNoise(unsigned int seed = 0) {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);
        p.insert(p.end(), p.begin(), p.end());
    }

    // 获取 2D 噪声值 (返回 0.0 ~ 1.0)
    double noise(double x, double y) {
        // 这里的实现是标准的 Perlin Noise 算法
        // 为了篇幅，使用了简化的查找表实现
        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        x -= floor(x);
        y -= floor(y);
        double u = fade(x);
        double v = fade(y);
        int A = p[X] + Y, AA = p[A] + 0, AB = p[A] + 1;
        int B = p[X + 1] + Y, BA = p[B] + 0, BB = p[B] + 1;

        double res = lerp(v, lerp(u, grad(p[AA], x, y), grad(p[BA], x - 1, y)),
            lerp(u, grad(p[AB], x, y - 1), grad(p[BB], x - 1, y - 1)));

        // 归一化到 0.0 ~ 1.0
        return (res + 1.0) / 2.0;
    }

    // 分形叠加 (FBM) - 让地形更有细节 (海岸线更破碎)
    double octaveNoise(double x, double y, int octaves, double persistence) {
        double total = 0;
        double frequency = 1;
        double amplitude = 1;
        double maxValue = 0;
        for (int i = 0; i < octaves; i++) {
            total += noise(x * frequency, y * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2;
        }
        return total / maxValue;
    }

private:
    std::vector<int> p;
    double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    double lerp(double t, double a, double b) { return a + t * (b - a); }
    double grad(int hash, double x, double y) {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : h == 12 || h == 14 ? x : 0;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};

#endif