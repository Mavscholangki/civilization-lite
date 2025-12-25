/*
* 城市基类
*/
#include "BaseCity.h"
#include "District/Building/Building.h"
#include "Core/GameManager.h"
#include "Scene/GameScene.h"
#include <cmath>
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
		neededFoodToMultiply = 15 + 8 * (population - 1) + (float)(pow(population - 1, 1.5) + 0.5f);
		currentAccumulation = 0;
	}
	else if (currentAccumulation < 0)
	{
		population--;
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
			districts.push_back(static_cast<District*>(currentProduction));
		}
		else if(currentProduction->getType() == ProductionProgram::ProductionType::BUILDING)
		{
			auto newBuilding = dynamic_cast<Building*>(currentProduction);
			for (auto district : districts)
			{
				district->addBuilding(newBuilding->getType());
			}
			delete newBuilding;
		}
		else if(currentProduction->getType() == ProductionProgram::ProductionType::UNIT)
		{
			GameManager::getInstance()->getPlayer(ownerPlayer)->addUnit(dynamic_cast<AbstractUnit*>(currentProduction));
		}
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
	newProgram->purchaseCompletion();
	int cost = newProgram->getPurchaseCost();
	Player* player = GameManager::getInstance()->getPlayer(this->ownerPlayer);
	if(cost <= player->getGold())
	{
		player->setGold(player->getGold() - newProgram->getPurchaseCost());
		if (newProgram->getType() == ProductionProgram::ProductionType::DISTRICT)
		{
			districts.push_back(static_cast<District*>(newProgram));
		}
		else if (newProgram->getType() == ProductionProgram::ProductionType::BUILDING)
		{
			auto newBuilding = static_cast<Building*>(newProgram);
			for (auto district : districts)
			{
				district->addBuilding(newBuilding->getType());
			}
			delete newBuilding;
		}
		else if (newProgram->getType() == ProductionProgram::ProductionType::UNIT)
		{
			GameManager::getInstance()->getPlayer(ownerPlayer)->addUnit(dynamic_cast<AbstractUnit*>(newProgram));
		}
	}
	else
	{
		warningFlash("CANNOT PURCHASE: GOLD SHORTAGE");
	}
}

void BaseCity::updateDistribution() // 更新分配信息
{
	int i = 0;
	for (auto& tile : populationDistribution) {
		if (unallocated == 0)
			break;
		if(populationDistribution[tile.first] == 0)
		{
			unallocated--;
			populationDistribution[tile.first] += 1;
		}
	}
	updateYield(); // 更新城市总产出
}

void BaseCity::updatePanel() // 更新生产面板信息
{
	dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene())->updateProductionPanel(ownerPlayer, this);
}

void BaseCity::onTurnEnd() {
	// 处理生产
	updateProduction();
    // 人口增长
	updatePopulation();
}