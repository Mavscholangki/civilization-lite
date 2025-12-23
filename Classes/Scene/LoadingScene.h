// LoadingScene.h
#ifndef __LOADING_SCENE_H__
#define __LOADING_SCENE_H__

#include "cocos2d.h"
#include "GameScene.h"
#include <functional>
#include <vector>

class LoadingScene : public cocos2d::Scene {
public:
    // 创建加载场景，传入一个创建目标场景的函数
    static cocos2d::Scene* createScene(std::function<cocos2d::Scene* ()> sceneCreator);

    virtual bool init();
    CREATE_FUNC(LoadingScene);

    // 设置场景创建函数
    void setSceneCreator(std::function<cocos2d::Scene* ()> creator);

    // 添加要加载的资源
    void addResourceToLoad(const std::string& resourcePath);
    void addResourcesToLoad(const std::vector<std::string>& resources);

    // 开始真正的加载
    void startRealLoading();

    virtual void onExit() override;

private:
    // 加载完成回调
    void onLoadingUpdate(float dt);

    // 资源加载回调
    void onSingleResourceLoaded(cocos2d::Texture2D* texture);

    // 跳转到下一个场景
    void goToNextScene();

    // 同步预创建游戏对象
    void preCreateGameObjectsSync();

    // UI元素
    cocos2d::Label* _loadingLabel;
    cocos2d::ProgressTimer* _progressBar;

    // 资源管理
    std::vector<std::string> _resourcesToLoad;
    int _currentResourceIndex;
    int _totalResources;
    int _successfullyLoaded;
    int _failedToLoad;

    // 场景创建函数
    std::function<cocos2d::Scene* ()> _sceneCreator;

    // 加载状态
    enum class LoadingState {
        INITIALIZING,
        LOADING_TEXTURES,
        PRE_CREATING_SCENE,  // 同步预创建场景
        FINALIZING,
        COMPLETE
    };
    LoadingState _currentState;

    // 计时器（确保最小显示时间）
    float _elapsedTime;
    float _minDisplayTime;

    // 预创建的游戏场景
    cocos2d::Scene* _preCreatedScene;
};

#endif // __LOADING_SCENE_H__