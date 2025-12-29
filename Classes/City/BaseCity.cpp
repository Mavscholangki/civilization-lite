 // BaseCity.cpp
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

/**
 * 显示警告闪烁文字
 */
void warningFlash(std::string)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	// 创建提示标签
	auto label = Label::createWithTTF("提示文字", "fonts/Marker Felt.ttf", 24);
	label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	Director::getInstance()->getRunningScene()->addChild(label, 500);

	label->setOpacity(0); // 初始透明

	// 创建闪烁动画序列
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
	delete pRet;
	return nullptr;
}

bool BaseCity::initCity(int player, Hex pos, std::string name) {
	if (!Node::init())
		return false;

	// 初始化基础属性
	this->ownerPlayer = player;
	this->gridPos = pos;
	this->cityName = name;
	this->population = 1;
	this->unallocated = population; // 初始时所有人口未分配
	this->maxHealth = 100;
	this->currentHealth = maxHealth;
	this->addedHealth = 0;

	// 初始化领土范围：城市所在格子及其六个邻接格子
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

	// 更新领土后立即选择可能的扩展地块
	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (gameScene) {
		choosePossibleExpand();
	}

	// 仅在存在扩展地块时创建可视化
	if (nextTerritoryTile != Hex()) {
		updateTerritory();

		// 创建扩展地块可视化
		auto draw = DrawNode::create();
		auto layout = HexLayout(RADIUS);

		if (gameScene && gameScene->getMapLayer()) {
			auto mapLayer = gameScene->getMapLayer();
			auto gameLayout = mapLayer->getLayout();

			if (gameLayout) {
				// 计算扩展地块的世界坐标
				Vec2 expandPos = gameLayout->hexToPixel(nextTerritoryTile);

				// 创建六边形顶点
				Vec2 vertices[6];
				float size = RADIUS;
				for (int i = 0; i < 6; i++) {
					float angle = 2.0f * M_PI / 6.0f * i + M_PI / 6.f;
					float x = expandPos.x + size * cos(angle);
					float y = expandPos.y + size * sin(angle);
					vertices[i] = Vec2(x, y);
				}

				// 绘制半透明六边形
				draw->drawSolidPoly(vertices, 6, Color4F(255 / 255.f, 51 / 255.f, 255 / 255.f, 0.3f));

				// 添加扩展倒计时标签
				std::string expandText = std::to_string(turnsLeftToExpand) + " turns";
				auto leftTurnLabel = Label::createWithSystemFont(expandText, "Arial", 14);
				leftTurnLabel->setPosition(Vec2(expandPos.x, expandPos.y));
				leftTurnLabel->setTextColor(Color4B::WHITE);
				leftTurnLabel->enableOutline(Color4B::BLACK, 1);
				draw->addChild(leftTurnLabel);

				// 添加到地图层
				mapLayer->addChild(draw, 20);
				_expandVisual = draw;
			}
		}
	}
	else {
		_expandVisual = nullptr;
	}

	// 创建市中心区
	Downtown* downtownDistrict = new Downtown(this->ownerPlayer, pos, "Downtown");
	this->addDistrict(static_cast<District*>(downtownDistrict));

	_visual = Node::create();
	_visual->addChild(downtownDistrict->_downtownVisual);
	this->addChild(_visual);

	// 城市名称标签（可点击）
	_nameLabel = ui::Button::create();
	_nameLabel->setTitleText(cityName);
	_nameLabel->setTitleFontSize(20);
	_nameLabel->setTitleColor(Color3B::WHITE);
	_nameLabel->setPosition(Vec2(0, 25));
	_nameLabel->addClickEventListener([=](Ref* sender) {
		// 点击显示生产面板
		dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene())->updateProductionPanel(this->ownerPlayer, this);
		});

	this->addChild(_nameLabel, 100);
	updateDistribution();
	updateYield();
	drawTerritory();

	return true;
}

/**
 * 绘制城市领土边界
 */
void BaseCity::drawTerritory() {
	// 清理旧的可视化
	if (this->_boundaryVisual) {
		_boundaryVisual->removeFromParent();
		_boundaryVisual = nullptr;
	}

	auto draw = DrawNode::create();
	for (auto& tile : territory) {
		// 计算每个格子的相对位置
		HexLayout layout(RADIUS);
		Vec2 center = layout.hexToPixel(tile) - layout.hexToPixel(this->gridPos);

		// 绘制六边形
		Vec2 vertices[6];
		for (int i = 0; i < 6; i++) {
			float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
			vertices[i] = Vec2(center.x + layout.size * cos(rad), center.y + layout.size * sin(rad));
		}
		draw->drawSolidPoly(vertices, 6, Color4F(1.f, 0, 0, 0.3));
	}
	_boundaryVisual = draw;
	this->addChild(_boundaryVisual, 10);
}

