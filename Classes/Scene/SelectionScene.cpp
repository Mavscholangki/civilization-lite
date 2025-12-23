#include "SelectionScene.h"
#include "LoadingScene.h"
#include "MainMenuScene.h"
#include "../Core/Player.h"
#include "../Core/GameManager.h"

USING_NS_CC;
using namespace ui;

// 初始化静态变量
CivilizationType CivilizationSelectionScene::s_selectedCivilization = CivilizationType::CHINA;
std::vector<AIPlayerSetting> CivilizationSelectionScene::s_aiPlayerSettings;
int CivilizationSelectionScene::s_aiPlayerCount = 2; // 默认2个AI

Scene* CivilizationSelectionScene::createScene() {
    return CivilizationSelectionScene::create();
}

bool CivilizationSelectionScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // 1. 创建背景
    createBackground();

    // 2. 创建标题
    createTitle();

    // 3. 初始化文明信息
    _civilizations.clear();
    _civilizations.push_back(CivilizationInfo(
        u8"华夏文明", u8"始皇帝",
        u8"古老的东方文明，拥有悠久的历史和文化传承",
        u8"建造者使用次数+2，尤里卡和灵感提供75%加成",
        u8"虎蹲炮", u8"",
        CivilizationType::CHINA, Color3B(200, 50, 50) // 红色
    ));

    _civilizations.push_back(CivilizationInfo(
        u8"德意志", u8"神圣罗马皇帝",
        u8"工业与军事强国，以效率和纪律著称",
        u8"工业区建造速度+50%，每个城市可多建一个区域",
        "", u8"汉萨商站",
        CivilizationType::GERMANY, Color3B(100, 100, 200) // 蓝色
    ));

    _civilizations.push_back(CivilizationInfo(
        u8"俄罗斯", u8"彼得大帝",
        u8"横跨欧亚的庞大帝国，拥有广袤的领土",
        u8"初始领土+5，学院和剧院广场产出+20%",
        u8"", u8"",
        CivilizationType::RUSSIA, Color3B(50, 150, 50) // 绿色
    ));

    // 4. 初始化AI设置
    _aiPlayerCount = 2; // 默认2个AI
    _aiSettings.clear();

    // 默认AI设置（排除玩家选择的文明）
    std::vector<CivilizationType> availableCivs = getAvailableAICivilizations();
    for (int i = 0; i < _aiPlayerCount && i < availableCivs.size(); i++) {
        std::string aiName = "AI Player " + std::to_string(i + 1);
        _aiSettings.push_back(AIPlayerSetting(i + 1, availableCivs[i], aiName));
    }

    // 5. 创建玩家文明选择面板
    createPlayerCivilizationPanel();

    // 6. 创建AI设置面板
    createAISettingsPanel();

    // 7. 创建控制按钮
    createControlButtons();

    // 8. 设置默认选择
    _currentSelection = CivilizationType::CHINA;
    updateSelection(_currentSelection);

    return true;
}

void CivilizationSelectionScene::createBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 渐变背景
    auto bg = LayerGradient::create(
        Color4B(30, 30, 45, 255),  // 顶部颜色
        Color4B(15, 15, 25, 255),  // 底部颜色
        Vec2(0, -1)
    );
    bg->setContentSize(visibleSize);
    this->addChild(bg);
}

void CivilizationSelectionScene::createTitle() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 主标题
    auto title = Label::createWithSystemFont(u8"选择你的文明", "Arial", 60);
    title->setPosition(visibleSize.width / 2, visibleSize.height - 100);
    title->setColor(Color3B(220, 200, 120));
    title->enableShadow(Color4B::BLACK, Size(3, -3), 3);
    this->addChild(title, 1);

    // 副标题
    auto subtitle = Label::createWithSystemFont(u8"配置你的文明和AI对手", "Arial", 24);
    subtitle->setPosition(visibleSize.width / 2, visibleSize.height - 160);
    subtitle->setColor(Color3B(180, 180, 200));
    this->addChild(subtitle, 1);
}

