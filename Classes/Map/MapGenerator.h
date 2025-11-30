/*
* 这是地图生成器的头文件。
* 你不应该修改这个文件。
*/

#ifndef __MAP_GENERATOR_H__
#define __MAP_GENERATOR_H__

#include <map>
#include "Utils/HexUtils.h"
#include "TileData.h"
#include "cocos2d.h"

class MapGenerator {
public:
    // 静态函数：输入宽高，返回生成的地图数据
    static std::map<Hex, TileData> generate(int width, int height);
};

#endif