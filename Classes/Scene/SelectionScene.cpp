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

    // 1. 渐变底色
    auto bg = LayerGradient::create(Color4B(10, 15, 30, 255), Color4B(30, 35, 50, 255), Vec2(0, -1));
    this->addChild(bg, -2);

    // 2. 延续主菜单的圆环元素 (但是要更淡、更大)
    auto bgDecor = DrawNode::create();
    bgDecor->drawCircle(Vec2(visibleSize.width * 0.8f, visibleSize.height * 0.5f),
        400, 0, 100, false, 1.0f, 1.0f, Color4F(0.75f, 0.65f, 0.35f, 0.05f));
    this->addChild(bgDecor, -1);

    // 让大圆环缓慢旋转
    bgDecor->runAction(RepeatForever::create(RotateBy::create(100.0f, 360)));
}

void CivilizationSelectionScene::createTitle() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 主标题
    auto title = Label::createWithSystemFont(u8"初始设置", "Arial", 60);
    title->setPosition(visibleSize.width / 2, visibleSize.height - 80);
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
    Color4F civGold(0.75f, 0.65f, 0.35f, 0.6f);
    Color4F panelBg(0.05f, 0.07f, 0.12f, 0.8f);

    // --- 左侧面板 ---
    auto leftPanel = Layout::create();
    leftPanel->setContentSize(Size(visibleSize.width * 0.22f, visibleSize.height * 0.55f));
    leftPanel->setPosition(Vec2(visibleSize.width * 0.04f, visibleSize.height * 0.25f)); // 稍微上移避开下划线
    this->addChild(leftPanel, 2); // 确保层级足够高

    auto leftBorder = DrawNode::create();
    leftPanel->addChild(leftBorder, -1);
    leftBorder->drawSolidRect(Vec2::ZERO, leftPanel->getContentSize(), panelBg);
    leftBorder->drawRect(Vec2::ZERO, leftPanel->getContentSize(), civGold);

    // 重新添加左侧文明选择按钮 (关键点：确保在这里循环添加)
    float buttonHeight = 55;
    float startY = leftPanel->getContentSize().height - 80;

    _civButtons.clear(); // 清空旧引用
    for (int i = 0; i < _civilizations.size(); i++) {
        const auto& civ = _civilizations[i];
        auto civButton = Button::create();
        civButton->setTitleText(civ.name);
        civButton->setTitleFontName("Georgia"); // 使用衬线体
        civButton->setTitleFontSize(22);
        civButton->setContentSize(Size(leftPanel->getContentSize().width * 0.9f, buttonHeight));
        civButton->setPosition(Vec2(leftPanel->getContentSize().width * 0.5f, startY - i * (buttonHeight + 15)));

        // 设置按钮基础颜色
        civButton->setTitleColor(Color3B(200, 200, 200));
        civButton->addClickEventListener([this, civ](Ref* sender) {
            this->onCivilizationSelected(sender, civ.type);
            });

        leftPanel->addChild(civButton);
        _civButtons.push_back(civButton);
    }

    // --- 右侧面板 ---
    _selectedPanel = Layout::create();
    _selectedPanel->setContentSize(Size(visibleSize.width * 0.68f, visibleSize.height * 0.55f));
    _selectedPanel->setPosition(Vec2(visibleSize.width * 0.28f, visibleSize.height * 0.25f));
    this->addChild(_selectedPanel, 2);

    auto rightBorder = DrawNode::create();
    _selectedPanel->addChild(rightBorder, -1);
    rightBorder->drawSolidRect(Vec2::ZERO, _selectedPanel->getContentSize(), panelBg);
    rightBorder->drawRect(Vec2::ZERO, _selectedPanel->getContentSize(), civGold);

    // 文明标题
    _selectedLabel = Label::createWithSystemFont("", "Georgia", 40);
    _selectedLabel->setPosition(_selectedPanel->getContentSize().width * 0.5f, _selectedPanel->getContentSize().height - 40);
    _selectedLabel->setColor(Color3B(255, 230, 150));
    _selectedPanel->addChild(_selectedLabel);

    // 领袖名称（副标题）
    _leaderLabel = Label::createWithSystemFont("", "Arial", 28);
    _leaderLabel->setAnchorPoint(Vec2(0.5f, 0.5f));  // 居中锚点
    _leaderLabel->setPosition(_selectedPanel->getContentSize().width * 0.5f,
        _selectedPanel->getContentSize().height * 0.78f);
    _leaderLabel->setColor(Color3B(220, 200, 150));
    _selectedPanel->addChild(_leaderLabel);

    // 左侧文字描述区域（占60%宽度）
    auto textArea = Layout::create();
    textArea->setBackGroundColorType(Layout::BackGroundColorType::NONE);
    textArea->setContentSize(Size(_selectedPanel->getContentSize().width * 0.6f,
        _selectedPanel->getContentSize().height * 0.6f));
    textArea->setPosition(Vec2(_selectedPanel->getContentSize().width * 0.05f,
        _selectedPanel->getContentSize().height * 0.15f));
    textArea->setAnchorPoint(Vec2(0, 0));
    _selectedPanel->addChild(textArea);

    // 自然语言描述标题
    auto descriptionTitle = Label::createWithSystemFont(u8"文明特性", "Arial", 24);
    descriptionTitle->setAnchorPoint(Vec2(0, 1));  // 左上角锚点
    descriptionTitle->setPosition(0, textArea->getContentSize().height - 10);
    descriptionTitle->setColor(Color3B(180, 200, 220));
    textArea->addChild(descriptionTitle);

    // 自然语言描述内容
    _descriptionLabel = Label::createWithSystemFont("", "Arial", 18);
    _descriptionLabel->setAnchorPoint(Vec2(0, 1));  // 左上角锚点
    _descriptionLabel->setPosition(0, textArea->getContentSize().height - 40);
    _descriptionLabel->setColor(Color3B(180, 200, 220));
    _descriptionLabel->setDimensions(textArea->getContentSize().width - 10,
        textArea->getContentSize().height - 50);
    _descriptionLabel->setHorizontalAlignment(TextHAlignment::LEFT);
    _descriptionLabel->setVerticalAlignment(TextVAlignment::TOP);
    textArea->addChild(_descriptionLabel);

    // 右侧领袖画像区域（占35%宽度，留5%作为边距）
    auto portraitArea = Layout::create();
    portraitArea->setBackGroundColorType(Layout::BackGroundColorType::NONE);
    portraitArea->setContentSize(Size(_selectedPanel->getContentSize().width * 0.35f,
        _selectedPanel->getContentSize().height * 0.6f));
    portraitArea->setPosition(Vec2(_selectedPanel->getContentSize().width * 0.65f,
        _selectedPanel->getContentSize().height * 0.15f));
    portraitArea->setAnchorPoint(Vec2(0, 0));
    _selectedPanel->addChild(portraitArea);

    // 领袖画像占位矩形（全身照比例，大约2:3）
    float portraitWidth = portraitArea->getContentSize().width * 0.9f;
    float portraitHeight = portraitWidth * 1.075f;  // 2:3比例

    _portraitFrame = LayerColor::create(Color4B(60, 60, 80, 255), portraitWidth, portraitHeight);
    _portraitFrame->setPosition((portraitArea->getContentSize().width - portraitWidth) * 0.5f,
        (portraitArea->getContentSize().height - portraitHeight) * 0.5f);
    portraitArea->addChild(_portraitFrame);

    // 画像占位文字
    auto portraitPlaceholder = Label::createWithSystemFont(u8"领袖画像", "Arial", 20);
    portraitPlaceholder->setPosition(portraitWidth * 0.5f, portraitHeight * 0.5f);
    portraitPlaceholder->setColor(Color3B(150, 150, 170));
    _portraitFrame->addChild(portraitPlaceholder);

    // 领袖名称在画像下方
    auto portraitNameLabel = Label::createWithSystemFont(u8"领袖", "Arial", 18);
    portraitNameLabel->setPosition(portraitWidth * 0.5f, -20);
    portraitNameLabel->setColor(Color3B(180, 180, 200));
    _portraitFrame->addChild(portraitNameLabel);

    // 选择提示
    auto selectionHint = Label::createWithSystemFont(u8"点击左侧文明按钮选择你的文明", "Arial", 18);
    selectionHint->setPosition(_selectedPanel->getContentSize().width * 0.5f, 30);
    selectionHint->setColor(Color3B(150, 150, 170));
    selectionHint->setOpacity(180);
    _selectedPanel->addChild(selectionHint);
}