void CivilizationSelectionScene::createPlayerCivilizationPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float startX = visibleSize.width * 0.15f;
    float spacing = visibleSize.width * 0.25f;

    // 玩家文明选择标题
    auto playerTitle = Label::createWithSystemFont(u8"玩家文明选择", "Arial", 28);
    playerTitle->setPosition(visibleSize.width * 0.5f, visibleSize.height - 220);
    playerTitle->setColor(Color3B(100, 180, 255));
    this->addChild(playerTitle);

    for (int i = 0; i < _civilizations.size(); i++) {
        const auto& civ = _civilizations[i];
        float posX = startX + spacing * i;

        // 文明卡片背景
        auto cardBg = LayerColor::create(Color4B(50, 50, 65, 200), 280, 350);
        cardBg->setPosition(posX - 140, visibleSize.height * 0.65f - 175);
        cardBg->setOpacity(200);
        cardBg->setTag(1000 + i);
        this->addChild(cardBg);

        // 文明名称
        auto nameLabel = Label::createWithSystemFont(civ.name, "Arial", 28);
        nameLabel->setPosition(140, 310);
        nameLabel->setColor(civ.color);
        nameLabel->enableShadow(Color4B::BLACK, Size(2, -2), 2);
        cardBg->addChild(nameLabel);

        // 领袖名称
        auto leaderLabel = Label::createWithSystemFont(civ.leader, "Arial", 22);
        leaderLabel->setPosition(140, 280);
        leaderLabel->setColor(Color3B(200, 200, 150));
        cardBg->addChild(leaderLabel);

        // 文明描述
        auto descLabel = Label::createWithSystemFont(civ.description, "Arial", 16);
        descLabel->setPosition(140, 240);
        descLabel->setColor(Color3B(180, 180, 200));
        descLabel->setDimensions(260, 50);
        descLabel->setHorizontalAlignment(TextHAlignment::CENTER);
        cardBg->addChild(descLabel);

        // 特殊能力
        auto abilityLabel = Label::createWithSystemFont(u8"能力: " + civ.ability, "Arial", 15);
        abilityLabel->setPosition(140, 190);
        abilityLabel->setColor(Color3B(160, 200, 255));
        abilityLabel->setDimensions(260, 40);
        abilityLabel->setHorizontalAlignment(TextHAlignment::CENTER);
        cardBg->addChild(abilityLabel);

        // 特殊单位
        auto unitLabel = Label::createWithSystemFont(u8"单位: " + civ.uniqueUnit, "Arial", 16);
        unitLabel->setPosition(140, 140);
        unitLabel->setColor(Color3B(150, 220, 150));
        cardBg->addChild(unitLabel);

        // 特殊建筑
        auto buildingLabel = Label::createWithSystemFont(u8"建筑: " + civ.uniqueBuilding, "Arial", 16);
        buildingLabel->setPosition(140, 110);
        buildingLabel->setColor(Color3B(220, 180, 100));
        cardBg->addChild(buildingLabel);

        // 选择按钮
        auto selectButton = Button::create();
        selectButton->setTitleText(u8"选择");
        selectButton->setTitleFontSize(20);
        selectButton->setTitleColor(Color3B::WHITE);
        selectButton->setContentSize(Size(150, 40));
        selectButton->setPosition(Vec2(140, 60));
        selectButton->setColor(civ.color);
        selectButton->addClickEventListener([this, civ](Ref* sender) {
            this->onCivilizationSelected(sender, civ.type);
            });
        cardBg->addChild(selectButton);
    }

    // 玩家文明详情面板
    _selectedPanel = Layout::create();
    _selectedPanel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    _selectedPanel->setBackGroundColor(Color3B(40, 40, 55));
    _selectedPanel->setContentSize(Size(350, 200));
    _selectedPanel->setPosition(Vec2(visibleSize.width * 0.5f - 175, visibleSize.height * 0.35f - 100));
    _selectedPanel->setOpacity(230);
    this->addChild(_selectedPanel, 1);

    // 选中标签
    _selectedLabel = Label::createWithSystemFont("", "Arial", 30);
    _selectedLabel->setPosition(175, 160);
    _selectedLabel->setColor(Color3B::YELLOW);
    _selectedLabel->enableShadow(Color4B::BLACK, Size(2, -2), 2);
    _selectedPanel->addChild(_selectedLabel);

    _abilityLabel = Label::createWithSystemFont("", "Arial", 18);
    _abilityLabel->setPosition(175, 120);
    _abilityLabel->setColor(Color3B(180, 220, 255));
    _abilityLabel->setDimensions(340, 40);
    _abilityLabel->setHorizontalAlignment(TextHAlignment::CENTER);
    _selectedPanel->addChild(_abilityLabel);

    _unitLabel = Label::createWithSystemFont("", "Arial", 18);
    _unitLabel->setPosition(175, 80);
    _unitLabel->setColor(Color3B(180, 255, 180));
    _selectedPanel->addChild(_unitLabel);

    _buildingLabel = Label::createWithSystemFont("", "Arial", 18);
    _buildingLabel->setPosition(175, 50);
    _buildingLabel->setColor(Color3B(255, 220, 140));
    _selectedPanel->addChild(_buildingLabel);
}

