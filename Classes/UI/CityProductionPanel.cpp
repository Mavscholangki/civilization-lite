#include "CityProductionPanel.h"
USING_NS_CC;

//// CityProductionPanel 类实现 ////
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

	// 生产面板
	productionPanel = ProductionPanel::create();
	PanelBG->addChild(productionPanel);

	// 人口分配面板
	populationPanel = PopulationDistributionPanel::create();
	PanelBG->addChild(populationPanel);

	currentShownPanel = productionPanel; // 默认显示生产列表
	populationPanel->setVisible(false); // 初始隐藏人口分配面板

	// 面板拉出按钮
	this->Pull = ui::Button::create();
	this->Pull->setTitleText(" <[ ");
	this->Pull->setTitleColor(Color3B(204, 204, 0)); // 黄色文字
	this->Pull->setTitleFontSize(36);
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

	// 显示生产面板按钮
	this->showProductionPanel = ui::Button::create();
	this->showProductionPanel->setTitleText("[ Production ]");
	this->showProductionPanel->setTitleColor(Color3B(204, 204, 0)); // 黄色文字
	this->showProductionPanel->setTitleFontSize(24);
	this->showProductionPanel->setPosition(Vec2(-100, 150)); // 初始位置在面板外
	this->showProductionPanel->addClickEventListener([=](Ref* sender) {
		if (currentShownPanel == productionPanel)
		{
			// 切回人口分配面板
			currentShownPanel = populationPanel;
			this->populationPanel->setVisible(true);
			this->productionPanel->setVisible(false);
		}
		else
		{
			// 切到生产面板
			currentShownPanel = productionPanel;
			this->populationPanel->setVisible(false);
			this->productionPanel->setVisible(true);
		}
	});
	PanelBG->addChild(this->showProductionPanel);

	// 显示人口分配面板按钮
	this->showPopulationPanel = ui::Button::create();
	this->showPopulationPanel->setTitleText("[ Population Distribution ]");
	this->showPopulationPanel->setTitleColor(Color3B(204, 204, 0)); // 黄色文字
	this->showPopulationPanel->setTitleFontSize(24);
	this->showPopulationPanel->setPosition(Vec2(-100, 200)); // 初始位置在面板外
	this->showPopulationPanel->addClickEventListener([=](Ref* sender) {
		if (currentShownPanel == populationPanel)
		{
			// 切回生产面板
			currentShownPanel = this;
			this->populationPanel->setVisible(false);
			this->productionPanel->setVisible(true);
		}
		else
		{
			// 切到人口分配面板
			currentShownPanel = populationPanel;
			this->populationPanel->setVisible(true);
			this->productionPanel->setVisible(false);

		}
	});
	PanelBG->addChild(this->showPopulationPanel);

	return true;
}