void CivilizationSelectionScene::createAISettingsPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Color4F civGold(0.75f, 0.65f, 0.35f, 0.4f);

    _aiSettingsPanel = Layout::create();
    _aiSettingsPanel->setContentSize(Size(visibleSize.width * 0.92f, visibleSize.height * 0.18f));
    _aiSettingsPanel->setPosition(Vec2(visibleSize.width * 0.04f, 25));
    this->addChild(_aiSettingsPanel);

    auto aiBorder = DrawNode::create();
    _aiSettingsPanel->addChild(aiBorder, -1);
    // 仅绘制上下两条横线，营造开放式面板感
    aiBorder->drawSegment(Vec2(0, _aiSettingsPanel->getContentSize().height),
        Vec2(_aiSettingsPanel->getContentSize().width, _aiSettingsPanel->getContentSize().height), 1.0f, civGold);
    aiBorder->drawSegment(Vec2(0, 0), Vec2(_aiSettingsPanel->getContentSize().width, 0), 1.0f, civGold);

    // AI数量显示
    _aiCountLabel = Label::createWithSystemFont(u8"AI对手数量: 2", "Arial", 24);
    _aiCountLabel->setPosition(_aiSettingsPanel->getContentSize().width * 0.15f,
        _aiSettingsPanel->getContentSize().height * 0.7f);
    _aiCountLabel->setColor(Color3B(180, 180, 200));
    _aiSettingsPanel->addChild(_aiCountLabel);

    // 增加AI按钮
    auto addAIButton = Button::create();
    addAIButton->setTitleText(u8"+ 增加AI");
    addAIButton->setTitleFontSize(20);
    addAIButton->setTitleColor(Color3B::WHITE);
    addAIButton->setContentSize(Size(100, 40));
    addAIButton->setPosition(Vec2(_aiSettingsPanel->getContentSize().width * 0.25f,
        _aiSettingsPanel->getContentSize().height * 0.7f));
    addAIButton->setColor(Color3B(100, 180, 100));
    addAIButton->addClickEventListener(CC_CALLBACK_1(CivilizationSelectionScene::onAIAddClicked, this));
    _aiSettingsPanel->addChild(addAIButton);

    // 减少AI按钮
    auto removeAIButton = Button::create();
    removeAIButton->setTitleText(u8"- 减少AI");
    removeAIButton->setTitleFontSize(20);
    removeAIButton->setTitleColor(Color3B::WHITE);
    removeAIButton->setContentSize(Size(100, 40));
    removeAIButton->setPosition(Vec2(_aiSettingsPanel->getContentSize().width * 0.35f,
        _aiSettingsPanel->getContentSize().height * 0.7f));
    removeAIButton->setColor(Color3B(200, 100, 100));
    removeAIButton->addClickEventListener(CC_CALLBACK_1(CivilizationSelectionScene::onAIRemoveClicked, this));
    _aiSettingsPanel->addChild(removeAIButton);

    // AI对手列表标题
    auto aiListTitle = Label::createWithSystemFont(u8"AI文明选择:", "Arial", 22);
    aiListTitle->setPosition(_aiSettingsPanel->getContentSize().width * 0.5f,
        _aiSettingsPanel->getContentSize().height * 0.7f);
    aiListTitle->setColor(Color3B(180, 200, 220));
    _aiSettingsPanel->addChild(aiListTitle);

    // 更新AI设置显示
    updateAISettingsDisplay();
}