/**
 * 扣除城市健康度（优先扣除额外健康度）
 */
void BaseCity::deduceHealth(int damage) {
	if (addedHealth > 0) {
		int remainingDamage = damage - addedHealth;
		addedHealth -= damage;
		if (addedHealth < 0)
			addedHealth = 0;
		if (remainingDamage > 0) {
			currentHealth -= remainingDamage;
		}
	}
	else {
		currentHealth -= damage;
	}
}

/**
 * 更新城市总产出（包含领土基础产出）
 */
void BaseCity::updateYield() {
	Yield totalYield = { 0, 0, 0, 0, 0 };

	// 累加所有区域的产出
	for (auto district : districts) {
		totalYield = totalYield + district->getYield();
	}

	// 累加领土的基础产出
	for (auto& tile : territory) {
		totalYield.foodYield += 1;
		totalYield.productionYield += 1;
		totalYield.goldYield += 1;
		totalYield.scienceYield += 1;
		totalYield.cultureYield += 1;
	}

	cityYield = totalYield;
}

/**
 * 更新人口（基于食物盈余/赤字）
 */
void BaseCity::updatePopulation() {
	// 计算增长所需食物
	neededFoodToMultiply = 15 + 8 * (population - 1) + (float)(pow(population - 1, 1.5) + 0.5f);

	// 计算食物积累
	float accumulationIncrease = (cityYield.foodYield - population * 2.f) * 0.75f;
	currentAccumulation += accumulationIncrease;

	// 检查是否达到增长阈值
	if (currentAccumulation >= neededFoodToMultiply) {
		population++;
		unallocated++;
		neededFoodToMultiply = 15 + 8 * (population - 1) + (float)(pow(population - 1, 1.5) + 0.5f);
		currentAccumulation = 0;
	}
	else if (currentAccumulation < 0) {
		// 食物赤字导致人口减少
		population--;
		unallocated--;
		neededFoodToMultiply = 15 + 8 * (population - 1) + (float)(pow(population - 1, 1.5) + 0.5f);
		currentAccumulation += neededFoodToMultiply;
	}

	updateDistribution();
}

/**
 * 添加新的生产项目（如果当前有项目则挂起）
 */
void BaseCity::addNewProduction(ProductionProgram* newProgram) {
	if (this->currentProduction != nullptr)
		this->suspendedProductions.push_back(currentProduction);

	currentProduction = newProgram;
	updatePanel();
}

/**
 * 更新生产进度
 */
void BaseCity::updateProduction() {
	if (!currentProduction) {
		if (suspendedProductions.empty())
			return;
		currentProduction = suspendedProductions.back();
		suspendedProductions.pop_back();
	}

	// 增加生产进度
	currentProduction->addProgress(this->cityYield.productionYield);

	// 检查是否完成
	if (currentProduction->isCompleted()) {
		if (currentProduction->getType() == ProductionProgram::ProductionType::DISTRICT) {
			// 创建新区
			Node* districtVisual = nullptr;
			auto newDistrict = DistrictFactory::createDistrict(currentProduction->getName(), ownerPlayer, currentProduction->getPosOnCreated(), districtVisual);
			districts.push_back(newDistrict);

			// 添加到地图
			auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
			districtVisual->setPosition(gameScene->getMapLayer()->getLayout()->hexToPixel(currentProduction->getPosOnCreated()));
			gameScene->getMapLayer()->addChild(districtVisual, 5);

			delete currentProduction;
		}
		else if (currentProduction->getType() == ProductionProgram::ProductionType::BUILDING) {
			// 为所有区域添加建筑
			for (auto district : districts) {
				district->addBuilding(currentProduction->getName());
			}
		}
		else if (currentProduction->getType() == ProductionProgram::ProductionType::UNIT) {
			// 创建单位
			AbstractUnit* newUnit = UnitFactory::createUnit(currentProduction->getName(), this->ownerPlayer, this->gridPos);
			if (newUnit) {
				GameManager::getInstance()->getPlayer(ownerPlayer)->addUnit(newUnit);
				GameManager::getInstance()->getPlayer(ownerPlayer)->addToMapFunc(newUnit);
			}
		}

		currentProduction = nullptr;

		// 检查是否有挂起的项目
		if (suspendedProductions.empty()) {
			currentProduction = nullptr;
		}
		else {
			currentProduction = suspendedProductions.back();
			suspendedProductions.pop_back();
		}
	}
}

/**
 * 直接购买项目（使用金币）
 */
