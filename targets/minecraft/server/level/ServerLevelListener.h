#pragma once
#include <string>

#include "minecraft/core/particles/ParticleTypes.h"
#include "minecraft/world/level/LevelListener.h"

class Player;
class TileEntity;
class Entity;
class MinecraftServer;
class ServerLevel;


class ServerLevelListener : public LevelListener {
private:
    MinecraftServer* server;
    ServerLevel* level;

public:
    ServerLevelListener(MinecraftServer* server, ServerLevel* level);
    
    
    virtual void addParticle(ePARTICLE_TYPE name, double x, double y, double z,
                             double xa, double ya, double za);  
    virtual void allChanged();
    virtual void entityAdded(std::shared_ptr<Entity> entity);
    virtual void entityRemoved(std::shared_ptr<Entity> entity);
    virtual void playerRemoved(
        std::shared_ptr<Entity>
            entity);  
                      
    virtual void playSound(int iSound, double x, double y, double z,
                           float volume, float pitch, float fClipSoundDist);
    virtual void playSoundExceptPlayer(std::shared_ptr<Player> player,
                                       int iSound, double x, double y, double z,
                                       float volume, float pitch,
                                       float fSoundClipDist);
    virtual void setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1,
                               Level* level);  
    virtual void skyColorChanged();
    virtual void tileChanged(int x, int y, int z);
    virtual void tileLightChanged(int x, int y, int z);
    virtual void playStreamingMusic(const std::wstring& name, int x, int y,
                                    int z);
    virtual void levelEvent(std::shared_ptr<Player> source, int type, int x,
                            int y, int z, int data);
    virtual void globalLevelEvent(int type, int sourceX, int sourceY,
                                  int sourceZ, int data);
    virtual void destroyTileProgress(int id, int x, int y, int z, int progress);
};
