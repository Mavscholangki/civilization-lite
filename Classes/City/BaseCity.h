/*
* 城市基础类
*/
#ifndef __BASE_CITY_H__
#define __BASE_CITY_H__

#include "cocos2d.h"
#include "Utils/HexUtils.h"

class BaseCity : public cocos2d::Node {
public:
    Hex gridPos;

    static BaseCity* create(Hex pos, std::string name);
    bool initCity(Hex pos, std::string name);

    // 城市属性
    std::string cityName;
    int population;
    int goldYield;   // 每回合产金币
    int scienceYield;// 每回合产科技

    // 回合结算逻辑
    void onTurnEnd();

private:
    cocos2d::Label* _nameLabel;
    cocos2d::Node* _visual;
};

#endif