void CivilizationSelectionScene::createAISettingsPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // AI设置面板背景
    _aiSettingsPanel = Layout::create();
    _aiSettingsPanel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    _aiSettingsPanel->setBackGroundColor(Color3B(45, 45, 60));
    _aiSettingsPanel->setContentSize(Size(visibleSize.width * 0.9f, 220));
    _aiSettingsPanel->setPosition(Vec2(visibleSize.width * 0.05f, 150));
    _aiSettingsPanel->setOpacity(200);
    this->addChild(_aiSettingsPanel);

    // AI设置标题
    auto aiTitle = Label::createWithSystemFont(u8"AI对手设置", "Arial", 28);
    aiTitle->setPosition(_aiSettingsPanel->getContentSize().width / 2, 190);
    aiTitle->setColor(Color3B(200, 150, 100));
    _aiSettingsPanel->addChild(aiTitle);

    // AI数量显示
    _aiCountLabel = Label::createWithSystemFont(u8"AI数量: 2", "Arial", 24);
    _aiCountLabel->setPosition(_aiSettingsPanel->getContentSize().width * 0.15f, 150);
    _aiCountLabel->setColor(Color3B(180, 180, 200));
    _aiSettingsPanel->addChild(_aiCountLabel);

    // 增加AI按钮
    auto addAIButton = Button::create();
    addAIButton->setTitleText("+");
    addAIButton->setTitleFontSize(28);
    addAIButton->setTitleColor(Color3B::WHITE);
    addAIButton->setContentSize(Size(50, 40));
    addAIButton->setPosition(Vec2(_aiSettingsPanel->getContentSize().width * 0.25f, 150));
    addAIButton->setColor(Color3B(100, 180, 100));
    addAIButton->addClickEventListener(CC_CALLBACK_1(CivilizationSelectionScene::onAIAddClicked, this));
    _aiSettingsPanel->addChild(addAIButton);

    // 减少AI按钮
    auto removeAIButton = Button::create();
    removeAIButton->setTitleText("-");
    removeAIButton->setTitleFontSize(28);
    removeAIButton->setTitleColor(Color3B::WHITE);
    removeAIButton->setContentSize(Size(50, 40));
    removeAIButton->setPosition(Vec2(_aiSettingsPanel->getContentSize().width * 0.3f, 150));
    removeAIButton->setColor(Color3B(200, 100, 100));
    removeAIButton->addClickEventListener(CC_CALLBACK_1(CivilizationSelectionScene::onAIRemoveClicked, this));
    _aiSettingsPanel->addChild(removeAIButton);

    // AI对手列表标题
    auto aiListTitle = Label::createWithSystemFont(u8"AI对手列表:", "Arial", 22);
    aiListTitle->setPosition(_aiSettingsPanel->getContentSize().width * 0.5f, 150);
    aiListTitle->setColor(Color3B(180, 200, 220));
    _aiSettingsPanel->addChild(aiListTitle);

    // 更新AI设置显示
    updateAISettingsDisplay();
}

