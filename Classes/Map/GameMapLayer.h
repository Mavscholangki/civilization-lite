/**
 * @file GameMapLayer.h
 * @brief 游戏地图层
 * 
 * 管理游戏地图的渲染、单位管理、战斗系统等
 * 这是游戏的主要交互层，处理触摸输入、单位移动、战斗等
 */

#ifndef __GAME_MAP_LAYER_H__
#define __GAME_MAP_LAYER_H__

#include <chrono>
#include "cocos2d.h"
#include <map>
#include "../Utils/HexUtils.h"
#include "TileData.h"
#include "../Units/Base/AbstractUnit.h"
#include <functional> 
#include "../Units/Civilian/Settler.h"
#include "../City/BaseCity.h"

class BaseCity;

/**
 * @class GameMapLayer
 * @brief 游戏地图层
 * 
 * 职责：
 * - 六边形网格地图的生成和渲染
 * - 地块资源的显示（食物、生产力等）
 * - 单位的管理和移动
 * - 单位之间的战斗系统
 * - 城市的建立和管理
 * - 玩家输入处理（触摸）
 * 
 * @note 此类继承自 cocos2d::Layer，作为场景的主要内容层
 */
class GameMapLayer : public cocos2d::Layer {
public:
    /**
     * @brief 初始化地图层
     * @return true 初始化成功，false 失败
     * 
     * 此方法会：
     * 1. 生成六边形网格地图
     * 2. 创建玩家单位
     * 3. 创建敌对单位（演示用）
     * 4. 注册触摸事件监听器
     */
    virtual bool init();

    /**
     * @brief 初始化GameManager和玩家类
     * @return true 初始化成功，false 失败
     *
     * 此方法会：
     * 1. 创建玩家初始开拓者
     * 2. 绑定到当前玩家
     */
    void initGameManagerAndPlayers();
    
    /**
     * @brief 创建函数
     * @return 自动释放的 GameMapLayer 指针
     * 
     * 使用示例：
     * @code
     * GameMapLayer* layer = GameMapLayer::create();
     * scene->addChild(layer);
     * @endcode
     */
    CREATE_FUNC(GameMapLayer);

    /**
     * @brief 设置单位选中回调函数
     * @param cb 回调函数，参数为选中的单位指针
     * 
     * 当玩家点击选中单位时会调用此回调
     */
    void setOnUnitSelectedCallback(const std::function<void(AbstractUnit*)>& cb);

    /**
	* @brief 设置城市选中回调函数
    * @param cb 回调函数，参数为选中的单位指针
    * 
    * 当玩家点击选中城市时会调用此回调
    */
    void setOnCitySelectedCallback(const std::function<void(BaseCity*)>& cb);
    void setOnInvalidSeletedCallback(const std::function<void()>& cb);  ///< 城市选中回调

    /**
     * @brief 处理建立城市的动作
     * 
     * 此方法会：
     * 1. 检查选中的单位是否能建城
     * 2. 在单位位置建立城市
     * 3. 移除定居者单位
     * 4. 更新地图显示
     */
    void onBuildCityAction();

    /**
     * @brief 处理回合结束
     * 
     * 此方法会：
     * 1. 更新所有城市的产出
     * 2. 重置单位的移动力
     * 3. 触发回合相关事件
     */
    void onNextTurnAction();
	TileData getTileData(Hex h);

    //========地块选择模式========//

    /**
     * @brief 启用地块选择模式
     * @param allowedTiles 允许选择的地块数组
     * @param callback 选择完成后的回调函数
     *
     * 进入此模式后，玩家可以点击地图选择地块
     * 只有 allowedTiles 中的地块可以被选择
     * 点击其他地块会被忽略
     */
    void enableTileSelection(const std::vector<Hex>& allowedTiles,
        const std::function<void(Hex)>& callback,
        const std::function<void()>& cancelCallback);


    /**
    * @brief 禁用地块选择模式
    */
   
    void disableTileSelection(bool triggerCancelCallback);
    /**
     * @brief 检查是否在选择模式下
     * @return true 如果在地块选择模式下
     */
    bool isSelectingTile() const { return _isSelectingTile; }

    /**
     * @brief 高亮显示可选地块
     * @param allowedTiles 需要高亮的地块数组
     */
    void highlightAllowedTiles(const std::vector<Hex>& allowedTiles);

    /**
     * @brief 清除所有高亮显示
     */
    void clearHighlights();

    void handleTileSelection(Touch* touch);
    void showSelectionMarker(Hex hex);

    void clearSelectionMarker();

