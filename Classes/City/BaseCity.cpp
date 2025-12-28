/*
* 城市基类
*/
#include "BaseCity.h"
#include "District/Building/Building.h"
#include "Core/GameManager.h"
#include "Scene/GameScene.h"
#include "Map/GameMapLayer.h"
#include "AllKindsOfUnits.h"
#include "UnitFactory.h"
#include "DistrictFactory.h"
#include <cmath>
#include <unordered_set>
#define RADIUS 50.0f // 六边形半径

USING_NS_CC;

void warningFlash(std::string)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	// 创建Label
	auto label = Label::createWithTTF("提示文字", "fonts/Marker Felt.ttf", 24);
	label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	Director::getInstance()->getRunningScene()->addChild(label, 500);

	// 设置初始透明度为0（完全透明）
	label->setOpacity(0);

	// 创建闪烁动作：淡入淡出重复
	auto fadeIn = FadeIn::create(0.2f);
	auto fadeOut = FadeOut::create(1.0f);
	auto sequence = Sequence::create(fadeIn, fadeOut, nullptr);
	auto repeatForever = RepeatForever::create(sequence);
	label->runAction(repeatForever);
}

BaseCity* BaseCity::create(int player, Hex pos, std::string name) {
    BaseCity* pRet = new BaseCity();
    if (pRet && pRet->initCity(player, pos, name)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet; return nullptr;
}

bool BaseCity::initCity(int player, Hex pos, std::string name) {
    if (!Node::init()) return false;

	this->ownerPlayer = player;
    this->gridPos = pos;
    this->cityName = name;
    this->population = 1;
	this->unallocated = population; // 初始时所有人口未分配
	this->maxHealth = 100;
	this->currentHealth = maxHealth;
	this->addedHealth = 0;
	// 初始化领土范围 (城市所在格子及其六个邻接格子)
	this->addToTerritory(Hex(pos.q, pos.r));
    this->addToTerritory(Hex(pos.q + 1, pos.r));
	this->addToTerritory(Hex(pos.q, pos.r + 1));
	this->addToTerritory(Hex(pos.q - 1, pos.r));
	this->addToTerritory(Hex(pos.q, pos.r - 1));
	this->addToTerritory(Hex(pos.q + 1, pos.r - 1));
	this->addToTerritory(Hex(pos.q - 1, pos.r + 1));
	this->nextTerritoryTile = Hex();
	this->turnsLeftToExpand = 999;
	this->expandAccumulation = 0;

	// === 修正2：先更新领土，再检查是否有可扩展的地块 ===
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (gameScene) {
		// 选择下一个可能的扩展地块
		choosePossibleExpand();
	}

	// === 修正3：只有在有扩展地块时才创建可视化 ===
	if (nextTerritoryTile != Hex())
	{
		updateTerritory();

		// 创建扩展可视化
		auto draw = DrawNode::create();
		auto layout = HexLayout(RADIUS);

		// 修正4：正确的坐标转换
		// 扩展可视化应该在地图层的坐标系统中，而不是相对于城市
		// 我们需要获取地图层来转换坐标
		if (gameScene && gameScene->getMapLayer()) {
			auto mapLayer = gameScene->getMapLayer();
			auto gameLayout = mapLayer->getLayout();

			if (gameLayout) {
				// 计算扩展地块的世界坐标
				Vec2 expandPos = gameLayout->hexToPixel(nextTerritoryTile);

				// 创建六边形
				Vec2 vertices[6];
				float size = RADIUS; // 使用布局的尺寸

				for (int i = 0; i < 6; i++) {
					float angle = 2.0f * M_PI / 6.0f * i + M_PI / 6.f;
					float x = expandPos.x + size * cos(angle);
					float y = expandPos.y + size * sin(angle);
					vertices[i] = Vec2(x, y);
				}

				// 绘制半透明的六边形
				draw->drawSolidPoly(vertices, 6, Color4F(255 / 255.f, 51 / 255.f, 255 / 255.f, 0.3f));

				// 添加回合数标签
				std::string expandText = std::to_string(turnsLeftToExpand) + " turns";
				auto leftTurnLabel = Label::createWithSystemFont(expandText, "Arial", 14);
				leftTurnLabel->setPosition(Vec2(expandPos.x, expandPos.y));
				leftTurnLabel->setTextColor(Color4B::WHITE);
				leftTurnLabel->enableOutline(Color4B::BLACK, 1);
				draw->addChild(leftTurnLabel);

				// 添加到地图层，而不是城市节点
				mapLayer->addChild(draw, 20);
				_expandVisual = draw;
			}
		}
	}
	else
	{
		// 没有扩展地块，设置为空
		_expandVisual = nullptr;
	}


	// 创建并添加市中心区
	Downtown* downtownDistrict = new Downtown(this->ownerPlayer, pos, "Downtown");
	this->addDistrict(static_cast<District*>(downtownDistrict)); // 添加市中心区

	_visual = Node::create();
    _visual->addChild(downtownDistrict->_downtownVisual);
    this->addChild(_visual);

    // 城市名字
    _nameLabel = ui::Button::create();
	_nameLabel->setTitleText(cityName);
	_nameLabel->setTitleFontSize(20);
	_nameLabel->setTitleColor(Color3B::WHITE);
    _nameLabel->setPosition(Vec2(0, 25));
	_nameLabel->addClickEventListener([=](Ref* sender) {
		// 点击城市名称时显示生产面板
		dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene())->updateProductionPanel(this->ownerPlayer, this);
		});
    
    this->addChild(_nameLabel, 100);
	updateDistribution(); // 更新分配信息
	updateYield(); // 更新城市总产出
	drawTerritory(); // 绘制城市边界

    return true;
}

