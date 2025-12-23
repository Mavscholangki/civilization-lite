#ifndef __CIVILIZATION_SELECTION_SCENE_H__
#define __CIVILIZATION_SELECTION_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Civilizations/BaseCiv.h"
#include "../Civilizations/CivChina.h"
#include "../Civilizations/CivGermany.h"
#include "../Civilizations/CivRussia.h"
#include "../Core/Player.h"
#include <vector>

// 文明信息结构
struct CivilizationInfo {
    std::string name;
    std::string leader;
    std::string description;
    std::string ability;
    std::string uniqueUnit;
    std::string uniqueBuilding;
    CivilizationType type;
    cocos2d::Color3B color;

    CivilizationInfo(const std::string& n, const std::string& l, const std::string& d,
        const std::string& a, const std::string& uu, const std::string& ub,
        CivilizationType t, const cocos2d::Color3B& c)
        : name(n), leader(l), description(d), ability(a),
        uniqueUnit(uu), uniqueBuilding(ub), type(t), color(c) {
    }
};

// AI玩家设置
struct AIPlayerSetting {
    int playerId;
    CivilizationType civilization;
    std::string name;

    AIPlayerSetting(int id, CivilizationType civ, const std::string& n)
        : playerId(id), civilization(civ), name(n) {
    }
};

class CivilizationSelectionScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(CivilizationSelectionScene);

    // 获取选中的文明类型
    static CivilizationType getSelectedCivilization() { return s_selectedCivilization; }
    static void setSelectedCivilization(CivilizationType civ) { s_selectedCivilization = civ; }

    // 获取AI玩家设置
    static std::vector<AIPlayerSetting> getAIPlayerSettings() { return s_aiPlayerSettings; }

    // 获取AI玩家数量
    static int getAIPlayerCount() { return s_aiPlayerCount; }
    static void setAIPlayerCount(int count) { s_aiPlayerCount = count; }

private:
    // UI创建
    void createBackground();
    void createTitle();
    void createPlayerCivilizationPanel();
    void createAISettingsPanel();
    void createControlButtons();

    // 按钮回调
    void onCivilizationSelected(cocos2d::Ref* sender, CivilizationType civType);
    void onAIButtonClicked(cocos2d::Ref* sender, int aiIndex);
    void onAIAddClicked(cocos2d::Ref* sender);
    void onAIRemoveClicked(cocos2d::Ref* sender);
    void onStartGameClicked(cocos2d::Ref* sender);
    void onBackClicked(cocos2d::Ref* sender);

    // 更新UI
    void updateSelection(CivilizationType selectedCiv);
    void updateAISettingsDisplay();
    void updateAICountLabel();
    std::string generateNaturalDescription(const CivilizationInfo& civ);
    void updatePortraitColor(const cocos2d::Color3B& civColor);

    // 获取可用的AI文明（排除玩家选择的）
    std::vector<CivilizationType> getAvailableAICivilizations();

    // UI元素
    cocos2d::ui::Layout* _selectedPanel;
    cocos2d::Label* _selectedLabel;
    cocos2d::Label* _abilityLabel;
    cocos2d::Label* _unitLabel;
    cocos2d::Label* _buildingLabel;

    cocos2d::ui::Layout* _aiSettingsPanel;
    cocos2d::Label* _aiCountLabel;
    std::vector<cocos2d::ui::Button*> _aiButtons;

    // 文明信息列表
    std::vector<CivilizationInfo> _civilizations;

    // 当前选中的玩家文明
    CivilizationType _currentSelection;

    // AI玩家设置
    std::vector<AIPlayerSetting> _aiSettings;
    int _aiPlayerCount;

    std::vector<cocos2d::ui::Button*> _civButtons;  // 存储文明按钮

    cocos2d::Label* _leaderLabel;        // 领袖名称
    cocos2d::Label* _descriptionLabel;   // 自然语言描述

    cocos2d::LayerColor* _portraitFrame; // 领袖画像边框

    // 静态变量存储选择
    static CivilizationType s_selectedCivilization;
    static std::vector<AIPlayerSetting> s_aiPlayerSettings;
    static int s_aiPlayerCount;
};

#endif // __CIVILIZATION_SELECTION_SCENE_H__