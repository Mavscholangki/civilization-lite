#include "CityProductionPanel.h"
#include "Core/GameManager.h"
#include "Map/GameMapLayer.h"
#include "Development/ProductionProgram.h"
#include "District/Building/Building.h"
#include "algorithm"
USING_NS_CC;

//// =================== CityProductionPanel 类实现 =================== ////
bool CityProductionPanel::init()
{
	if (!Layer::init())
	{
		return false;
	}

	this->currentPlayerID = 0;
	this->visible = false;
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto panelSize = Size(visibleSize.width / 5, visibleSize.height);

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
	this->Pull->setPosition(Vec2(-Pull->getContentSize().width, 100));
	this->Pull->addClickEventListener([=](Ref* sender) {
		if (visible)
		{
			visible = false;
			auto moveBy = MoveBy::create(0.2f, Vec2(-panelSize.width, 0));
			PanelBG->runAction(moveBy);
			showPopulationPanel->setVisible(true);
			showProductionPanel->setVisible(true);
		}
		else
		{
			visible = true;
			auto moveBy = MoveBy::create(0.2f, Vec2(panelSize.width, 0));
			PanelBG->runAction(moveBy);
			showPopulationPanel->setVisible(false);
			showProductionPanel->setVisible(false);
		}
	});
	PanelBG->addChild(this->Pull);

	// 显示生产面板按钮
	this->showProductionPanel = ui::Button::create();
	this->showProductionPanel->setTitleText("[ Production ]");
	this->showProductionPanel->setTitleColor(Color3B(204, 204, 0)); // 黄色文字
	this->showProductionPanel->setTitleFontSize(24);
	this->showProductionPanel->setPosition(Vec2(-showProductionPanel->getContentSize().width / 2, 150)); // 初始位置在面板外
	this->showProductionPanel->addClickEventListener([=](Ref* sender) {
		if (currentShownPanel != productionPanel)
		{
			// 切到生产面板
			currentShownPanel = productionPanel;
			this->populationPanel->setVisible(false);
			this->productionPanel->setVisible(true);
		}
	});
	PanelBG->addChild(this->showProductionPanel);

	// 显示生产面板按钮背景
	auto showProductionPanelBg = ui::Layout::create();
	showProductionPanelBg->setContentSize(Size(showProductionPanel->getContentSize().width + 10, showProductionPanel->getContentSize().height + 10));
	showProductionPanelBg->setBackGroundColor(Color3B(0, 76, 153)); // 深蓝色背景
	showProductionPanelBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	showProductionPanelBg->setAnchorPoint(Vec2(0.5f, 0.5f));
	showProductionPanelBg->setPosition(Vec2(showProductionPanel->getContentSize().width / 2, showProductionPanel->getContentSize().height / 2));
	this->showProductionPanel->addChild(showProductionPanelBg, -1);

	// 显示人口分配面板按钮
	this->showPopulationPanel = ui::Button::create();
	this->showPopulationPanel->setTitleText("[ Population Distribution ]");
	this->showPopulationPanel->setTitleColor(Color3B(204, 204, 0)); // 黄色文字
	this->showPopulationPanel->setTitleFontSize(24);
	this->showPopulationPanel->setPosition(Vec2(-showPopulationPanel->getContentSize().width / 2, 200)); // 初始位置在面板外
	this->showPopulationPanel->addClickEventListener([=](Ref* sender) {
		if (currentShownPanel != populationPanel)
		{
			// 切到人口分配面板
			currentShownPanel = populationPanel;
			this->populationPanel->setVisible(true);
			this->productionPanel->setVisible(false);

		}
	});
	PanelBG->addChild(this->showPopulationPanel);

	// 显示人口分配面板按钮背景
	auto showPopulationPanelBg = ui::Layout::create();
	showPopulationPanelBg->setContentSize(Size(showPopulationPanel->getContentSize().width + 10, showPopulationPanel->getContentSize().height + 10));
	showPopulationPanelBg->setBackGroundColor(Color3B(0, 76, 153)); // 深蓝色背景
	showPopulationPanelBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	showPopulationPanelBg->setAnchorPoint(Vec2(0.5f, 0.5f));
	showPopulationPanelBg->setPosition(Vec2(showPopulationPanel->getContentSize().width / 2, showPopulationPanel->getContentSize().height / 2));
	this->showPopulationPanel->addChild(showPopulationPanelBg, -1);
	return true;
}

