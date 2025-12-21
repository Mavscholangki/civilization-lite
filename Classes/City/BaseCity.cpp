/*
* 城市基类
*/
#include "BaseCity.h"

USING_NS_CC;

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
	Downtown* downtownDistrict = new Downtown(pos, name + " Downtown");
	this->addDistrict(static_cast<District*>(downtownDistrict)); // 添加市中心区

    _visual->addChild(downtownDistrict->_downtownVisual);
    this->addChild(_visual);

	// 创建生产面板 
	this->productionPanelLayer = CityProductionPanel::create();
	this->productionPanelLayer->setVisible(false); // 初始隐藏

    // 城市名字
    _nameLabel = ui::Button::create();
	_nameLabel->setTitleText(cityName);
	_nameLabel->setTitleFontSize(20);
	_nameLabel->setTitleColor(Color3B::WHITE);
    _nameLabel->setPosition(Vec2(0, 25));
	_nameLabel->addClickEventListener([=](Ref* sender) {
		// 点击城市名称时显示生产面板
		if (productionPanelLayer->isVisible()) {
			productionPanelLayer->setVisible(false);
		}
		else {
			productionPanelLayer->setVisible(true);
		}
		});
    
    this->addChild(_nameLabel);
	updateDistribution(); // 更新分配信息
	updateYield(); // 更新城市总产出
	drawBoundary(); // 绘制城市边界
	updatePanel();

	Director::getInstance()->getRunningScene()->addChild(productionPanelLayer, 100);
    return true;
}

void BaseCity::drawBoundary() // 绘制城市边界
{
	auto draw = DrawNode::create();
	for (auto tile : territory) {
		// 计算每个格子的像素位置
		HexLayout layout(40.0f); // 假设六边形大小为40
		Vec2 center = layout.hexToPixel(tile) - layout.hexToPixel(this->gridPos); // 相对于城市中心的位置
		// 画出边界
		Vec2 vertices[6];
		for (int i = 0; i < 6; i++) {
			float rad = CC_DEGREES_TO_RADIANS(60 * i - 30);
			vertices[i] = Vec2(center.x + layout.size * cos(rad), center.y + layout.size * sin(rad)); // 六边形顶点
		}
		draw->drawPoly(vertices, 6, false, Color4F::RED); // 红色边界
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
		// 这里可以加入地块的基础产出计算逻辑
		// 假设每个地块提供1食物和1生产力作为示例
		totalYield.foodYield += 1;
		totalYield.productionYield += 1;
	}
	cityYield = totalYield;
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
	// if (!this->productionPanelLayer) return; // 没有面板则不更新
	this->productionPanelLayer->populationPanel->updatePanel(this->populationDistribution, this->population);
}

void BaseCity::onTurnEnd() {
    // 这里可以处理人口增长逻辑
    CCLOG("City [%s] produced %d Gold, %d Science", cityName.c_str(), cityYield.goldYield, cityYield.scienceYield);
}