void CivilizationSelectionScene::createControlButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // --- 1. 返回按钮 ---
    auto backButton = Button::create();
    backButton->setTitleText(u8"BACK TO MENU");
    backButton->setTitleFontName("Georgia");
    backButton->setTitleFontSize(20);
    backButton->setAnchorPoint(Vec2(0, 0.5f));
    backButton->setPosition(Vec2(visibleSize.width * 0.05f, 50));
    backButton->addClickEventListener(CC_CALLBACK_1(CivilizationSelectionScene::onBackClicked, this));
    this->addChild(backButton, 10); // 提高层级

    // --- 2. 开始按钮 (BEGIN JOURNEY) ---
    auto startButton = Button::create();
    // 使用九宫格设置背景，如果没有图片，我们可以手动设置一个纯色块作为背景
    startButton->setScale9Enabled(true);
    startButton->setContentSize(Size(260, 70)); // 手动指定可点击区域大小

    startButton->setTitleText(u8"BEGIN JOURNEY");
    startButton->setTitleFontName("Georgia");
    startButton->setTitleFontSize(28);
    startButton->setTitleColor(Color3B::WHITE);

    // 修正坐标与锚点：居中对齐，距离右边 10% 屏幕宽度
    startButton->setAnchorPoint(Vec2(0.5f, 0.5f));
    startButton->setPosition(Vec2(visibleSize.width * 0.85f, 50));

    // 核心：设置点击回调
    startButton->addClickEventListener(CC_CALLBACK_1(CivilizationSelectionScene::onStartGameClicked, this));

    // 视觉装饰：不要挂在子节点，直接画在 Scene 上或给按钮加一层描边
    auto btnDecor = DrawNode::create();
    this->addChild(btnDecor, 9); // 放在按钮下面一层

    // 以按钮坐标为中心画框
    Vec2 pos = startButton->getPosition();
    btnDecor->drawSolidRect(pos - Vec2(130, 35), pos + Vec2(130, 35), Color4F(0.4f, 0.35f, 0.2f, 0.5f));
    btnDecor->drawRect(pos - Vec2(130, 35), pos + Vec2(130, 35), Color4F(0.75f, 0.65f, 0.35f, 1.0f));

    this->addChild(startButton, 10);
}