void CityProductionPanel::updateProductionPanel(int playerID, BaseCity* currentCity, std::vector<ProductionProgram*> programs[3])
{
	this->productionPanel->clear();
	currentPlayerID = playerID;

	// Districts
	bool firstPurchase = false;
	if (!programs[0].empty())
	{
		productionPanel->createNewLabelItem(playerID, currentCity, "       Districts      ", PanelItem::ItemType::PRODUCT);
		for (auto d: programs[0])
		{
			if (currentCity)
			{
				bool isAlreadyExisting = false;
				for (auto currentDistrict : currentCity->districts)
				{
					if (d->getID() == currentDistrict->getID())
					{
						isAlreadyExisting = true;
						break;
					}
				}
				for (auto underProduction : currentCity->suspendedProductions)
				{
					if (d->getID() == underProduction->getID())
					{
						isAlreadyExisting = true;
						break;
					}
				}
				if (currentCity->currentProduction && d->getID() == currentCity->currentProduction->getID())
				{
					isAlreadyExisting = true;
				}
				if (isAlreadyExisting)
					continue;
			}
			else if (d->getName() == "Downtown")
				continue;
			auto district = new District(playerID, Hex(), District::DistrictType::TO_BE_DEFINED, d->getName());
			productionPanel->createNewButtonItem(playerID, currentCity, PanelItem::ItemType::PRODUCT, district);
			if(district->getCanPurchase())
			{
				if (!firstPurchase)
				{
					productionPanel->createNewLabelItem(playerID, currentCity, "       Districts      ", PanelItem::ItemType::PURCHASE);
					firstPurchase = true;
				}
				productionPanel->createNewButtonItem(playerID, currentCity, PanelItem::ItemType::PURCHASE, district);
			}
		}
	}
	firstPurchase = false;
	if (!programs[1].empty())
	{
		productionPanel->createNewLabelItem(playerID, currentCity, "       Buildings      ", PanelItem::ItemType::PRODUCT);
		for (auto b : programs[1])
		{
			if (currentCity)
			{
				bool isAlreadyExisting = false;
				for (auto currentDistrict : currentCity->districts)
				{
					for (auto currentBuilding : currentDistrict->getBuildings())
					{
						if (b->getID() == currentBuilding->getID())
						{
							isAlreadyExisting = true;
							break;
						}
					}
					if (isAlreadyExisting)
						break;
				}
				for (auto underProduction : currentCity->suspendedProductions)
				{
					if (b->getID() == underProduction->getID())
					{
						isAlreadyExisting = true;
						break;
					}
				}
				if (currentCity->currentProduction && b->getID() == currentCity->currentProduction->getID())
				{
					isAlreadyExisting = true;
				}
				if (isAlreadyExisting)
					continue;
			}
			auto building = new Building(playerID, b->getName());
			productionPanel->createNewButtonItem(playerID, currentCity, PanelItem::ItemType::PRODUCT, building);
			if (building->getCanPurchase())
			{
				if (!firstPurchase)
				{
					productionPanel->createNewLabelItem(playerID, currentCity, "       Buildings      ", PanelItem::ItemType::PURCHASE);
					firstPurchase = true;
				}
				productionPanel->createNewButtonItem(playerID, currentCity, PanelItem::ItemType::PURCHASE, building);
			}
		}
	}
	firstPurchase = false;
	if (!programs[2].empty())
	{
		productionPanel->createNewLabelItem(playerID, currentCity, "         Units         ", PanelItem::ItemType::PRODUCT);
		for (auto u : programs[2])
		{
			auto unit = new AbstractUnit(u->getName());
			productionPanel->createNewButtonItem(playerID, currentCity, PanelItem::ItemType::PRODUCT, unit);
			if (unit->getCanPurchase())
			{
				if (!firstPurchase)
				{
					productionPanel->createNewLabelItem(playerID, currentCity, "         Units         ", PanelItem::ItemType::PURCHASE);
					firstPurchase = true;
				}
				productionPanel->createNewButtonItem(playerID, currentCity, PanelItem::ItemType::PURCHASE, unit);
			}
		}
	}
	productionPanel->updateCurrentPanel(currentCity);
	populationPanel->updatePanel(currentCity->populationDistribution, currentCity->population);
}

