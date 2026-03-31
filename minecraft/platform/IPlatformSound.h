#pragma once

#include <memory>
#include <string>

class File;
class Mob;
class Options;

class IPlatformSound {
public:
    virtual ~IPlatformSound() = default;

    virtual void init(Options*) = 0;
    virtual void destroy() = 0;
    virtual void tick(std::shared_ptr<Mob>* players, float a) = 0;

    // SFX
    virtual void play(int iSound, float x, float y, float z, float volume,
                      float pitch) = 0;
    virtual void playUI(int iSound, float volume, float pitch) = 0;

    // Streaming / music
    virtual void playStreaming(const std::wstring& name, float x, float y,
                               float z, float volume, float pitch,
                               bool bMusicDelay = true) = 0;
    virtual void playMusicTick() = 0;
    virtual void updateMusicVolume(float fVal) = 0;
    virtual void updateSystemMusicPlaying(bool isPlaying) = 0;
    virtual void updateSoundEffectVolume(float fVal) = 0;

    // Asset registration
    virtual void add(const std::wstring& name, File* file) = 0;
    virtual void addMusic(const std::wstring& name, File* file) = 0;
    virtual void addStreaming(const std::wstring& name, File* file) = 0;
    virtual char* ConvertSoundPathToName(const std::wstring& name,
                                         bool bConvertSpaces = false) = 0;
};