void CivilizationSelectionScene::onCivilizationSelected(Ref* sender, CivilizationType civType) {
    _currentSelection = civType;
    updateSelection(civType);

    for (int i = 0; i < _civilizations.size(); i++) {
        if (_civilizations[i].type == civType) {
            // 选中的按钮效果：金色边框，文字变亮
            _civButtons[i]->setTitleColor(Color3B(255, 230, 150));
            _civButtons[i]->setScale(1.05f);
        }
        else {
            _civButtons[i]->setTitleColor(Color3B(180, 180, 180));
            _civButtons[i]->setScale(1.0f);
        }
    }
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
    for (const auto& civ : _civilizations) {
        if (civ.type == selectedCiv) {
            _selectedLabel->setString(civ.name);
            _leaderLabel->setString(civ.leader);
            _descriptionLabel->setString(generateNaturalDescription(civ));

            // 清理旧画像
            _portraitFrame->removeAllChildren();

            std::string portraitPath = "";
            if (civ.type == CivilizationType::CHINA) portraitPath = "Images/Leaders/civChina.png";
            else if (civ.type == CivilizationType::GERMANY) portraitPath = "Images/Leaders/civGerman.png";
            else if (civ.type == CivilizationType::RUSSIA) portraitPath = "Images/Leaders/civRussia.png";

            auto leaderSprite = Sprite::create(portraitPath);
            if (leaderSprite) {
                // 确保图片居中
                leaderSprite->setPosition(_portraitFrame->getContentSize().width / 2,
                    _portraitFrame->getContentSize().height / 2);

                // 缩放逻辑
                float scale = (_portraitFrame->getContentSize().height * 0.9f) / leaderSprite->getContentSize().height;
                leaderSprite->setScale(scale);

                _portraitFrame->addChild(leaderSprite);
            }
            else {
                // 如果图片加载失败，显示占位文字
                auto failLabel = Label::createWithSystemFont("Image Missing", "Arial", 20);
                failLabel->setPosition(_portraitFrame->getContentSize().width / 2, _portraitFrame->getContentSize().height / 2);
                _portraitFrame->addChild(failLabel);
            }
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

    // 创建加载场景，传入创建游戏场景的函数
    auto sceneCreator = []() -> cocos2d::Scene* {
        return dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
        };

    auto loadingScene = LoadingScene::createScene(sceneCreator);

    // 切换到Loading场景
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, loadingScene));
}

void CivilizationSelectionScene::onBackClicked(cocos2d::Ref* sender) {
    // 返回主菜单
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, MainMenuScene::createScene()));
}

std::string CivilizationSelectionScene::generateNaturalDescription(const CivilizationInfo& civ) {
    // 根据文明信息生成自然语言描述
    std::string description;

    if (civ.type == CivilizationType::CHINA) {
        description =
            u8"华夏文明由始皇帝领导，是世界上最古老的文明之一。\n\n"
            u8"中国的尤里卡和灵感加成提升至75%，让科技和文化发展更加迅速。\n\n"
            u8"中国的特殊单位是虎蹲炮，一种强大的早期攻城武器。\n\n"
            u8"适合喜欢通过高效建造和快速科技发展来建立优势的玩家。";
    }
    else if (civ.type == CivilizationType::GERMANY) {
        description =
            u8"德意志文明由神圣罗马皇帝巴巴罗萨领导，是欧洲的工业与军事强国。\n\n"
            u8"德国的工业区建造速度比其他文明快50%，成本减半，让您可以更快建立工业基础。"
            u8"每个德国城市还可以额外建造一个区域，提供了更多的战略灵活性。\n\n"
            u8"德国的特殊建筑是汉萨商站，"
            u8"与商业区相邻时提供额外的生产力。\n\n"
            u8"适合喜欢通过强大的工业基础和高效生产来建立帝国的玩家。";
    }
    else if (civ.type == CivilizationType::RUSSIA) {
        description =
            u8"俄罗斯文明由彼得大帝领导，是一个横跨欧亚大陆的庞大帝国。\n\n"
            u8"俄罗斯的初始领土比其他文明大5个地块，让您在游戏早期就能控制更多资源。"
            u8"俄罗斯的学院和剧院广场产出提升20%，使科技和文化发展更加迅速。\n\n"
            u8"适合喜欢扩张领土并通过科技和文化双重优势取胜的玩家。";
    }

    return description;
}

void CivilizationSelectionScene::updatePortraitColor(const cocos2d::Color3B& civColor) {
    if (_portraitFrame) {
        // 使用文明的代表色，但稍微调暗一些作为边框
        Color4B frameColor(civColor.r * 0.7f, civColor.g * 0.7f, civColor.b * 0.7f, 255);
        _portraitFrame->setColor(Color3B(frameColor.r, frameColor.g, frameColor.b));

        // 或者如果希望有边框效果，可以这样做：
        // _portraitFrame->setColor(Color3B(civColor.r, civColor.g, civColor.b));
    }
}