//// =============== PopulationDistributionPanel 类实现 =============== ////
bool PopulationDistributionPanel::init()
{
	if (!ui::Layout::init())
	{
		return false;
	}
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto panelSize = Size(visibleSize.width / 5, visibleSize.height - 50);

	// 初始化人口分配面板的逻辑
	visible = false; // 初始为隐藏状态

	this->workedTilesList = ui::ScrollView::create();
	this->workedTilesList->setDirection(ui::ScrollView::Direction::VERTICAL); // 垂直滚动
	this->workedTilesList->setBounceEnabled(true);
	this->workedTilesList->setContentSize(panelSize); // 和productionPanel大小相同
	this->workedTilesList->setScrollBarWidth(5);
	this->addChild(this->workedTilesList);

	// 标题
	this->title = Label::createWithSystemFont("Population Distribution", "Arial", 24);
	this->title->setTextColor(Color4B(Color3B(0, 76, 153)));
	this->title->setContentSize(Size(panelSize.width, 40));
	this->title->setPosition(Vec2(panelSize.width / 2, panelSize.height - title->getContentSize().height / 2 + 50));
	this->addChild(this->title);

	// 为标题添加背景
	auto titleBg = ui::Layout::create();
	titleBg->setContentSize(Size(panelSize.width, title->getContentSize().height + 10));
	titleBg->setBackGroundColor(Color3B(153, 204, 255));
	titleBg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	titleBg->setAnchorPoint(Vec2(0.5f, 0.5f));
	titleBg->setPosition(Vec2(panelSize.width / 2, title->getContentSize().height / 2));
	this->title->addChild(titleBg, -1);

	return true;
}

void PopulationDistributionPanel::updatePanel(std::map<Hex, int> populationDistribution, int population)
{
	int unallocated = population;
	// 清空当前列表
	this->workedTilesList->removeAllChildren();
	currentDistribution = populationDistribution;
	// 重新添加地块信息
	for (auto& tile : currentDistribution)
	{
		bool isWorked = (currentDistribution[tile.first] > 0);
		createNewItem(tile.first, isWorked);
		if (isWorked)
			unallocated--;
	}
	// 更新标题显示人口信息
	this->title->setString(
		"Unallocated: " + std::to_string(unallocated) + "/ Total: " + std::to_string(population));
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
	item->setTitleColor(isWorked ? Color3B::BLUE : Color3B(128, 128, 128)); // 已耕作为蓝色，未耕作为灰色
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
				bg->setBackGroundColor(Color3B(153, 204, 255)); // 恢复背景颜色
			}
			else
			{
				// 将人口从selectedTile调到tile
				if (currentDistribution[selectedTile] == 1 && currentDistribution[tile] == 0) // 交换状态
				{
					currentDistribution[tile] = 1;
					item->setTitleColor(Color3B::BLUE); // 变为已耕作颜色
					currentDistribution[selectedTile] = 0;
					selectedItem->setTitleColor(Color3B(128, 128, 128)); // 之前选中的地块变为未耕作颜色
				}
				else if (currentDistribution[selectedTile] == 0 && currentDistribution[tile] == 1)
				{
					currentDistribution[tile] = 0;
					item->setTitleColor(Color3B(128, 128, 128)); // 变为未耕作颜色
					currentDistribution[selectedTile] = 1;
					selectedItem->setTitleColor(Color3B::BLUE); // 之前选中的地块变为已耕作颜色
				}
				selectedItemBg->setBackGroundColor(Color3B(153, 204, 255)); // 恢复背景颜色
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
			selectedItemBg = bg;
			bg->setBackGroundColor(Color3B::YELLOW); // 黄色背景表示选中
		}
	});
	
	addListItem(item);
}

