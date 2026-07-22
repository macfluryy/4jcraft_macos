#pragma once
#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include "java/Class.h"
#include "minecraft/Pos.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/util/SmoothFloat.h"
#include "minecraft/world/entity/player/Player.h"


class Level;
class User;
class CompoundTag;
class FurnaceTileEntity;
class DispenserTileEntity;
class SignTileEntity;
class Container;
class Input;
class Stat;
class Minecraft;
class ModelPart;


#define PLAYER_IDLE_TIME 300

class LocalPlayer : public Player {
public:
    static const int SPRINT_DURATION = 20 * 30;

    eINSTANCEOF GetType() { return eTYPE_LOCALPLAYER; }

    Input* input;

protected:
    Minecraft* minecraft;
    int sprintTriggerTime;
    bool sprintTriggerRegisteredReturn;  
    bool twoJumpsRegistered;             

    unsigned int m_uiInactiveTicks;  

    unsigned long long m_awardedThisSession;

    
    

public:
    int sprintTime;

    float yBob, xBob;
    float yBobO, xBobO;

    float portalTime;
    float oPortalTime;

    LocalPlayer(Minecraft* minecraft, Level* level, User* user, int dimension);
    virtual ~LocalPlayer();

    int m_iScreenSection;  
                           
    uint64_t
        ullButtonsPressed;  
                            
    
    

    uint64_t ullDpad_last;
    uint64_t ullDpad_this;
    uint64_t ullDpad_filtered;

    
    
    
    int missTime;
    int lastClickTick[2];
    bool isRaining;
    int m_iThirdPersonView;

    bool m_bHasAwardedStayinFrosty;

private:
    float flyX, flyY, flyZ;

    int jumpRidingTicks;
    float jumpRidingScale;

protected:
    
    int m_iPad;

    bool m_bIsIdle;

private:
    
    
    

    SmoothFloat smoothFlyX;
    SmoothFloat smoothFlyY;
    SmoothFloat smoothFlyZ;

    void calculateFlight(float xa, float ya, float za);

public:
    virtual void serverAiStep();

protected:
    bool isEffectiveAi();

public:
    virtual void aiStep();
    virtual void changeDimension(int i);
    virtual float getFieldOfViewModifier();
    virtual void addAdditonalSaveData(CompoundTag* entityTag);
    virtual void readAdditionalSaveData(CompoundTag* entityTag);
    virtual void closeContainer();
    virtual void openTextEdit(std::shared_ptr<TileEntity> sign);
    virtual bool openContainer(
        std::shared_ptr<Container> container);  
    virtual bool openHopper(
        std::shared_ptr<HopperTileEntity> container);  
    virtual bool openHopper(
        std::shared_ptr<MinecartHopper> container);  
    virtual bool openHorseInventory(
        std::shared_ptr<EntityHorse> horse,
        std::shared_ptr<Container> container);        
    virtual bool startCrafting(int x, int y, int z);  
    virtual bool openFireworks(int x, int y, int z);  
    virtual bool startEnchanting(
        int x, int y, int z, const std::wstring& name);  
    virtual bool startRepairing(int x, int y, int z);
    virtual bool openFurnace(
        std::shared_ptr<FurnaceTileEntity> furnace);  
    virtual bool openBrewingStand(std::shared_ptr<BrewingStandTileEntity>
                                      brewingStand);  
    virtual bool openBeacon(
        std::shared_ptr<BeaconTileEntity> beacon);  
    virtual bool openTrap(
        std::shared_ptr<DispenserTileEntity> trap);  
    virtual bool openTrading(std::shared_ptr<Merchant> traderTarget,
                             const std::wstring& name);
    virtual void crit(std::shared_ptr<Entity> e);
    virtual void magicCrit(std::shared_ptr<Entity> e);
    virtual void take(std::shared_ptr<Entity> e, int orgCount);
    virtual void chat(const std::wstring& message);
    virtual bool isSneaking();
    
    virtual void hurtTo(float newHealth, uint8_t damageSource);
    virtual void respawn();
    virtual void animateRespawn();
    virtual void displayClientMessage(int messageId);
    virtual void awardStat(Stat* stat, const std::vector<uint8_t>& param);
    virtual int ThirdPersonView() { return m_iThirdPersonView; }
    
    
    virtual void SetThirdPersonView(int val) { m_iThirdPersonView = val; }

    void ResetInactiveTicks() { m_uiInactiveTicks = 0; }
    unsigned int GetInactiveTicks() { return m_uiInactiveTicks; }
    void IncrementInactiveTicks() {
        if (m_uiInactiveTicks < 255) m_uiInactiveTicks++;
    }

    void mapPlayerChunk(unsigned int);
    
    void SetXboxPad(int iPad) { m_iPad = iPad; }
    int GetXboxPad() { return m_iPad; }
    void SetPlayerRespawned(bool bVal) { m_bPlayerRespawned = bVal; }
    bool GetPlayerRespawned() { return m_bPlayerRespawned; }

    
    
    void handleMouseDown(int button, bool down);
    bool handleMouseClick(int button);

    
    bool creativeModeHandleMouseClick(int button, bool buttonPressed);
    float lastClickX;
    float lastClickY;
    float lastClickZ;
    float lastClickdX;
    float lastClickdY;
    float lastClickdZ;
    enum eLastClickState {
        lastClick_invalid,
        lastClick_init,
        lastClick_moving,
        lastClick_stopped,
        lastClick_oldRepeat,
        lastClick_disabled
    };
    float lastClickTolerance;
    int lastClickState;

    
    
    virtual void onCrafted(std::shared_ptr<ItemInstance> item);

    virtual void setAndBroadcastCustomSkin(uint32_t skinId);
    virtual void setAndBroadcastCustomCape(uint32_t capeId);

private:
    bool isSolidBlock(int x, int y, int z);
    bool m_bPlayerRespawned;

protected:
    bool checkInTile(double x, double y, double z);

public:
    void setSprinting(bool value);
    void setExperienceValues(float experienceProgress, int totalExp,
                             int experienceLevel);

    
    virtual Pos getCommandSenderWorldPosition();
    virtual std::shared_ptr<ItemInstance> getCarriedItem();
    virtual void playSound(int soundId, float volume, float pitch);
    bool isRidingJumpable();
    float getJumpRidingScale();

protected:
    virtual void sendRidingJump();

public:
    bool hasPermission(EGameCommand command);

    void updateRichPresence();

    
    float m_sessionTimeStart;
    float m_dimensionTimeStart;

    void SetSessionTimerStart(void);
    float getSessionTimer(void);

    float getAndResetChangeDimensionTimer();

    virtual void handleCollectItem(std::shared_ptr<ItemInstance> item);
    void SetPlayerAdditionalModelParts(
        std::vector<ModelPart*>& pAdditionalModelParts);

private:
    std::vector<ModelPart*> m_pAdditionalModelParts;
};
