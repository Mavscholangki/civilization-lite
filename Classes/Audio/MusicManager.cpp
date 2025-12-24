#include "MusicManager.h"
#include "platform/CCFileUtils.h"
#include <random>
#include <ctime>

USING_NS_CC;

// 静态成员初始化
MusicManager* MusicManager::_instance = nullptr;

// 音乐文件夹路径定义
const std::string MusicManager::MUSIC_FOLDER_BASE = "Private/music/";
const std::string MusicManager::MUSIC_FOLDER_THEME = "Private/music/Theme/";
const std::string MusicManager::MUSIC_FOLDER_CHINA = "Private/music/China/";
const std::string MusicManager::MUSIC_FOLDER_GERMANY = "Private/music/Germany/";
const std::string MusicManager::MUSIC_FOLDER_RUSSIA = "Private/music/Russia/";
const std::string MusicManager::MUSIC_FILE_MAIN_MENU = "Private/music/Theme/MainMenu.ogg";

MusicManager* MusicManager::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new (std::nothrow) MusicManager();
    }
    return _instance;
}

void MusicManager::destroyInstance()
{
    if (_instance != nullptr)
    {
        delete _instance;
        _instance = nullptr;
    }
}

MusicManager::MusicManager()
    : _currentMusicId(AudioEngine::INVALID_AUDIO_ID)
    , _musicVolume(0.8f)
    , _currentMusicPath("")
    , _currentCivType(CivilizationType::CHINA)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

MusicManager::~MusicManager()
{
    stopMusic();
    uncacheAllMusic();
}

void MusicManager::playMainMenuMusic()
{
    playMusic(MUSIC_FILE_MAIN_MENU, true);
}

void MusicManager::playCivilizationMusic(CivilizationType civType)
{
    _currentCivType = civType;
    loadCivilizationMusicList(civType);

    int civIndex = static_cast<int>(civType);
    auto it = _civMusicLists.find(civIndex);

    if (it != _civMusicLists.end() && !it->second.empty())
    {
        std::string musicPath = getRandomMusic(it->second);
        playMusic(musicPath, false);
    }
    else
    {
        CCLOG("MusicManager: No music found for civilization %d", civIndex);
        playMusic(MUSIC_FILE_MAIN_MENU, true);
    }
}

void MusicManager::playNextTrack()
{
    playCivilizationMusic(_currentCivType);
}

void MusicManager::loadCivilizationMusicList(CivilizationType civType)
{
    int civIndex = static_cast<int>(civType);

    if (_civMusicLists.find(civIndex) != _civMusicLists.end())
    {
        return;
    }

    std::string folderPath = getMusicFolderForCivilization(civType);
    std::vector<std::string> musicFiles;

    std::vector<std::string> possibleFiles = {
        "1.ogg",
        "2.ogg",
        "3.ogg",
        "4.ogg",
        "5.ogg",
        "6.ogg",
        "7.ogg"
    };

    for (const auto& fileName : possibleFiles)
    {
        std::string fullPath = folderPath + fileName;
        if (FileUtils::getInstance()->isFileExist(fullPath))
        {
            musicFiles.push_back(fullPath);
            CCLOG("MusicManager: Found music file: %s", fullPath.c_str());
        }
    }

    _civMusicLists[civIndex] = musicFiles;
    CCLOG("MusicManager: Loaded %zu music files for civilization %d", musicFiles.size(), civIndex);
}

std::string MusicManager::getMusicFolderForCivilization(CivilizationType civType) const
{
    switch (civType)
    {
    case CivilizationType::CHINA:
        return MUSIC_FOLDER_CHINA;
    case CivilizationType::GERMANY:
        return MUSIC_FOLDER_GERMANY;
    case CivilizationType::RUSSIA:
        return MUSIC_FOLDER_RUSSIA;
    default:
        return MUSIC_FOLDER_THEME;
    }
}

