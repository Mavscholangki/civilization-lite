#ifndef __HEX_UTILS_H__
#define __HEX_UTILS_H__

#include "cocos2d.h"
#include <cmath>
#include <algorithm>

// 六边形坐标类 (使用立方坐标系的q,r,s三轴表示法，满足q + r + s = 0)
struct Hex { 
    int q, r, s;

    Hex() : q(0), r(0), s(0) {}

    Hex(int _q, int _r) : q(_q), r(_r), s(-_q - _r) {}

    bool operator==(const Hex& other) const {
        return q == other.q && r == other.r;
    }

    bool operator<(const Hex& other) const {
        if (q != other.q) return q < other.q;
        return r < other.r;
    }

    Hex operator+(const Hex& other) const {
        return Hex(q + other.q, r + other.r);
    }
    int distance(const Hex& other) const {
        int dq = std::abs(q - other.q);
        int dr = std::abs(r - other.r);
        int ds = std::abs(s - other.s);
        // 距离 = (dq + dr + ds) / 2
        return (dq + dr + ds) / 2;
    }
};
// 六边形布局类 (点y向下，0度在右侧水平向右)
class HexLayout {
public:
	float size; // 六边形大小(从中心到任意顶点的距离)
	HexLayout(float _size) : size(_size) {} // 构造函数

	cocos2d::Vec2 hexToPixel(Hex h) { // 六边形坐标转像素坐标
        float x = size * sqrt(3.0) * (h.q + h.r / 2.0);
        float y = size * 3.0 / 2.0 * h.r;
        return cocos2d::Vec2(x, y);
    }

	Hex pixelToHex(cocos2d::Vec2 p) { // 像素坐标转六边形坐标
        float q = (sqrt(3.0) / 3.0 * p.x - 1.0 / 3.0 * p.y) / size;
        float r = (2.0 / 3.0 * p.y) / size;
        return hexRound(q, r);
    }

private:
	Hex hexRound(float fracQ, float fracR) { // 四舍五入取整
        float fracS = -fracQ - fracR;
        int q = std::round(fracQ);
        int r = std::round(fracR);
        int s = std::round(fracS);

        double q_diff = std::abs(q - fracQ);
        double r_diff = std::abs(r - fracR);
        double s_diff = std::abs(s - fracS);

        if (q_diff > r_diff && q_diff > s_diff) q = -r - s;
        else if (r_diff > s_diff) r = -q - s;
        return Hex(q, r);
    }
};
#endif