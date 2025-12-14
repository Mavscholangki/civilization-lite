#ifndef __CITY_PRODUCTION_PANEL_H__
#define __CITY_PRODUCTION_PANEL_H__


#include "cocos2d.h"
#include "City/BaseCity.h"
#include "UI/CocosGUI.h"

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


class CityProductionPanel : public cocos2d::Layer {
public:
	bool init();
	CREATE_FUNC(CityProductionPanel);

	void addListItem(cocos2d::ui::ScrollView* listView, Node* item);
	void createNewButtonItem(std::string name, PanelItem::ItemType toWhichList, int cost);
	void createNewLabelItem(std::string text, PanelItem::ItemType toWhichList);
private:
	bool visible;
	cocos2d::ui::Layout* PanelBG;

	cocos2d::ui::Button* Pull;
	cocos2d::ui::Button* ProductButton;
	cocos2d::ui::Button* PurchaseButton;
	cocos2d::ui::ScrollView* ProductList;
	cocos2d::ui::ScrollView* PurchaseList;
};


#endif // __CITY_PRODUCTION_PANEL_H__