void BaseCity::drawTerritory() // 绘制城市边界
{
	// 移除旧的扩展可视化
	if (this->_boundaryVisual) {
		_boundaryVisual->removeFromParent();
		_boundaryVisual = nullptr;
	}
	auto draw = DrawNode::create();
	for (auto& tile : territory) {
		// 计算每个格子的像素位置
		HexLayout layout(RADIUS);
		Vec2 center = layout.hexToPixel(tile) - layout.hexToPixel(this->gridPos); // 相对于城市中心的位置
		// 画出边界
		Vec2 vertices[6];
		for (int i = 0; i < 6; i++) {
			float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
			vertices[i] = Vec2(center.x + layout.size * cos(rad), center.y + layout.size * sin(rad)); // 六边形顶点
		}
		draw->drawSolidPoly(vertices, 6, Color4F(1.f, 0, 0, 0.3)); // 红色，半透明
	}
	_boundaryVisual = draw;
    this->addChild(_boundaryVisual, 10);
}

void BaseCity::deduceHealth(int damage)
{
	if (addedHealth > 0) // 先扣除额外健康度
	{
		int remainingDamage = damage - addedHealth;
		addedHealth -= damage;
		if (addedHealth < 0)
			addedHealth = 0;
		if (remainingDamage > 0)
		{
			currentHealth -= remainingDamage;
		}
	}
	else
	{
		currentHealth -= damage;
	}
}

void BaseCity::updateYield() // 更新城市总产出
{
	Yield totalYield = { 0, 0, 0, 0, 0 };
	for (auto district : districts) {
		totalYield = totalYield + district->getYield();
	}
	for (auto& tile : territory) {
		totalYield.foodYield += 1;
		totalYield.productionYield += 1;
		totalYield.goldYield += 1;       // 增加金币产出
		totalYield.scienceYield += 1;    // 增加科技产出
		totalYield.cultureYield += 1;    // 增加文化产出
	}
	cityYield = totalYield;
}

