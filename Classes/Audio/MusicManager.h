#ifndef __MUSIC_MANAGER_H__
#define __MUSIC_MANAGER_H__

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "Core/player.h"


/**
 * @brief 音乐管理器单例类
 * @details 负责管理游戏中的背景音乐，包括主菜单音乐和各文明专属音乐
 */
class MusicManager
{
public:
    /**
     * @brief 获取单例实例
     */
    static MusicManager* getInstance();

    /**
     * @brief 销毁单例实例
     */
    static void destroyInstance();

    /**
     * @brief 播放主菜单背景音乐
     */
    void playMainMenuMusic();

    /**
     * @brief 播放指定文明的背景音乐（随机选择一首）
     */
    void playCivilizationMusic(CivilizationType civType);

    /**
     * @brief 播放下一首当前文明的音乐
     */
    void playNextTrack();

    /**
     * @brief 停止当前播放的背景音乐
     */
    void stopMusic();

    /**
     * @brief 暂停当前播放的背景音乐
     */
    void pauseMusic();

    /**
     * @brief 恢复播放背景音乐
     */
    void resumeMusic();

    /**
     * @brief 设置音乐音量
     */
    void setMusicVolume(float volume);

    /**
     * @brief 获取当前音乐音量
     */
    float getMusicVolume() const;

    /**
     * @brief 检查是否正在播放音乐
     */
    bool isPlaying() const;

    /**
     * @brief 预加载所有音乐文件
     */
    void preloadAllMusic();

    /**
     * @brief 清理所有缓存的音乐资源
     */
    void uncacheAllMusic();

private:
    MusicManager();
    ~MusicManager();

    void playMusic(const std::string& musicPath, bool loop = false);
    std::string getMusicFolderForCivilization(CivilizationType civType) const;
    void loadCivilizationMusicList(CivilizationType civType);
    std::string getRandomMusic(const std::vector<std::string>& musicList) const;
    void onMusicFinished(int audioId, const std::string& filePath);

    static MusicManager* _instance;

    int _currentMusicId;
    float _musicVolume;
    std::string _currentMusicPath;
    CivilizationType _currentCivType;

    std::unordered_map<int, std::vector<std::string>> _civMusicLists;

    static const std::string MUSIC_FOLDER_BASE;
    static const std::string MUSIC_FOLDER_THEME;
    static const std::string MUSIC_FOLDER_CHINA;
    static const std::string MUSIC_FOLDER_GERMANY;
    static const std::string MUSIC_FOLDER_RUSSIA;
    static const std::string MUSIC_FILE_MAIN_MENU;
};

#endif // __MUSIC_MANAGER_H__