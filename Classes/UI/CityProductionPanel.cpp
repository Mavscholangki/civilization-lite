#include "CityProductionPanel.h"
USING_NS_CC;

bool CityProductionPanel::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto panelSize = Size(visibleSize.width / 7, visibleSize.height);

	// 背景面板
	this->PanelBG = ui::Layout::create();
	PanelBG->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	PanelBG->setBackGroundColor(Color3B(0, 76, 153)); // 深灰色背景
	PanelBG->setContentSize(panelSize);
	PanelBG->setPosition(Vec2(visibleSize.width - panelSize.width, 0));
	this->addChild(PanelBG);

	// 按钮切换: 生产
	this->ProductButton = ui::Button::create();
	this->ProductButton->setTitleText("[ Product ]");
	this->ProductButton->setTitleColor(Color3B(204, 204, 0)); // 黄色文字
	this->ProductButton->setTitleFontSize(20);
	this->ProductButton->setPosition(Vec2(60, panelSize.height - 30));
	this->ProductButton->addClickEventListener([=](Ref* sender) {
			this->ProductList->setVisible(true);
			this->PurchaseList->setVisible(false);
		});
	PanelBG->addChild(this->ProductButton);

	// 按钮切换: 购买
	this->PurchaseButton = ui::Button::create();
	this->PurchaseButton->setTitleText("[ Purchase ]");
	this->PurchaseButton->setTitleColor(Color3B(255, 255, 51)); // 浅黄色文字
	this->PurchaseButton->setTitleFontSize(20);
	this->PurchaseButton->setPosition(Vec2(50 + panelSize.width / 2, panelSize.height - 30));
	this->PurchaseButton->addClickEventListener([=](Ref* sender) {
		this->ProductList->setVisible(false);
		this->PurchaseList->setVisible(true);
		});
	PanelBG->addChild(this->PurchaseButton);

	// 生产列表
	this->ProductList = ui::ScrollView::create();
	this->ProductList->setDirection(ui::ScrollView::Direction::VERTICAL); // 垂直滚动
	this->ProductList->setBounceEnabled(true);
	this->ProductList->setContentSize(Size(panelSize.width - 20, panelSize.height - 50));
	this->ProductList->setPosition(Vec2(10, 10));
	this->ProductList->setInnerContainerSize(Size(panelSize.width - 20, 0)); // 初始内容大小
	this->ProductList->setVisible(true); // 默认显示生产列表
	createNewLabelItem("     Districts and Buildings     ", PanelItem::ItemType::PRODUCT); 
	createNewButtonItem("Memorial Hall", PanelItem::ItemType::PRODUCT, 6);
	createNewLabelItem("             Units             ", PanelItem::ItemType::PRODUCT);
	createNewButtonItem("Warrior", PanelItem::ItemType::PRODUCT, 4);
	createNewButtonItem("Constructor", PanelItem::ItemType::PRODUCT, 5);
	PanelBG->addChild(this->ProductList);

	// 购买列表
	this->PurchaseList = ui::ScrollView::create();
	this->PurchaseList->setDirection(ui::ScrollView::Direction::VERTICAL); // 垂直滚动
	this->PurchaseList->setBounceEnabled(true);
	this->PurchaseList->setContentSize(Size(panelSize.width - 20, panelSize.height - 50));
	this->PurchaseList->setPosition(Vec2(10, 10));
	this->PurchaseList->setInnerContainerSize(Size(panelSize.width - 20, 0)); // 初始内容大小
	this->PurchaseList->setVisible(false); // 默认隐藏购买列表
	createNewLabelItem("     Districts and Buildings    ", PanelItem::ItemType::PURCHASE);
	createNewButtonItem("Granary", PanelItem::ItemType::PURCHASE, 60);
	createNewLabelItem("             Units            ", PanelItem::ItemType::PURCHASE);
	createNewButtonItem("Warrior", PanelItem::ItemType::PURCHASE, 40);
	createNewButtonItem("Settler", PanelItem::ItemType::PURCHASE, 100);
	PanelBG->addChild(this->PurchaseList);

	// 面板拉出按钮
	this->Pull = ui::Button::create();
	this->Pull->setTitleText("[ Production ]");
	this->Pull->setTitleColor(Color3B(204, 204, 0)); // 黄色文字
	this->Pull->setTitleFontSize(24);
	this->Pull->setPosition(Vec2(-100, 100));
	this->Pull->addClickEventListener([=](Ref* sender) {
		if (visible)
		{
			visible = false;
			auto moveBy = MoveBy::create(0.2f, Vec2(-panelSize.width, 0));
			PanelBG->runAction(moveBy);
		}
		else
		{
			visible = true;
			auto moveBy = MoveBy::create(0.2f, Vec2(panelSize.width, 0));
			PanelBG->runAction(moveBy);
		}
	});
	PanelBG->addChild(this->Pull);
	return true;
}

