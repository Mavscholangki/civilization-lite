#ifndef __CITY_PRODUCTION_PANEL_H__
#define __CITY_PRODUCTION_PANEL_H__


#include "cocos2d.h"
#include "Utils/HexUtils.h"
#include "UI/CocosGUI.h"
#include "City/Yield.h"

USING_NS_CC;

class PopulationDistributionPanel : public cocos2d::ui::Layout {
public:
	bool init();
	CREATE_FUNC(PopulationDistributionPanel);

	void updatePanel(std::map<Hex, int> populationDistribution, int population);
	void manualDistribute(Hex tileIndex); // 手动分配人口到指定地块
	void addListItem(Node* item); // 向列表中添加项目
	void createNewItem(Hex tile, bool isWorked); // 创建新的地块项目
	std::map<Hex, int> currentDistribution; // 当前分配状态
	ui::ScrollView* workedTilesList; // 显示耕作地块的列表
private:
	bool visible;
	bool selected; // 有地块被选中将调走人口，否则为false，与selectedTile配合使用
	Hex selectedTile; // 选中的地块(当手动调整时用到)
	ui::Button* selectedItem; // 选中的地块对应的按钮
	ui::Layout* selectedItemBg; // 选中地块按钮的背景
	Label* title; // 标题(带有人口信息)
};

class PanelItem : public cocos2d::ui::Button {
public:
	bool init()
	{
		if (!Button::init()) return false;
		auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();

		itemName = "To Be Defined";
		itemType = ItemType::PRODUCT;
		cost = 4;
		this->setContentSize(cocos2d::Size(visibleSize.width / 7 - 20, 50));
		this->setTitleText(itemName);
		this->setTitleFontSize(20);
		this->setTitleColor(cocos2d::Color3B::WHITE);

		return true;
	}
	CREATE_FUNC(PanelItem);

	enum class ItemType {
		PRODUCT,
		PURCHASE
	};

	void setInfo(std::string name, PanelItem::ItemType type, int cost)
	{
		setItemName(name);
		setItemType(type);
		setCost(cost);
	}

	void				setItemName(std::string name) { itemName = name; }
	std::string			getItemName() { return itemName; }
	void				setItemType(PanelItem::ItemType type) { itemType = type; }
	PanelItem::ItemType getItemType() { return itemType; }
	void				setCost(int cost) { this->cost = cost; }
	int					getCost() { return this->cost; }

	std::string itemName;
	ItemType itemType;
	int cost; // for product, it costs turns; for purchases it costs golds
	void updateItem()
	{
		this->setTitleText(itemName + "     cost:" + std::to_string(cost) + (itemType == ItemType::PRODUCT ? " turns" : " golds"));
	}
};

class ProductionPanel : public cocos2d::ui::Layout {
public:
	bool init();
	CREATE_FUNC(ProductionPanel);
	void addListItem(cocos2d::ui::ScrollView* listView, Node* item);
	void createNewButtonItem(std::string name, PanelItem::ItemType toWhichList, int cost);
	void createNewLabelItem(std::string text, PanelItem::ItemType toWhichList);
private:
	cocos2d::ui::Button* ProductButton;
	cocos2d::ui::Button* PurchaseButton;
	cocos2d::ui::ScrollView* ProductList;
	cocos2d::ui::ScrollView* PurchaseList;
};

class CityProductionPanel : public cocos2d::Layer {
public:
	bool init();
	CREATE_FUNC(CityProductionPanel);

	PopulationDistributionPanel* populationPanel; // 人口分配面板
	ProductionPanel* productionPanel; // 生产面板
private:
	bool visible;
	cocos2d::ui::Layout* PanelBG;
	
	cocos2d::ui::Button* Pull; // 拉取按钮
	cocos2d::ui::Button* showProductionPanel; // 显示生产面板按钮
	cocos2d::ui::Button* showPopulationPanel; // 显示人口分配面板按钮
	cocos2d::Node* currentShownPanel; // 当前显示的面板
};


#endif // __CITY_PRODUCTION_PANEL_H__

