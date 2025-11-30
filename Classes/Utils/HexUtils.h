#ifndef __HEX_UTILS_H__
#define __HEX_UTILS_H__

#include "cocos2d.h"
#include <cmath>
#include <algorithm>

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
};

class HexLayout {
public:
    float size;
    HexLayout(float _size) : size(_size) {}

    cocos2d::Vec2 hexToPixel(Hex h) {
        float x = size * sqrt(3.0) * (h.q + h.r / 2.0);
        float y = size * 3.0 / 2.0 * h.r;
        return cocos2d::Vec2(x, y);
    }

    Hex pixelToHex(cocos2d::Vec2 p) {
        float q = (sqrt(3.0) / 3.0 * p.x - 1.0 / 3.0 * p.y) / size;
        float r = (2.0 / 3.0 * p.y) / size;
        return hexRound(q, r);
    }

private:
    Hex hexRound(float fracQ, float fracR) {
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