void BaseCity::updatePopulation()
{
	neededFoodToMultiply = 15 + 8 * (population - 1) + (float)(pow(population - 1, 1.5) + 0.5f);
	float accumulationIncrease = (cityYield.foodYield - population * 2.f) * 0.75f;
	currentAccumulation += accumulationIncrease;
	if (currentAccumulation >= neededFoodToMultiply)
	{
		population++;
		unallocated++;
		neededFoodToMultiply = 15 + 8 * (population - 1) + (float)(pow(population - 1, 1.5) + 0.5f);
		currentAccumulation = 0;
	}
	else if (currentAccumulation < 0)
	{
		population--;
		unallocated--;
		neededFoodToMultiply = 15 + 8 * (population - 1) + (float)(pow(population - 1, 1.5) + 0.5f);
		currentAccumulation += neededFoodToMultiply;
	}
	updateDistribution();
}

void BaseCity::addNewProduction(ProductionProgram* newProgram)
{
	if (this->currentProduction != nullptr)
		this->suspendedProductions.push_back(currentProduction);
	currentProduction = newProgram;
	updatePanel();
}

void BaseCity::updateProduction()
{
	if (!currentProduction)
	{
		if (suspendedProductions.empty())
			return;
		currentProduction = suspendedProductions.back();
		suspendedProductions.pop_back();
	}

	currentProduction->addProgress(this->cityYield.productionYield);
	if (currentProduction->isCompleted())
	{
		if (currentProduction->getType() == ProductionProgram::ProductionType::DISTRICT)
		{
			Node* districtVisual = nullptr;
			auto newDistrict = DistrictFactory::createDistrict(currentProduction->getName(), ownerPlayer, currentProduction->getPosOnCreated(), districtVisual);
			districts.push_back(newDistrict);
			auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
			districtVisual->setPosition(gameScene->getMapLayer()->getLayout()->hexToPixel(currentProduction->getPosOnCreated()));
			gameScene->getMapLayer()->addChild(districtVisual, 5);
			delete currentProduction;
		}
		else if(currentProduction->getType() == ProductionProgram::ProductionType::BUILDING)
		{
			for (auto district : districts)
			{
				district->addBuilding(currentProduction->getName());
			}
		}
		else if(currentProduction->getType() == ProductionProgram::ProductionType::UNIT)
		{
			AbstractUnit* newUnit = UnitFactory::createUnit(currentProduction->getName(), this->ownerPlayer, this->gridPos);
			if (newUnit)
			{
				GameManager::getInstance()->getPlayer(ownerPlayer)->addUnit(newUnit);
				GameManager::getInstance()->getPlayer(ownerPlayer)->addToMapFunc(newUnit);
			}
		}
		currentProduction = nullptr;
		if (suspendedProductions.empty())
		{
			currentProduction = nullptr;
		}
		else
		{
			currentProduction = suspendedProductions.back();
			suspendedProductions.pop_back();
		}
	}
}

void BaseCity::purchaseDirectly(ProductionProgram* newProgram)
{
	if (!newProgram->getCanPurchase())
		return;
	int cost = newProgram->getPurchaseCost();
	Player* player = GameManager::getInstance()->getPlayer(this->ownerPlayer);
	if(cost <= player->getGold())
	{
		newProgram->purchaseCompletion();
		player->setGold(player->getGold() - newProgram->getPurchaseCost());
		if (newProgram->getType() == ProductionProgram::ProductionType::DISTRICT)
		{
			Node* districtVisual = nullptr;
			auto newDistrict = DistrictFactory::createDistrict(newProgram->getName(), ownerPlayer, newProgram->getPosOnCreated(), districtVisual);
			districts.push_back(newDistrict);
			auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
			districtVisual->setPosition(gameScene->getMapLayer()->getLayout()->hexToPixel(currentProduction->getPosOnCreated()));
			gameScene->getMapLayer()->addChild(districtVisual, 5);
			delete newProgram;
		}
		else if (newProgram->getType() == ProductionProgram::ProductionType::BUILDING)
		{
			auto newBuilding = static_cast<Building*>(newProgram);
			for (auto district : districts)
			{
				district->addBuilding(newBuilding->getName());
			}
			delete newProgram;
		}
		else if (newProgram->getType() == ProductionProgram::ProductionType::UNIT)
		{
			AbstractUnit* newUnit = UnitFactory::createUnit(newProgram->getName(), this->ownerPlayer, this->gridPos);
			if (newUnit)
			{
				GameManager::getInstance()->getPlayer(ownerPlayer)->addUnit(newUnit);
				GameManager::getInstance()->getPlayer(ownerPlayer)->addToMapFunc(newUnit);
			}
			delete newProgram;
		}
		newProgram = nullptr;
	}
	else
	{
		warningFlash("CANNOT PURCHASE: GOLD SHORTAGE");
	}
}