//// PopulationDistributionPanel 类实现 ////
bool PopulationDistributionPanel::init()
{
	if (!ui::Layout::init())
	{
		return false;
	}
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto panelSize = Size(visibleSize.width / 7, visibleSize.height);

	// 初始化人口分配面板的逻辑
	visible = false; // 初始为隐藏状态

	this->workedTilesList = ui::ScrollView::create();
	this->workedTilesList->setDirection(ui::ScrollView::Direction::VERTICAL); // 垂直滚动
	this->workedTilesList->setBounceEnabled(true);
	this->workedTilesList->setContentSize(panelSize); // 和productionPanel大小相同
	this->workedTilesList->setScrollBarWidth(5);
	this->addChild(this->workedTilesList);

	return true;
}
void PopulationDistributionPanel::updatePanel(std::map<Hex, int> populationDistribution, int population)
{
	// 清空当前列表
	this->workedTilesList->removeAllChildren();
	currentDistribution = populationDistribution;
	// 重新添加地块信息
	for (auto tile : currentDistribution)
	{
		bool isWorked = (currentDistribution[tile.first] > 0);
		createNewItem(tile.first, isWorked);
	}
}
void PopulationDistributionPanel::manualDistribute(Hex tileIndex)
{
	// 手动分配人口到指定地块的逻辑

}
void PopulationDistributionPanel::addListItem(Node* item)
{
	auto innerContainer = workedTilesList->getInnerContainer();
	float itemHeight = item->getContentSize().height;
	float spacing = 15.f; // item之间的间距

	// 计算所有已有item的总高度（包括间距）
	float totalChildrenHeight = 0;
	auto children = innerContainer->getChildren();
	for (auto child : children) {
		totalChildrenHeight += child->getContentSize().height + spacing;
	}

	// 计算新item的Y位置（从容器的顶部开始往下排列）
	float yPos = innerContainer->getContentSize().height - itemHeight / 2 - spacing - totalChildrenHeight;

	// 设置位置
	item->setPosition(Vec2(workedTilesList->getContentSize().width / 2, yPos));

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
void PopulationDistributionPanel::createNewItem(Hex tile, bool isWorked) // 创建新的地块项目
{
	auto item = ui::Button::create();
	item->setTitleText("Tile (" + std::to_string(tile.q) + ", " + std::to_string(tile.r) + ")");
	item->setTitleColor(isWorked ? Color3B::BLUE : Color3B::BLACK); // 已耕作为蓝色，未耕作为白色
	item->setTitleFontSize(22);
	item->setTitleFontSize(18);
	item->setContentSize(Size(this->workedTilesList->getContentSize().width - 20, 40));
	// 为按钮添加背景
	auto bg = ui::Layout::create();
	bg->setContentSize(Size(item->getContentSize().width + 20, item->getContentSize().height + 10));
	bg->setBackGroundColor(Color3B(153, 204, 255)); // 浅蓝色背景
	bg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	bg->setAnchorPoint(Vec2(0.5f, 0.5f));
	bg->setPosition(Vec2(item->getContentSize().width / 2, item->getContentSize().height / 2));
	item->addChild(bg, -1);
	// 点击按钮时手动分配人口
	item->addClickEventListener([=](Ref* sender) {
		if (selected)
		{
			if (selectedTile == tile)
			{
				// 取消选择该地块
				selected = false;
				item->setTitleFontSize(18); // 字体恢复原大小
				bg->setPosition(Vec2(item->getContentSize().width / 2, item->getContentSize().height / 2));
			}
			else
			{
				// 将人口从selectedTile调到tile
				if (currentDistribution[selectedTile] == 1 && currentDistribution[tile] == 0) // 交换状态
				{
					currentDistribution[tile] = 1;
					item->setTitleColor(Color3B::BLUE); // 变为已耕作颜色
					currentDistribution[selectedTile] = 0;
					selectedItem->setTitleColor(Color3B::BLACK); // 之前选中的地块变为未耕作颜色
				}
				else if (currentDistribution[selectedTile] == 0 && currentDistribution[tile] == 1)
				{
					currentDistribution[tile] = 0;
					item->setTitleColor(Color3B::WHITE); // 变为未耕作颜色
					currentDistribution[selectedTile] = 1;
					selectedItem->setTitleColor(Color3B::BLACK); // 之前选中的地块变为已耕作颜色
				}
				selectedItem->setTitleFontSize(18); // 之前选中的地块字体恢复原大小
				bg->setPosition(Vec2(item->getContentSize().width / 2, item->getContentSize().height / 2));
				selected = false;
			}
		}
		else
		{
			// 选择该地块
			selected = true;
			selectedTile = tile;
			selectedItem = item;
			// 选中时字体变大
			item->setTitleFontSize(22);
			bg->setPosition(Vec2(item->getContentSize().width / 2, item->getContentSize().height / 2));
		}
	});
	
	addListItem(item);
}

//// ProductionPanel 类实现 ////
bool ProductionPanel::init()
{
	if (!ui::Layout::init())
	{
		return false;
	}
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto panelSize = Size(visibleSize.width / 7, visibleSize.height);
	
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
	this->addChild(this->ProductButton);

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
	this->addChild(this->PurchaseButton);

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
	this->addChild(this->ProductList);

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
	this->addChild(this->PurchaseList);

	return true;
}
void ProductionPanel::addListItem(cocos2d::ui::ScrollView* listView, Node* item)
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
void ProductionPanel::createNewButtonItem(std::string name, PanelItem::ItemType toWhichList, int cost)
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
void ProductionPanel::createNewLabelItem(std::string text, PanelItem::ItemType toWhichList)
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