//// ===================== ProductionPanel 类实现 ===================== ////
bool ProductionPanel::init()
{
	if (!ui::Layout::init())
	{
		return false;
	}
	currentPlayer = -1;
	isSelectingTile = false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto panelSize = Size(visibleSize.width / 5, visibleSize.height);
	
	// 按钮切换: 生产
	this->ProductButton = ui::Button::create();
	this->ProductButton->setTitleText("[ Product ]");
	this->ProductButton->setTitleColor(Color3B(204, 204, 0)); // 黄色文字
	this->ProductButton->setTitleFontSize(20);
	this->ProductButton->setPosition(Vec2(70, panelSize.height - 30));
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
	this->PurchaseButton->setPosition(Vec2(60 + panelSize.width / 2, panelSize.height - 30));
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
	this->addChild(this->ProductList);

	// 购买列表
	this->PurchaseList = ui::ScrollView::create();
	this->PurchaseList->setDirection(ui::ScrollView::Direction::VERTICAL); // 垂直滚动
	this->PurchaseList->setBounceEnabled(true);
	this->PurchaseList->setContentSize(Size(panelSize.width - 20, panelSize.height - 50));
	this->PurchaseList->setPosition(Vec2(10, 10));
	this->PurchaseList->setInnerContainerSize(Size(panelSize.width - 20, 0)); // 初始内容大小
	this->PurchaseList->setVisible(false); // 默认隐藏购买列表
	this->addChild(this->PurchaseList);

	// 当前生产
	currentProductionInfo = Label::createWithSystemFont("No Production Program Now.", "Arial", 20);
	currentProductionInfo->setColor(Color3B::BLACK); // 白色文字
	currentProductionInfo->setPosition(0, 0);
	currentProductionInfo->setAnchorPoint(Vec2(0, 0));
	currentProductionInfo->setContentSize(Size(this->ProductList->getContentSize().width - 20, 50));
	auto bg = ui::Layout::create();
	bg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	bg->setBackGroundColor(Color3B(153, 204, 255)); // 浅蓝色背景
	bg->setContentSize(Size(panelSize.width, currentProductionInfo->getContentSize().height));
	currentProductionInfo->addChild(bg, -1);
	this->addChild(currentProductionInfo, 50);
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

	innerContainer->addChild(item, 50);
}

void ProductionPanel::createNewButtonItem(int playerID, BaseCity* currentCity, PanelItem::ItemType toWhichList, ProductionProgram* program)
{
	auto player = GameManager::getInstance()->getPlayer(playerID);
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto panelSize = Size(visibleSize.width / 5, visibleSize.height);
	auto newItem = ui::Button::create();
	if(toWhichList == PanelItem::ItemType::PRODUCT)
	{
		newItem->setTitleText(program->getName() +
			"     cost:" +
			std::to_string(program->getCost() / currentCity->getCityYield().productionYield + 1) +
			" turns");
	}
	else
	{
		newItem->setTitleText(program->getName() +
			"     cost:" +
			std::to_string(program->getPurchaseCost()) +
			" golds");
	}
	newItem->setTitleColor(Color3B::WHITE);
	newItem->setTitleFontSize(22);
	newItem->setContentSize(Size(this->ProductList->getContentSize().width - 20, (program->getType() == ProductionProgram::ProductionType::BUILDING) ? 120 : 50));
	newItem->addClickEventListener([=](Ref* sender) {
		newItem->addClickEventListener([=](Ref* sender) {
			// 直接使用 currentCity，不需要遍历所有城市
			if (program->getType() == ProductionProgram::ProductionType::DISTRICT)
			{
				auto newProgram = new District(currentPlayer, 
					Hex(),
					District::DistrictType::TO_BE_DEFINED, 
					program->getName());
				auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
				if (!gameScene) return;

				auto _mapLayer = gameScene->getMapLayer();
				if (!_mapLayer) return;

				// 检查是否已经在选择模式中
				bool isSelecting = _mapLayer->isSelectingTile();

				if (!isSelecting)
				{
					// 获取城市的可建造地块（排除水域、山脉等）
					auto allTerritory = currentCity->getTerritory();
					std::vector<Hex> allowedTiles;

					for (Hex& hex : allTerritory)
					{
						// 检查地块是否适合建造区域
						if (newProgram->canErectDistrict(hex))
						{
							allowedTiles.push_back(hex);
						}
					}

					if (allowedTiles.empty())
					{
						CCLOG("No valid tiles to build district");
						return;
					}

					// 启用地块选择模式
					_mapLayer->enableTileSelection(allowedTiles,
						[this, currentCity, toWhichList, program, gameScene](Hex selectedHex) {
							// 创建新的区域对象
							District* newDistrict = new District(
								currentPlayer,
								selectedHex,
								District::DistrictType::TO_BE_DEFINED,
								program->getName()
							);

							if (newDistrict)
							{
								if (toWhichList == PanelItem::ItemType::PRODUCT)
								{
									currentCity->addNewProduction(newDistrict);
								}
								else
								{
									currentCity->purchaseDirectly(newDistrict);
								}
							}
						},
						[this, gameScene]() {
							CCLOG("Building location selection cancelled");
							// 这里可以添加取消选择后的清理工作
						}
					);
				}
				else
				{
					// 如果已经在选择模式，取消它
					gameScene->cancelTileSelection(true);
				}
			}
			// 处理其他类型的建筑（不需要选择位置）
			else
			{
				// 直接添加到城市，不需要选择位置
				if (toWhichList == PanelItem::ItemType::PRODUCT)
				{
					currentCity->addNewProduction(new ProductionProgram(*program));
				}
				else
				{
					currentCity->purchaseDirectly(new ProductionProgram(*program));
				}
			}
			});
	});



	auto bg = ui::Layout::create();
	bg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	bg->setBackGroundColor(Color3B(0, 102, 204)); // 浅蓝色背景
	bg->setContentSize(Size(panelSize.width - 80, (program->getType() == ProductionProgram::ProductionType::BUILDING) ? 100 : 50));
	bg->setAnchorPoint(Vec2(0.5f, 0.5f));
	newItem->setPosition(Vec2(bg->getContentSize().width / 2, bg->getContentSize().height / 2 - (program->getType() == ProductionProgram::ProductionType::BUILDING) ? 20 : 0));
	bg->addChild(newItem, 10);

	if (program->getType() == ProductionProgram::ProductionType::BUILDING)
	{
		auto newItemTopBar = ui::Layout::create();
		auto building = new Building(currentPlayer, program->getName());
		auto newItemTopBarName = Label::createWithSystemFont(building->getDistrictName(), "Arial", 20);
		newItemTopBarName->setTextColor(Color4B::BLUE);
		newItemTopBarName->setAlignment(TextHAlignment::LEFT);
		newItemTopBarName->setContentSize(Size(panelSize.width - 80, 40));
		newItemTopBarName->setPosition(Vec2(45, newItem->getContentSize().height + bg->getContentSize().height / 2));
		newItemTopBar->setContentSize(Size(panelSize.width - 80, 40));
		newItemTopBar->setPosition(Vec2(0, 0));
		newItemTopBar->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
		newItemTopBar->setBackGroundColor(Color3B(204, 204, 0)); // 土黄色
		newItemTopBarName->addChild(newItemTopBar, -10);
		bg->addChild(newItemTopBarName, 5);
		delete building;
	}



	if (toWhichList == PanelItem::ItemType::PRODUCT)
	{
		addListItem(this->ProductList, bg);
	}
	else
	{
		addListItem(this->PurchaseList, bg);
	}
}