void CityProductionPanel::addListItem(cocos2d::ui::ScrollView* listView, Node* item)
{
	auto innerContainer = listView->getInnerContainer();
	float itemHeight = item->getContentSize().height;
	float spacing = 10.f; // item之间的间距

	// 计算所有已有item的总高度（包括间距）
	float totalChildrenHeight = 0;
	auto children = innerContainer->getChildren();
	for (auto child : children) {
		totalChildrenHeight += child->getContentSize().height + spacing;
	}

	// 计算新item的Y位置（从容器的顶部开始往下排列）
	float yPos = innerContainer->getContentSize().height - itemHeight / 2 - spacing - totalChildrenHeight;

	// 设置位置
	item->setPosition(Vec2(listView->getContentSize().width / 2, yPos));

	// 更新容器大小（如果需要）
	float newTotalHeight = totalChildrenHeight + itemHeight + spacing * 2; // 顶部和底部的间距
	if (newTotalHeight > innerContainer->getContentSize().height) {
		innerContainer->setContentSize(Size(
			innerContainer->getContentSize().width,
			newTotalHeight
		));

		// 调整所有item的位置，因为容器高度变了
		float heightDiff = newTotalHeight - innerContainer->getContentSize().height;
		for (auto child : children) {
			child->setPositionY(child->getPosition().y + heightDiff);
		}
		item->setPositionY(item->getPosition().y + heightDiff);
	}

	innerContainer->addChild(item);
}
void CityProductionPanel::createNewButtonItem(std::string name, PanelItem::ItemType toWhichList, int cost)
{
	auto newItem = ui::Button::create();
	newItem->setTitleText(name + "     cost:" + std::to_string(cost) + (toWhichList == PanelItem::ItemType::PRODUCT ? " turns" : " golds"));
	newItem->setTitleColor(Color3B::WHITE);
	newItem->setTitleFontSize(20);
	newItem->setContentSize(Size(this->ProductList->getContentSize().width - 20, 50));
	if (toWhichList == PanelItem::ItemType::PRODUCT)
	{
		addListItem(this->ProductList, newItem);
	}
	else
	{
		addListItem(this->PurchaseList, newItem);
	}
}
void CityProductionPanel::createNewLabelItem(std::string text, PanelItem::ItemType toWhichList)
{
	auto newItem = Label::createWithSystemFont(text, "Arial", 22);
	newItem->setColor(Color3B(0, 76, 153)); // 蓝色文字
	newItem->setContentSize(Size(this->ProductList->getContentSize().width - 20, 50));
	auto bg = ui::Layout::create();
	bg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	bg->setBackGroundColor(Color3B(153, 204, 255)); // 浅蓝色背景
	bg->setContentSize(newItem->getContentSize());
	newItem->addChild(bg, -1);
	if (toWhichList == PanelItem::ItemType::PRODUCT)
	{
		addListItem(this->ProductList, newItem);
	}
	else
	{
		addListItem(this->PurchaseList, newItem);
	}
}