std::string MusicManager::getRandomMusic(const std::vector<std::string>& musicList) const
{
    if (musicList.empty())
    {
        return MUSIC_FILE_MAIN_MENU;
    }

    if (musicList.size() == 1)
    {
        return musicList[0];
    }

    std::string selected;
    int maxAttempts = 5;
    do
    {
        int index = std::rand() % musicList.size();
        selected = musicList[index];
        maxAttempts--;
    } while (selected == _currentMusicPath && maxAttempts > 0);

    return selected;
}

void MusicManager::playMusic(const std::string& musicPath, bool loop)
{
    if (_currentMusicPath == musicPath &&
        _currentMusicId != AudioEngine::INVALID_AUDIO_ID &&
        AudioEngine::getState(_currentMusicId) == AudioEngine::AudioState::PLAYING)
    {
        return;
    }

    stopMusic();

    if (!FileUtils::getInstance()->isFileExist(musicPath))
    {
        CCLOG("MusicManager: Music file not found: %s", musicPath.c_str());
        if (musicPath != MUSIC_FILE_MAIN_MENU && FileUtils::getInstance()->isFileExist(MUSIC_FILE_MAIN_MENU))
        {
            _currentMusicPath = MUSIC_FILE_MAIN_MENU;
        }
        else
        {
            return;
        }
    }
    else
    {
        _currentMusicPath = musicPath;
    }

    _currentMusicId = AudioEngine::play2d(_currentMusicPath, loop, _musicVolume);

    if (_currentMusicId == AudioEngine::INVALID_AUDIO_ID)
    {
        _currentMusicPath = "";
    }
    else
    {
        CCLOG("MusicManager: Playing: %s", _currentMusicPath.c_str());
        if (!loop)
        {
            AudioEngine::setFinishCallback(_currentMusicId,
                [this](int audioId, const std::string& filePath) {
                    this->onMusicFinished(audioId, filePath);
                });
        }
    }
}

void MusicManager::onMusicFinished(int audioId, const std::string& filePath)
{
    _currentMusicId = AudioEngine::INVALID_AUDIO_ID;
    _currentMusicPath = "";
    playNextTrack();
}

void MusicManager::stopMusic()
{
    if (_currentMusicId != AudioEngine::INVALID_AUDIO_ID)
    {
        AudioEngine::stop(_currentMusicId);
        _currentMusicId = AudioEngine::INVALID_AUDIO_ID;
        _currentMusicPath = "";
    }
}

void MusicManager::pauseMusic()
{
    if (_currentMusicId != AudioEngine::INVALID_AUDIO_ID)
    {
        AudioEngine::pause(_currentMusicId);
    }
}

void MusicManager::resumeMusic()
{
    if (_currentMusicId != AudioEngine::INVALID_AUDIO_ID)
    {
        AudioEngine::resume(_currentMusicId);
    }
}

void MusicManager::setMusicVolume(float volume)
{
    _musicVolume = std::max(0.0f, std::min(1.0f, volume));
    if (_currentMusicId != AudioEngine::INVALID_AUDIO_ID)
    {
        AudioEngine::setVolume(_currentMusicId, _musicVolume);
    }
}

float MusicManager::getMusicVolume() const
{
    return _musicVolume;
}

bool MusicManager::isPlaying() const
{
    if (_currentMusicId == AudioEngine::INVALID_AUDIO_ID)
    {
        return false;
    }
    return AudioEngine::getState(_currentMusicId) == AudioEngine::AudioState::PLAYING;
}

void MusicManager::preloadAllMusic()
{
    AudioEngine::preload(MUSIC_FILE_MAIN_MENU);
}

void MusicManager::uncacheAllMusic()
{
    AudioEngine::uncache(MUSIC_FILE_MAIN_MENU);
    for (auto& pair : _civMusicLists)
    {
        for (const auto& musicPath : pair.second)
        {
            AudioEngine::uncache(musicPath);
        }
    }
    _civMusicLists.clear();
}