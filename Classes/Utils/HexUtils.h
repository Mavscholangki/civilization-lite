#ifndef __HEX_UTILS_H__
#define __HEX_UTILS_H__

#include "cocos2d.h"
#include <cmath>
#include <algorithm>

/**
 * @brief 六边形坐标系统
 * 使用立方体坐标系统 (Cube Coordinates): q, r, s 三个坐标，满足约束条件 q + r + s = 0
 */
struct Hex {
    /** @brief 立方体坐标Q轴分量 */
    int q;
    /** @brief 立方体坐标R轴分量 */
    int r;
    /** @brief 立方体坐标S轴分量（自动计算为 -q-r） */
    int s;

    /**
     * @brief 默认构造函数，初始化为原点 (0,0,0)
     */
    Hex() 
        : q(0)
        , r(0)
        , s(0) 
    {
    }

    /**
     * @brief 参数构造函数
     * @param _q 坐标分量 q
     * @param _r 坐标分量 r
     */
    Hex(int _q, int _r) 
        : q(_q)
        , r(_r)
        , s(-_q - _r) 
    {
    }

    /**
     * @brief 判断两个坐标是否相等
     * @param other 另一个坐标
     * @return 如果坐标完全一致则返回 true
     */
    bool operator==(const Hex& other) const {
        return q == other.q && r == other.r;
    }

    /**
     * @brief 判断两个坐标是否不相等
     * @param other 另一个坐标
     * @return 如果坐标完全一致则返回 false
     */
    bool operator!=(const Hex& other) const {
        return !(*this == other);
    }
    /**
     * @brief 比较坐标大小，主要用于 std::map 使用坐标作为键
     * @param other 另一个坐标
     * @return 返回比较结果
     */
    bool operator<(
        const Hex& other) const {
        if (q != other.q) {
            return q < other.q;
        }
        return r < other.r;
    }

    /**
     * @brief 两个坐标的向量和
     * @param other 要相加的偏移坐标
     * @return 新的坐标结果
     */
    Hex operator+(const Hex& other) const {
        return Hex(q + other.q, r + other.r);
    }

    /**
     * @brief 计算当前坐标与另一个坐标之间的距离
     * @param other 目标坐标
     * @return 整数距离值（六边形间距离）
     */
    int distance(const Hex& other) const {
        int dq = std::abs(q - other.q);
        int dr = std::abs(r - other.r);
        int ds = std::abs(s - other.s);
        return (dq + dr + ds) / 2;
    }
};

/**
 * @brief 六边形布局类
 * 处理六边形坐标与屏幕像素坐标之间的转换 (采用 Pointy-Topped 布局)
 */
class HexLayout {
public:
    /** @brief 六边形边长（实际上是从中心到顶点的距离） */
    float size;

    /**
     * @brief 构造函数
     * @param _size 设定六边形的大小（半径）
     */
    HexLayout(float _size) 
        : size(_size) 
    {
    }

    /**
     * @brief 把六边形坐标转换为屏幕坐标（像素）
     * @param h 六边形的坐标
     * @return 对应的 cocos2d::Vec2 屏幕位置
     */
    cocos2d::Vec2 hexToPixel(Hex h) {
        // 强制转换 sqrt 的结果 (double) 为 float
        float x = size * static_cast<float>(sqrt(3.0)) * (h.q + h.r / 2.0f);
        float y = size * 3.0f / 2.0f * h.r;
        return cocos2d::Vec2(x, y);
    }

    /**
     * @brief 把屏幕坐标转换为六边形坐标（向下取整）
     * @param p 像素点位置 (cocos2d::Vec2)
     * @return 转换后四舍五入的 Hex 坐标
     */
    Hex pixelToHex(cocos2d::Vec2 p) {
        // 强制转换
        float q = (static_cast<float>(sqrt(3.0)) / 3.0f * p.x - 1.0f / 3.0f * p.y) / size;
        float r = (2.0f / 3.0f * p.y) / size;
        return hexRound(q, r);
    }

private:
    /**
     * @brief 将分数坐标四舍五入为标准坐标，确保约束条件满足
     * 当坐标多边形时，如果产生浮点误差，此函数确保修正
     * @param fracQ 分数坐标 Q
     * @param fracR 分数坐标 R
     * @return 四舍五入后的标准 Hex 坐标
     */
    Hex hexRound(float fracQ, float fracR) {
        float fracS = -fracQ - fracR;
        int q = static_cast<int>(std::round(fracQ));
        int r = static_cast<int>(std::round(fracR));
        int s = static_cast<int>(std::round(fracS));

        double q_diff = std::abs(q - fracQ);
        double r_diff = std::abs(r - fracR);
        double s_diff = std::abs(s - fracS);

        if (q_diff > r_diff && q_diff > s_diff) {
            q = -r - s;
        }
        else if (r_diff > s_diff) {
            r = -q - s;
        }
        return Hex(q, r);
    }
};

#endif