void BaseCity::purchaseDirectly(ProductionProgram* newProgram) {
	if (!newProgram->getCanPurchase())
		return;

	int cost = newProgram->getPurchaseCost();
	Player* player = GameManager::getInstance()->getPlayer(this->ownerPlayer);

	if (cost <= player->getGold()) {
		newProgram->purchaseCompletion();
		player->setGold(player->getGold() - newProgram->getPurchaseCost());

		if (newProgram->getType() == ProductionProgram::ProductionType::DISTRICT) {
			Node* districtVisual = nullptr;
			auto newDistrict = DistrictFactory::createDistrict(newProgram->getName(), ownerPlayer, newProgram->getPosOnCreated(), districtVisual);
			districts.push_back(newDistrict);

			auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
			districtVisual->setPosition(gameScene->getMapLayer()->getLayout()->hexToPixel(currentProduction->getPosOnCreated()));
			gameScene->getMapLayer()->addChild(districtVisual, 5);

			delete newProgram;
		}
		else if (newProgram->getType() == ProductionProgram::ProductionType::BUILDING) {
			auto newBuilding = static_cast<Building*>(newProgram);
			for (auto district : districts) {
				district->addBuilding(newBuilding->getName());
			}
			delete newProgram;
		}
		else if (newProgram->getType() == ProductionProgram::ProductionType::UNIT) {
			AbstractUnit* newUnit = UnitFactory::createUnit(newProgram->getName(), this->ownerPlayer, this->gridPos);
			if (newUnit) {
				GameManager::getInstance()->getPlayer(ownerPlayer)->addUnit(newUnit);
				GameManager::getInstance()->getPlayer(ownerPlayer)->addToMapFunc(newUnit);
			}
			delete newProgram;
		}
		newProgram = nullptr;
	}
	else {
		warningFlash("CANNOT PURCHASE: GOLD SHORTAGE");
	}
}

/**
 * 更新人口分配（确保所有人口都有对应地块）
 */
void BaseCity::updateDistribution() {
	// 处理异常情况：未分配人口为负数
	if (unallocated < 0) {
		for (auto& tile : populationDistribution) {
			populationDistribution[tile.first] = 0;
		}
		unallocated = population;
	}

	// 为未分配人口分配地块
	for (auto& tile : populationDistribution) {
		if (unallocated == 0)
			break;
		if (populationDistribution[tile.first] == 0) {
			unallocated--;
			populationDistribution[tile.first] = 1;
		}
	}

	updateYield(); // 分配变化影响产出
}

/**
 * 更新生产面板
 */
void BaseCity::updatePanel() {
	dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene())->updateProductionPanel(ownerPlayer, this);
}

/**
 * 选择可能的扩展地块（基于领土边界和地块质量）
 */
void BaseCity::choosePossibleExpand() {
	// 检查领土数量上限
	if (territory.size() >= 36) {
		nextTerritoryTile = Hex();
		return;
	}

	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene)
		return;

	std::unordered_set<Hex> possibleExpandSet;

	// 遍历当前领土的每个地块
	for (const auto& tile : territory) {
		// 只考虑距离城市中心不超过5的地块
		if (tile.distance(gridPos) >= 5)
			continue;

		// 六个扩展方向
		const std::vector<Hex> directions = {
			Hex(1, 0),   // 东
			Hex(1, -1),  // 东南
			Hex(0, -1),  // 西南
			Hex(-1, 0),  // 西
			Hex(-1, 1),  // 西北
			Hex(0, 1)    // 东北
		};

		for (const auto& dir : directions) {
			Hex neighbor = tile + dir;

			// 检查是否已在领土中
			bool alreadyInTerritory = false;
			for (const auto& t : territory) {
				if (t == neighbor) {
					alreadyInTerritory = true;
					break;
				}
			}
			if (alreadyInTerritory)
				continue;

			// 检查地块是否可用
			if (!gameScene->isTileOccupied(neighbor)) {
				possibleExpandSet.insert(neighbor);
			}
		}
	}

	// 转换为向量便于处理
	std::vector<Hex> possibleExpand(possibleExpandSet.begin(), possibleExpandSet.end());

	if (possibleExpand.empty()) {
		nextTerritoryTile = Hex();
		return;
	}

	// 寻找最优地块（基于产出和距离）
	Hex optimalExpand = possibleExpand[0];
	int optimalYield = calculateTileYield(gameScene->getTileData(optimalExpand));

	for (const auto& possibleTile : possibleExpand) {
		int thisYield = calculateTileYield(gameScene->getTileData(possibleTile));
		if (thisYield > optimalYield) {
			optimalExpand = possibleTile;
			optimalYield = thisYield;
		}
		else if (thisYield == optimalYield) {
			// 产出相同则选择距离更近的
			if (possibleTile.distance(gridPos) < optimalExpand.distance(gridPos)) {
				optimalExpand = possibleTile;
				optimalYield = thisYield;
			}
		}
	}

	this->nextTerritoryTile = optimalExpand;
}