void CivilizationSelectionScene::createControlButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 返回按钮
    auto backButton = Button::create();
    backButton->setTitleText(u8"返回主菜单");
    backButton->setTitleFontSize(24);
    backButton->setTitleColor(Color3B(200, 200, 200));
    backButton->setContentSize(Size(200, 50));
    backButton->setPosition(Vec2(visibleSize.width * 0.25f, 70));
    backButton->addClickEventListener(CC_CALLBACK_1(CivilizationSelectionScene::onBackClicked, this));
    this->addChild(backButton);

    // 开始游戏按钮
    auto startButton = Button::create();
    startButton->setTitleText(u8"开始游戏");
    startButton->setTitleFontSize(28);
    startButton->setTitleColor(Color3B::WHITE);
    startButton->setContentSize(Size(250, 60));
    startButton->setPosition(Vec2(visibleSize.width * 0.75f, 70));
    startButton->setColor(Color3B(100, 180, 100));
    startButton->addClickEventListener(CC_CALLBACK_1(CivilizationSelectionScene::onStartGameClicked, this));
    this->addChild(startButton);
}

void CivilizationSelectionScene::onCivilizationSelected(cocos2d::Ref* sender, CivilizationType civType) {
    CCLOG("Player civilization selected: %d", static_cast<int>(civType));

    _currentSelection = civType;
    updateSelection(civType);

    // 移除之前的高亮
    this->removeChildByTag(999);

    // 找到选中的文明在数组中的索引
    int selectedIndex = -1;
    for (int i = 0; i < _civilizations.size(); i++) {
        if (_civilizations[i].type == civType) {
            selectedIndex = i;
            break;
        }
    }

    if (selectedIndex >= 0) {
        const auto& civ = _civilizations[selectedIndex];
        auto visibleSize = Director::getInstance()->getVisibleSize();
        float startX = visibleSize.width * 0.15f;
        float spacing = visibleSize.width * 0.25f;

        float posX = startX + spacing * selectedIndex;
        auto highlight = LayerColor::create(Color4B(civ.color.r, civ.color.g, civ.color.b, 50), 290, 360);
        highlight->setPosition(posX - 145, visibleSize.height * 0.65f - 180);
        highlight->setTag(999);
        this->addChild(highlight, 0);
    }

    // 更新AI设置（重新分配可用文明）
    updateAISettingsDisplay();
}

void CivilizationSelectionScene::onAIButtonClicked(cocos2d::Ref* sender, int aiIndex) {
    if (aiIndex < 0 || aiIndex >= _aiSettings.size()) return;

    // 弹出文明选择菜单
    auto availableCivs = getAvailableAICivilizations();
    if (availableCivs.empty()) return;

    // 简单实现：循环切换文明
    CivilizationType currentCiv = _aiSettings[aiIndex].civilization;
    int currentIndex = -1;

    // 找到当前文明在可用列表中的位置
    for (int i = 0; i < availableCivs.size(); i++) {
        if (availableCivs[i] == currentCiv) {
            currentIndex = i;
            break;
        }
    }

    // 选择下一个文明
    int nextIndex = (currentIndex + 1) % availableCivs.size();
    _aiSettings[aiIndex].civilization = availableCivs[nextIndex];

    // 更新按钮显示
    updateAISettingsDisplay();

    CCLOG("AI %d civilization changed to: %d", aiIndex, static_cast<int>(availableCivs[nextIndex]));
}

void CivilizationSelectionScene::onAIAddClicked(cocos2d::Ref* sender) {
    if (_aiPlayerCount >= MAX_AI_PLAYERS) {
        CCLOG("Cannot add more AI players. Maximum is %d", MAX_AI_PLAYERS);
        return;
    }

    _aiPlayerCount++;

    // 为新AI分配一个文明
    auto availableCivs = getAvailableAICivilizations();
    if (!availableCivs.empty()) {
        std::string aiName = "AI Player " + std::to_string(_aiPlayerCount);
        _aiSettings.push_back(AIPlayerSetting(_aiPlayerCount, availableCivs[0], aiName));
    }
    else {
        // 如果没有可用文明，使用默认文明
        std::string aiName = "AI Player " + std::to_string(_aiPlayerCount);
        _aiSettings.push_back(AIPlayerSetting(_aiPlayerCount, CivilizationType::GERMANY, aiName));
    }

    updateAICountLabel();
    updateAISettingsDisplay();

    CCLOG("AI player added. Total AI: %d", _aiPlayerCount);
}