void ProductionPanel::createNewLabelItem(int player, BaseCity* currentCity, std::string text, PanelItem::ItemType toWhichList)
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

void ProductionPanel::updateCurrentPanel(BaseCity* currentCity)
{
	if (!currentCity->currentProduction)
		return;
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto panelSize = Size(visibleSize.width / 5, visibleSize.height);

	currentProductionInfo = Label::createWithSystemFont("Program Producing :" + 
		currentCity->currentProduction->getName() + 
		", " + std::to_string(currentCity->currentProduction->getCost() / currentCity->getCityYield().productionYield + 1) + 
		"turns left", "Arial", 20);
	currentProductionInfo->setColor(Color3B::BLACK); // 白色文字
	currentProductionInfo->setPosition(0, 0);
	currentProductionInfo->setAnchorPoint(Vec2(0, 0));
	currentProductionInfo->setContentSize(Size(this->ProductList->getContentSize().width - 20, 50));
	auto bg = ui::Layout::create();
	bg->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
	bg->setBackGroundColor(Color3B(153, 204, 255)); // 浅蓝色背景
	bg->setContentSize(Size(panelSize.width, currentProductionInfo->getContentSize().height));
	currentProductionInfo->addChild(bg, -1);
	this->addChild(currentProductionInfo, 50);
}

void ProductionPanel::clear() {
	// 遍历移除
	auto container1 = ProductList->getInnerContainer();
	container1->removeAllChildrenWithCleanup(true); // true表示同时清理内存

	// 如果使用了自定义布局，需要重置内容大小
	container1->setContentSize(ProductList->getContentSize());

	// 遍历移除
	auto container2 = PurchaseList->getInnerContainer();
	container2->removeAllChildrenWithCleanup(true); // true表示同时清理内存

	// 如果使用了自定义布局，需要重置内容大小
	container2->setContentSize(PurchaseList->getContentSize());
}