/**
 * 计算地块总产出（辅助函数）
 */
int BaseCity::calculateTileYield(const TileData& data) {
	return data.food + data.gold + data.culture + data.science + data.production;
}

/**
 * 更新领土扩展逻辑
 */
void BaseCity::updateTerritory() {
	// 如果没有目标扩展地块，选择一个新的
	if (nextTerritoryTile == Hex()) {
		choosePossibleExpand();
		if (nextTerritoryTile == Hex()) {
			// 无可用扩展地块，清理可视化
			if (_expandVisual) {
				_expandVisual->removeFromParent();
				_expandVisual = nullptr;
			}
			return;
		}
	}

	// 计算扩展所需的文化积累
	int neededAccumulation = (20 + 10 * (territory.size() - 7));

	if (cityYield.cultureYield <= 0) {
		turnsLeftToExpand = 999; // 文化产出为0时几乎无法扩展
		return;
	}

	// 积累文化值
	expandAccumulation += cityYield.cultureYield;
	turnsLeftToExpand = (neededAccumulation - expandAccumulation + cityYield.cultureYield - 1) / cityYield.cultureYield;

	// 检查是否达到扩展条件
	if (expandAccumulation >= neededAccumulation) {
		// 添加新领土
		territory.push_back(nextTerritoryTile);

		// 重置扩展状态
		nextTerritoryTile = Hex();
		expandAccumulation = 0;

		// 选择下一个扩展地块
		choosePossibleExpand();

		// 清理旧的可视化
		if (_expandVisual) {
			_expandVisual->removeFromParent();
			_expandVisual = nullptr;
		}

		// 创建新的扩展可视化（如果存在）
		if (nextTerritoryTile != Hex()) {
			turnsLeftToExpand = (neededAccumulation - expandAccumulation + cityYield.cultureYield - 1) / cityYield.cultureYield;
			updateExpandVisualization();
		}
	}
	else {
		// 更新现有扩展可视化
		updateExpandVisualization();
	}

	drawTerritory();
}

/**
 * 更新扩展地块的可视化
 */
void BaseCity::updateExpandVisualization() {
	if (nextTerritoryTile == Hex())
		return;

	auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
	if (!gameScene)
		return;

	auto mapLayer = gameScene->getMapLayer();
	if (!mapLayer)
		return;

	// 清理旧的可视化
	if (_expandVisual) {
		_expandVisual->removeFromParent();
		_expandVisual = nullptr;
	}

	// 创建新的可视化节点
	auto draw = DrawNode::create();
	auto layout = mapLayer->getLayout();
	if (!layout)
		return;

	// 计算中心位置
	Vec2 center = layout->hexToPixel(nextTerritoryTile);

	// 生成六边形顶点
	std::vector<Vec2> vertices;
	float size = layout->size;
	for (int i = 0; i < 6; i++) {
		float angle = 2.0f * M_PI / 6.0f * i + M_PI / 6.f;
		float x = center.x + size * cos(angle);
		float y = center.y + size * sin(angle);
		vertices.push_back(Vec2(x, y));
	}

	// 绘制半透明填充和边框
	Color4F expandColor(255 / 255.f, 51 / 255.f, 255 / 255.f, 0.3f);
	draw->drawSolidPoly(&vertices[0], 6, expandColor);
	draw->drawPoly(&vertices[0], 6, true, Color4F(1.0f, 0.2f, 1.0f, 0.8f));

	// 添加扩展倒计时标签
	std::string expandText = std::to_string(turnsLeftToExpand) + " turns";
	auto leftTurnLabel = Label::createWithSystemFont(expandText, "Arial", 14);
	leftTurnLabel->setPosition(center);
	leftTurnLabel->setTextColor(Color4B::WHITE);
	leftTurnLabel->enableOutline(Color4B::BLACK, 1);
	draw->addChild(leftTurnLabel);

	// 添加到地图层（高z-order确保可见）
	mapLayer->addChild(draw, 20);
	_expandVisual = draw;
}

/**
 * 回合结束处理（城市所有更新逻辑）
 */
void BaseCity::onTurnEnd() {
	updateProduction();    // 生产进度
	updatePopulation();    // 人口增长
	updateDistribution();  // 人口分配
	updateTerritory();     // 领土扩展
	updatePanel();         // 界面更新
}