    /**
        * @brief 强制退出手动选择模式
        * @param cancelCallback 是否触发取消回调（可选，默认不触发）
        *
        * 当玩家改变主意或需要取消当前选择操作时调用
        */
    void cancelTileSelection(bool cancelCallback = false);
    void GameMapLayer::showInvalidSelectionFeedback(Hex hex);
    HexLayout* getLayout() const { return _layout; }                    ///< 六边形布局对象


private:
    // 添加取消回调函数
    std::function<void()> _onSelectionCancelled;
    /**
     * @brief 生成六边形网格地图
     * 
     * 此方法会：
     * 1. 创建指定大小的六边形网格
     * 2. 根据地形类型分配颜色
     * 3. 显示地块资源信息
     */
    void generateMap();

    /**
     * @brief 向合并的 DrawNode 中绘制六边形
     * @param node 目标 DrawNode
     * @param pos 屏幕像素坐标
     * @param size 六边形大小（像素）
     * @param color 六边形颜色
     * 
     * @note 性能优化：所有六边形共用一个 DrawNode，而非每个都创建新的
     */
    void drawHexOnNode(cocos2d::DrawNode* node, cocos2d::Vec2 pos, float size, cocos2d::Color4F color);

    /**
     * @brief 在地块上显示资源信息
     * @param hex 六边形坐标
     * @param data 地块数据
     * 
     * 此方法会在地块中心显示：粮食、生产力、金币、科技、文化产出
     */
    void drawTileResources(Hex hex, const TileData& data);

    /**
     * @brief 更新地块选中显示
     * @param h 要选中的六边形坐标
     * 
     * 此方法会在指定地块周围绘制黄色边框
     */
    void updateSelection(Hex h);

    /**
     * @brief 获取地块的移动成本
     * @param h 六边形坐标
     * @return 移动成本（正数），或 -1 表示无法通行
     * 
     * 不同地形的成本：
     * - 草地、平原：1
     * - 丛林、沙漠、雪地：2
     * - 山脉、大海、海岸：-1（不可通行）
     */
    int getTerrainCost(Hex h);
    

    /**
     * @brief 检查两点之间的距离是否在范围内
     * @param from 起始六边形坐标
     * @param to 目标六边形坐标
     * @param range 范围（格数）
     * @return true 在范围内，false 超出范围
     * 
     * 使用曼哈顿距离计算
     */
    bool isHexWithinRange(Hex from, Hex to, int range);

    /**
     * @brief 处理单位的攻击动作
     * @param attacker 攻击方单位指针
     * @param targetHex 目标位置的六边形坐标
     * 
     * 此方法会：
     * 1. 查找目标位置的敌方单位
     * 2. 验证攻击的有效性
     * 3. 执行战斗逻辑
     */
    void handleUnitAttack(AbstractUnit* attacker, Hex targetHex);

    

    // 用于判断是否在拖拽地图
    cocos2d::Vec2 _mouseDownPos;
    // ============ 成员变量 ============
    HexLayout* _layout;                    ///< 六边形布局对象
    AbstractUnit* _myUnit;                 ///< 玩家控制的主要单位
    cocos2d::DrawNode* _selectionNode;     ///< 选中地块的边框显示
    cocos2d::DrawNode* _tilesDrawNode;     ///< 所有六边形的合并 DrawNode
    bool _isDragging;                      ///< 是否正在拖拽地图
    std::function<void(AbstractUnit*)> _onUnitSelected; ///< 单位选中回调
    std::map<Hex, TileData> _mapData;      ///< 地块数据（地形、资源等）
    std::vector<BaseCity*> _cities;        ///< 所有城市列表
    std::vector<AbstractUnit*> _allUnits;  ///< 所有单位列表（包括敌方）
    AbstractUnit* _selectedUnit;           ///< 当前选中的单位
    std::function<void(BaseCity*)> _onCitySelected;  ///< 城市选中回调
    std::function<void()> _onInvalidSelected;  ///< 无效选中
	BaseCity* getCityAt(Hex hex);   ///<  获取指定位置的城市指针（无城市返回 nullptr）
    AbstractUnit* getUnitAt(Hex h);

    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    std::chrono::steady_clock::time_point _lastClickTime;
    Hex _lastClickHex;

    bool isValidStartingPosition(Hex centerHex);

private:
    // 选择模式相关成员变量
    bool _isSelectingTile;
    std::vector<Hex> _allowedTiles;
    std::function<void(Hex)> _tileSelectionCallback;
    cocos2d::DrawNode* _highlightNode;  // 用于绘制高亮地块
    std::vector<cocos2d::Node*> _highlightLabels;  // 高亮地块上的标签

};

#endif // __GAME_MAP_LAYER_H__