void BaseCity::updateDistribution() // 更新分配信息
{
	if (unallocated < 0)
	{
		for (auto& tile : populationDistribution)
		{
			populationDistribution[tile.first] = 0;
		}
		unallocated = population;
	}
	for (auto& tile : populationDistribution) {
		if (unallocated == 0)
			break;
		if(populationDistribution[tile.first] == 0)
		{
			unallocated--;
			populationDistribution[tile.first] = 1;
		}
	}
	updateYield(); // 更新城市总产出
}

void BaseCity::updatePanel() // 更新生产面板信息
{
	dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene())->updateProductionPanel(ownerPlayer, this);
}

void BaseCity::choosePossibleExpand()
{
	// 最多36个
	if (territory.size() >= 36)
	{
		nextTerritoryTile = Hex();
		return;
	}

	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return;

	// 使用集合避免重复
	std::unordered_set<Hex> possibleExpandSet;

	// 从当前领土边界寻找可扩展地块
	for (const auto& tile : territory)
	{
		// 只考虑距离城市中心不超过5的地块
		if (tile.distance(gridPos) >= 5)
			continue;

		// 检查六个邻接方向（轴向坐标系）
		const std::vector<Hex> directions = {
			Hex(1, 0),   // 东
			Hex(1, -1),   // 东南
			Hex(0, -1),   // 西南
			Hex(-1, 0),   // 西
			Hex(-1, 1),   // 西北
			Hex(0, 1)    // 东北
		};

		for (const auto& dir : directions)
		{
			Hex neighbor = tile + dir;

			// 检查是否已经在领土中
			bool alreadyInTerritory = false;
			for (const auto& t : territory) {
				if (t == neighbor) {
					alreadyInTerritory = true;
					break;
				}
			}
			if (alreadyInTerritory) continue;

			// 检查是否被占用
			if (!gameScene->isTileOccupied(neighbor))
			{
				possibleExpandSet.insert(neighbor);
			}
		}
	}

	// 转换为向量
	std::vector<Hex> possibleExpand(possibleExpandSet.begin(), possibleExpandSet.end());

	if (possibleExpand.empty())
	{
		nextTerritoryTile = Hex();
		return;
	}

	// 寻找最优地块
	Hex optimalExpand = possibleExpand[0];
	int optimalYield = calculateTileYield(gameScene->getTileData(optimalExpand));

	for (const auto& possibleTile : possibleExpand)
	{
		int thisYield = calculateTileYield(gameScene->getTileData(possibleTile));
		if (thisYield > optimalYield)
		{
			optimalExpand = possibleTile;
			optimalYield = thisYield;
		}
		else if (thisYield == optimalYield)
		{
			// 如果产出相同，选择距离城市更近的
			if (possibleTile.distance(gridPos) < optimalExpand.distance(gridPos))
			{
				optimalExpand = possibleTile;
				optimalYield = thisYield;
			}
		}
	}

	this->nextTerritoryTile = optimalExpand;
}

// 辅助函数：计算地块总产出
int BaseCity::calculateTileYield(const TileData& data)
{
	return data.food + data.gold + data.culture + data.science + data.production;
}