void CivilizationSelectionScene::onAIRemoveClicked(cocos2d::Ref* sender) {
    if (_aiPlayerCount <= MIN_AI_PLAYERS) {
        CCLOG("Cannot remove AI player. Minimum is %d", MIN_AI_PLAYERS);
        return;
    }

    _aiPlayerCount--;
    _aiSettings.pop_back();

    updateAICountLabel();
    updateAISettingsDisplay();

    CCLOG("AI player removed. Total AI: %d", _aiPlayerCount);
}

void CivilizationSelectionScene::updateSelection(CivilizationType selectedCiv) {
    // 查找选中的文明信息
    for (const auto& civ : _civilizations) {
        if (civ.type == selectedCiv) {
            _selectedLabel->setString(civ.name + " - " + civ.leader);
            _selectedLabel->setColor(civ.color);
            _abilityLabel->setString(civ.ability);
            _unitLabel->setString(u8"特殊单位: " + civ.uniqueUnit);
            _buildingLabel->setString(u8"特殊建筑: " + civ.uniqueBuilding);
            break;
        }
    }
}

void CivilizationSelectionScene::updateAISettingsDisplay() {
    // 清除现有的AI按钮
    for (auto button : _aiButtons) {
        _aiSettingsPanel->removeChild(button);
    }
    _aiButtons.clear();

    // 创建新的AI按钮
    int aiCount = _aiSettings.size();
    float buttonWidth = 200;
    float totalWidth = aiCount * buttonWidth + (aiCount - 1) * 20;
    float startX = (_aiSettingsPanel->getContentSize().width - totalWidth) / 2 + buttonWidth / 2;

    for (int i = 0; i < aiCount; i++) {
        float posX = startX + i * (buttonWidth + 20);

        // 查找文明信息
        CivilizationType civType = _aiSettings[i].civilization;
        CivilizationInfo civInfo("", "", "", "", "", "", CivilizationType::BASIC, Color3B::WHITE);

        for (const auto& civ : _civilizations) {
            if (civ.type == civType) {
                civInfo = civ;
                break;
            }
        }

        // 创建AI按钮
        auto aiButton = Button::create();
        aiButton->setTitleText(u8"AI " + std::to_string(i + 1) + ": " + civInfo.name);
        aiButton->setTitleFontSize(18);
        aiButton->setTitleColor(Color3B::WHITE);
        aiButton->setContentSize(Size(buttonWidth, 40));
        aiButton->setPosition(Vec2(posX, 80));
        aiButton->setColor(civInfo.color);
        aiButton->addClickEventListener([this, i](Ref* sender) {
            this->onAIButtonClicked(sender, i);
            });

        _aiSettingsPanel->addChild(aiButton);
        _aiButtons.push_back(aiButton);
    }
}

void CivilizationSelectionScene::updateAICountLabel() {
    _aiCountLabel->setString(u8"AI数量: " + std::to_string(_aiPlayerCount));
}

std::vector<CivilizationType> CivilizationSelectionScene::getAvailableAICivilizations() {
    std::vector<CivilizationType> available;

    //// 排除玩家选择的文明
    //for (const auto& civ : _civilizations) {
    //    if (civ.type != _currentSelection) {
    //        available.push_back(civ.type);
    //    }
    //}

    // 如果只有玩家文明，返回所有文明（包括玩家的）
    if (available.empty()) {
        for (const auto& civ : _civilizations) {
            available.push_back(civ.type);
        }
    }

    return available;
}

void CivilizationSelectionScene::onStartGameClicked(cocos2d::Ref* sender) {
    CCLOG("Starting game with civilization: %d", static_cast<int>(_currentSelection));

    // 保存玩家文明选择
    s_selectedCivilization = _currentSelection;

    // 保存AI设置
    s_aiPlayerSettings = _aiSettings;
    s_aiPlayerCount = _aiPlayerCount;

    // 跳转到加载界面
    auto sceneCreator = []() -> cocos2d::Scene* {
        return LoadingScene::createScene();
        };

    auto loadingScene = LoadingScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, loadingScene));
}

void CivilizationSelectionScene::onBackClicked(cocos2d::Ref* sender) {
    // 返回主菜单
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, MainMenuScene::createScene()));
}