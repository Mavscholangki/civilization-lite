#ifndef __LOADING_SCENE_H__
#define __LOADING_SCENE_H__

#include "cocos2d.h"

class LoadingScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(LoadingScene);
    ~LoadingScene();

    // 设置要跳转的下一个场景
    void setNextScene(cocos2d::Scene* scene);

private:
    // 加载完成回调
    void onLoadingComplete(float dt);

    // UI元素
    cocos2d::Label* _loadingLabel;
    cocos2d::ProgressTimer* _progressBar;

    // 下一个场景
    cocos2d::Scene* _nextScene;
};

#endif // __LOADING_SCENE_H__