void BaseCity::updateTerritory()
{
	if (nextTerritoryTile == Hex())
	{
		choosePossibleExpand();
		if (nextTerritoryTile == Hex())
		{
			// 没有可扩展的地块，清除可视化
			if (_expandVisual) {
				_expandVisual->removeFromParent();
				_expandVisual = nullptr;
			}
			return;
		}
	}

	int neededAccumulation = (20 + 10 * (territory.size() - 7));

	// 防止除零
	if (cityYield.cultureYield <= 0) {
		turnsLeftToExpand = 999; // 文化为0，几乎不可能扩展
		return;
	}

	expandAccumulation += cityYield.cultureYield;
	turnsLeftToExpand = (neededAccumulation - expandAccumulation + cityYield.cultureYield - 1) / cityYield.cultureYield;

	if (expandAccumulation >= neededAccumulation)
	{
		// 添加新领土
		territory.push_back(nextTerritoryTile);

		// 重置
		nextTerritoryTile = Hex();
		expandAccumulation = 0;

		// 立即开始选择下一个扩展地块
		choosePossibleExpand();

		// 移除旧的扩展可视化
		if (_expandVisual) {
			_expandVisual->removeFromParent();
			_expandVisual = nullptr;
		}

		// 如果有新的扩展地块，创建可视化
		if (nextTerritoryTile != Hex())
		{
			turnsLeftToExpand = (neededAccumulation - expandAccumulation + cityYield.cultureYield - 1) / cityYield.cultureYield;
			updateExpandVisualization();
		}
	}
	else
	{
		// 更新扩展可视化
		updateExpandVisualization();
	}
	drawTerritory();
}

void BaseCity::updateExpandVisualization()
{
	if (nextTerritoryTile == Hex()) return;

	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene) return;

	auto mapLayer = gameScene->getMapLayer();
	if (!mapLayer) return;

	// 移除旧的扩展可视化
	if (_expandVisual) {
		_expandVisual->removeFromParent();
		_expandVisual = nullptr;
	}

	// 创建新的扩展可视化
	auto draw = DrawNode::create();

	// 获取布局
	auto layout = mapLayer->getLayout();
	if (!layout) return;

	// 计算世界坐标
	Vec2 center = layout->hexToPixel(nextTerritoryTile);

	// 创建六边形
	std::vector<Vec2> vertices;
	float size = layout->size; // 假设有getSize()方法

	for (int i = 0; i < 6; i++) {
		float angle = 2.0f * M_PI / 6.0f * i + M_PI / 6.f;
		float x = center.x + size * cos(angle);
		float y = center.y + size * sin(angle);
		vertices.push_back(Vec2(x, y));
	}

	// 绘制半透明的六边形
	Color4F expandColor(255 / 255.f, 51 / 255.f, 255 / 255.f, 0.3f);
	draw->drawSolidPoly(&vertices[0], 6, expandColor);

	// 绘制边框
	draw->drawPoly(&vertices[0], 6, true, Color4F(1.0f, 0.2f, 1.0f, 0.8f));

	// 添加回合数标签
	std::string expandText = std::to_string(turnsLeftToExpand) + " turns";
	auto leftTurnLabel = Label::createWithSystemFont(expandText, "Arial", 14);
	leftTurnLabel->setPosition(center);
	leftTurnLabel->setTextColor(Color4B::WHITE);
	leftTurnLabel->enableOutline(Color4B::BLACK, 1);
	draw->addChild(leftTurnLabel);

	// 添加到地图层
	mapLayer->addChild(draw, 20); // 较高的z-order确保在最上层
	_expandVisual = draw;
}

void BaseCity::onTurnEnd() {
	// 处理生产
	updateProduction();
    // 人口增长
	updatePopulation();
	// 人口分配
	updateDistribution();
	// 领土扩张
	updateTerritory();
	// 面板更新
	updatePanel();
}