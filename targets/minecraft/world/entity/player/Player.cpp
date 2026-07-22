








#include "Player.h"

#include <limits.h>
#include <wchar.h>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

#include "Inventory.h"
#include "app/common/App_enums.h"
#include "app/common/App_structs.h"
#include "app/common/Minecraft_Macros.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCSkinFile.h"
#include "app/mac/MacGame.h"
#include "java/JavaMath.h"
#include "java/Random.h"
#include "minecraft/Direction.h"
#include "minecraft/Pos.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/model/HumanoidModel.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/core/particles/ParticleTypes.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/stats/GenericStats.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/Difficulty.h"
#include "minecraft/world/damageSource/CombatTracker.h"
#include "minecraft/world/damageSource/DamageSource.h"
#include "minecraft/world/effect/MobEffect.h"
#include "minecraft/world/effect/MobEffectInstance.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/EntityEvent.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/SyncedEntityData.h"
#include "minecraft/world/entity/ai/attributes/AttributeInstance.h"
#include "minecraft/world/entity/ai/attributes/BaseAttributeMap.h"
#include "minecraft/world/entity/animal/Pig.h"
#include "minecraft/world/entity/boss/MultiEntityMob.h"
#include "minecraft/world/entity/boss/MultiEntityMobPart.h"
#include "minecraft/world/entity/item/ItemEntity.h"
#include "minecraft/world/entity/item/Minecart.h"
#include "minecraft/world/entity/monster/Monster.h"
#include "minecraft/world/entity/monster/SharedMonsterAttributes.h"
#include "minecraft/world/entity/player/Abilities.h"
#include "minecraft/world/entity/projectile/Arrow.h"
#include "minecraft/world/food/FoodConstants.h"
#include "minecraft/world/food/FoodData.h"
#include "minecraft/world/inventory/AbstractContainerMenu.h"
#include "minecraft/world/inventory/InventoryMenu.h"
#include "minecraft/world/inventory/PlayerEnderChestContainer.h"
#include "minecraft/world/item/BowItem.h"
#include "minecraft/world/item/FishingRodItem.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/item/UseAnim.h"
#include "minecraft/world/item/enchantment/EnchantmentHelper.h"
#include "minecraft/world/item/enchantment/ThornsEnchantment.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/material/Material.h"
#include "minecraft/world/level/tile/BedTile.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/phys/AABB.h"
#include "minecraft/world/phys/Vec3.h"
#include "minecraft/world/scores/PlayerTeam.h"
#include "minecraft/world/scores/Score.h"
#include "minecraft/world/scores/Scoreboard.h"
#include "minecraft/world/scores/Team.h"
#include "minecraft/world/scores/criteria/ObjectiveCriteria.h"
#include "nbt/CompoundTag.h"
#include "nbt/ListTag.h"

class ModelPart;
class Objective;
class Stat;

void Player::_init() {
    registerAttributes();
    setHealth(getMaxHealth());

    inventory = std::make_shared<Inventory>(this);

    userType = 0;
    oBob = bob = 0.0f;

    xCloakO = yCloakO = zCloakO = 0.0;
    xCloak = yCloak = zCloak = 0.0;

    m_isSleeping = false;

    customTextureUrl = L"";
    customTextureUrl2 = L"";
    m_uiPlayerCurrentSkin = 0;

    bedPosition = nullptr;

    sleepCounter = 0;
    deathFadeCounter = 0;

    bedOffsetX = bedOffsetY = bedOffsetZ = 0.0f;
    stats = nullptr;

    respawnPosition = nullptr;
    respawnForced = false;
    minecartAchievementPos = nullptr;

    fishing = nullptr;

    distanceWalk = distanceSwim = distanceFall = distanceClimb =
        distanceMinecart = distanceBoat = distancePig = 0;

    m_uiDebugOptions = 0L;

    jumpTriggerTime = 0;
    takeXpDelay = 0;
    experienceLevel = totalExperience = 0;
    experienceProgress = 0.0f;

    useItem = nullptr;
    useItemDuration = 0;

    defaultWalkSpeed = 0.1f;
    defaultFlySpeed = 0.02f;

    lastLevelUpTime = 0;

    m_uiGamePrivileges = 0;

    m_ppAdditionalModelParts = nullptr;
    m_bCheckedForModelParts = false;
    m_bCheckedDLCForModelParts = false;

    enderChestInventory = std::shared_ptr<PlayerEnderChestContainer>(
        new PlayerEnderChestContainer());

    m_bAwardedOnARail = false;
}

Player::Player(Level* level, const std::wstring& name) : LivingEntity(level) {
    
    
    this->defineSynchedData();

    this->name = name;

    _init();
    inventoryMenu = new InventoryMenu(inventory, !level->isClientSide, this);

    containerMenu = inventoryMenu;

    heightOffset = 1.62f;
    Pos* spawnPos = level->getSharedSpawnPos();
    moveTo(spawnPos->x + 0.5, spawnPos->y + 1, spawnPos->z + 0.5, 0, 0);
    delete spawnPos;

    rotOffs = 180;
    flameTime = 20;

    m_skinIndex = EDefaultSkins::Skin0;
    m_playerIndex = 0;
    m_dwSkinId = 0;
    m_dwCapeId = 0;

    
    m_xuid = INVALID_XUID;
    m_OnlineXuid = INVALID_XUID;
    
    setShowOnMaps(
        app.GetGameHostOption(eGameHostOption_Gamertags) != 0 ? true : false);
    m_bIsGuest = false;

    
    
    setUUID(name);
}

Player::~Player() {
    
    
    delete inventoryMenu;

    
    
    
    
    
}

void Player::registerAttributes() {
    LivingEntity::registerAttributes();

    getAttributes()
        ->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)
        ->setBaseValue(1);
}

void Player::defineSynchedData() {
    LivingEntity::defineSynchedData();

    entityData->define(DATA_PLAYER_FLAGS_ID, (uint8_t)0);
    entityData->define(DATA_PLAYER_ABSORPTION_ID, (float)0);
    entityData->define(DATA_SCORE_ID, (int)0);
}

std::shared_ptr<ItemInstance> Player::getUseItem() { return useItem; }

int Player::getUseItemDuration() { return useItemDuration; }

bool Player::isUsingItem() { return useItem != nullptr; }

int Player::getTicksUsingItem() {
    if (isUsingItem()) {
        return useItem->getUseDuration() - useItemDuration;
    }
    return 0;
}

void Player::releaseUsingItem() {
    if (useItem != nullptr) {
        useItem->releaseUsing(
            level, std::dynamic_pointer_cast<Player>(shared_from_this()),
            useItemDuration);

        
        
        if (useItem->count == 0) {
            removeSelectedItem();
        }
    }
    stopUsingItem();
}

void Player::stopUsingItem() {
    useItem = nullptr;
    useItemDuration = 0;
    if (!level->isClientSide) {
        setUsingItemFlag(false);
    }
}

bool Player::isBlocking() {
    return isUsingItem() &&
           Item::items[useItem->id]->getUseAnimation(useItem) == UseAnim_block;
}



void Player::updateFrameTick() {
    if (useItem != nullptr) {
        std::shared_ptr<ItemInstance> item = inventory->getSelected();
        
        
        
        
        
        
        
        
        if (item == nullptr || !item->equals(useItem)) {
            stopUsingItem();
        } else {
            if (useItemDuration <= 25 && useItemDuration % 4 == 0) {
                spawnEatParticles(item, 5);
            }
            if (--useItemDuration == 0) {
                if (!level->isClientSide) {
                    completeUsingItem();
                }
            }
        }
    }

    if (takeXpDelay > 0) takeXpDelay--;

    if (isSleeping()) {
        sleepCounter++;
        if (sleepCounter > SLEEP_DURATION) {
            sleepCounter = SLEEP_DURATION;
        }

        if (!level->isClientSide) {
            if (!checkBed()) {
                stopSleepInBed(true, true, false);
            } else if (level->isDay()) {
                stopSleepInBed(false, true, true);
            }
        }
    } else if (sleepCounter > 0) {
        sleepCounter++;
        if (sleepCounter >= (SLEEP_DURATION + WAKE_UP_DURATION)) {
            sleepCounter = 0;
        }
    }

    if (!isAlive()) {
        deathFadeCounter++;
        if (deathFadeCounter > DEATHFADE_DURATION) {
            deathFadeCounter = DEATHFADE_DURATION;
        }
    }
}

void Player::tick() {
    if (level->isClientSide) {
        
        
        updateFrameTick();
    }

    LivingEntity::tick();

    if (!level->isClientSide) {
        if (containerMenu != nullptr &&
            !containerMenu->stillValid(
                std::dynamic_pointer_cast<Player>(shared_from_this()))) {
            closeContainer();
            containerMenu = inventoryMenu;
        }
    }

    if (isOnFire() && (abilities.invulnerable || hasInvulnerablePrivilege())) {
        clearFire();
    }

    xCloakO = xCloak;
    yCloakO = yCloak;
    zCloakO = zCloak;

    double xca = x - xCloak;
    double yca = y - yCloak;
    double zca = z - zCloak;

    double m = 10;
    if (xca > m) xCloakO = xCloak = x;
    if (zca > m) zCloakO = zCloak = z;
    if (yca > m) yCloakO = yCloak = y;
    if (xca < -m) xCloakO = xCloak = x;
    if (zca < -m) zCloakO = zCloak = z;
    if (yca < -m) yCloakO = yCloak = y;

    xCloak += xca * 0.25;
    zCloak += zca * 0.25;
    yCloak += yca * 0.25;

    if (riding == nullptr) {
        if (minecartAchievementPos != nullptr) {
            delete minecartAchievementPos;
            minecartAchievementPos = nullptr;
        }
    }

    if (!level->isClientSide) {
        foodData.tick(std::dynamic_pointer_cast<Player>(shared_from_this()));
    }

    
    if (!level->isClientSide) {
        static int count = 0;
        if (count++ == 100) {
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            

            
            
            
            

            
            


























            
            
            
            

            {
                
                
                
                
            }
        }
    }
    
}

int Player::getPortalWaitTime() {
    return abilities.invulnerable ? 0 : SharedConstants::TICKS_PER_SECOND * 4;
}

int Player::getDimensionChangingDelay() {
    return SharedConstants::TICKS_PER_SECOND / 2;
}

void Player::playSound(int iSound, float volume, float pitch) {
    
    
    level->playPlayerSound(
        std::dynamic_pointer_cast<Player>(shared_from_this()), iSound, volume,
        pitch);
}

void Player::spawnEatParticles(std::shared_ptr<ItemInstance> useItem,
                               int count) {
    if (useItem->getUseAnimation() == UseAnim_drink) {
        playSound(eSoundType_RANDOM_DRINK, 0.5f,
                  level->random->nextFloat() * 0.1f + 0.9f);
    }
    if (useItem->getUseAnimation() == UseAnim_eat) {
        for (int i = 0; i < count; i++) {
            Vec3 d{(random->nextFloat() - 0.5) * 0.1,
                   Math::random() * 0.1 + 0.1, 0};

            d.xRot(-xRot * std::numbers::pi / 180);
            d.yRot(-yRot * std::numbers::pi / 180);

            Vec3 p{(random->nextFloat() - 0.5) * 0.3,
                   -random->nextFloat() * 0.6 - 0.3, 0.6};
            p.xRot(-xRot * std::numbers::pi / 180);
            p.yRot(-yRot * std::numbers::pi / 180);
            p = p.add(x, y + getHeadHeight(), z);

            level->addParticle(PARTICLE_ICONCRACK(useItem->getItem()->id, 0),
                               p.x, p.y, p.z, d.x, d.y + 0.05, d.z);
        }

        
        playSound(eSoundType_RANDOM_EAT, 0.5f + 0.5f * random->nextInt(2),
                  (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
    }
}

void Player::completeUsingItem() {
    if (useItem != nullptr) {
        spawnEatParticles(useItem, 16);

        int oldCount = useItem->count;
        std::shared_ptr<ItemInstance> itemInstance = useItem->useTimeDepleted(
            level, std::dynamic_pointer_cast<Player>(shared_from_this()));
        if (itemInstance != useItem ||
            (itemInstance != nullptr && itemInstance->count != oldCount)) {
            inventory->items[inventory->selected] = itemInstance;
            if (itemInstance->count == 0) {
                inventory->items[inventory->selected] = nullptr;
            }
        }
        stopUsingItem();
    }
}

void Player::handleEntityEvent(uint8_t id) {
    if (id == EntityEvent::USE_ITEM_COMPLETE) {
        completeUsingItem();
    } else {
        LivingEntity::handleEntityEvent(id);
    }
}

bool Player::isImmobile() { return getHealth() <= 0 || isSleeping(); }

void Player::closeContainer() { containerMenu = inventoryMenu; }

void Player::ride(std::shared_ptr<Entity> e) {
    if (riding != nullptr && e == nullptr) {
        if (!level->isClientSide) findStandUpPosition(riding);

        if (riding != nullptr) {
            riding->rider = std::weak_ptr<Entity>();
        }
        riding = nullptr;

        return;
    }
    LivingEntity::ride(e);
}

void Player::setPlayerDefaultSkin(EDefaultSkins skin) {
#if !defined(_CONTENT_PACKAGE)
    wprintf(L"Setting default skin to %d for player %ls\n", std::to_underlying(skin), name.c_str());
#endif
    m_skinIndex = skin;
}

void Player::setCustomSkin(std::uint32_t skinId) {
#if !defined(_CONTENT_PACKAGE)
    wprintf(L"Attempting to set skin to %08X for player %ls\n", skinId,
            name.c_str());
#endif
    EDefaultSkins playerSkin = EDefaultSkins::ServerSelected;

    
    setIsIdle(false);

    setAnimOverrideBitmask(getSkinAnimOverrideBitmask(skinId));
    if (!GET_IS_DLC_SKIN_FROM_BITMASK(skinId)) {
        
        
        std::uint32_t ugcSkinIndex = GET_UGC_SKIN_ID_FROM_BITMASK(skinId);
        std::uint32_t defaultSkinIndex =
            GET_DEFAULT_SKIN_ID_FROM_BITMASK(skinId);
        if (ugcSkinIndex == 0 && defaultSkinIndex > 0) {
            playerSkin = static_cast<EDefaultSkins>(defaultSkinIndex);
        }
    }

    if (playerSkin == EDefaultSkins::ServerSelected) {
        playerSkin = static_cast<EDefaultSkins>(m_playerIndex + 1);
    }

    
    
    setPlayerDefaultSkin(playerSkin);

    m_dwSkinId = skinId;
    this->customTextureUrl = app.getSkinPathFromId(skinId);

    
    






































    
    m_bCheckedForModelParts = false;
    m_bCheckedDLCForModelParts = false;
    this->SetAdditionalModelParts(nullptr);
}

unsigned int Player::getSkinAnimOverrideBitmask(std::uint32_t skinId) {
    unsigned long bitmask = 0L;
    if (GET_IS_DLC_SKIN_FROM_BITMASK(skinId)) {
        
        switch (GET_DLC_SKIN_ID_FROM_BITMASK(skinId)) {
            case 0x2:    
            case 0x3:    
            case 0xc8:   
            case 0xc9:   
            case 0x1f8:  
            case 0x220:  
            case 0x23a:  
            case 0x23d:  
            case 0x247:  
            case 0x194:  
            case 0x195:  
                bitmask = 1 << HumanoidModel::eAnim_ArmsOutFront;
                break;
            case 0x1fa:  
                bitmask = 1 << HumanoidModel::eAnim_ArmsOutFront |
                          1 << HumanoidModel::eAnim_NoLegAnim;
                break;
            case 0x1f4:  
                bitmask = 1 << HumanoidModel::eAnim_ArmsDown |
                          1 << HumanoidModel::eAnim_NoLegAnim;
                break;
            case 0x1f7:  
                
                break;
                break;
            default:
                
                
                bitmask = app.GetAnimOverrideBitmask(skinId);
                break;
        }
    }
    return bitmask;
}

void Player::setXuid(PlayerUID xuid) { m_xuid = xuid; }

void Player::setCustomCape(std::uint32_t capeId) {
#if !defined(_CONTENT_PACKAGE)
    wprintf(L"Attempting to set cape to %08X for player %s\n", capeId,
            name.c_str());
#endif

    m_dwCapeId = capeId;

    if (capeId > 0) {
        this->customTextureUrl2 = Player::getCapePathFromId(capeId);
    } else {
        MOJANG_DATA* pMojangData = app.GetMojangDataForXuid(getOnlineXuid());
        if (pMojangData) {
            
            if (pMojangData->wchCape[0] != 0) {
                this->customTextureUrl2 = pMojangData->wchCape;
            } else {
                if (app.DefaultCapeExists()) {
                    this->customTextureUrl2 = std::wstring(L"Special_Cape.png");
                } else {
                    this->customTextureUrl2 = std::wstring(L"");
                }
            }

        } else {
            
            if (app.DefaultCapeExists()) {
                this->customTextureUrl2 = std::wstring(L"Special_Cape.png");
            } else {
                this->customTextureUrl2 = std::wstring(L"");
            }
        }
    }
}

std::uint32_t Player::getCapeIdFromPath(const std::wstring& cape) {
    bool dlcCape = false;
    std::uint32_t capeId = 0;

    if (cape.size() >= 14) {
        dlcCape = cape.substr(0, 3).compare(L"dlc") == 0;

        std::wstring capeValue = cape.substr(7, cape.size());
        capeValue = capeValue.substr(0, capeValue.find_first_of(L'.'));

        std::wstringstream ss;
        
        
        
        if (dlcCape)
            ss << std::dec << capeValue.c_str();
        else
            ss << std::hex << capeValue.c_str();
        ss >> capeId;

        capeId = MAKE_SKIN_BITMASK(dlcCape, capeId);
    }
    return capeId;
}

std::wstring Player::getCapePathFromId(std::uint32_t capeId) {
    
    
    
    wchar_t chars[256];
    if (GET_IS_DLC_SKIN_FROM_BITMASK(capeId)) {
        
        
        swprintf(chars, 256, L"dlccape%08d.png",
                 GET_DLC_SKIN_ID_FROM_BITMASK(capeId));

    } else {
        std::uint32_t ugcCapeIndex = GET_UGC_SKIN_ID_FROM_BITMASK(capeId);
        std::uint32_t defaultCapeIndex =
            GET_DEFAULT_SKIN_ID_FROM_BITMASK(capeId);
        if (ugcCapeIndex == 0) {
            swprintf(chars, 256, L"defcape%08X.png", defaultCapeIndex);
        } else {
            swprintf(chars, 256, L"ugccape%08X.png", ugcCapeIndex);
        }
    }
    return chars;
}

void Player::ChangePlayerSkin() {
    if (app.vSkinNames.size() > 0) {
        m_uiPlayerCurrentSkin++;
        if (m_uiPlayerCurrentSkin > app.vSkinNames.size()) {
            m_uiPlayerCurrentSkin = 0;
            this->customTextureUrl = L"";
        } else {
            if (m_uiPlayerCurrentSkin > 0) {
                
                this->customTextureUrl =
                    app.vSkinNames[m_uiPlayerCurrentSkin - 1];
            }
        }
    }
}

void Player::prepareCustomTextures() {
    MOJANG_DATA* pMojangData = app.GetMojangDataForXuid(getOnlineXuid());

    if (pMojangData) {
        
        if (pMojangData->wchSkin[0] != 0) {
            this->customTextureUrl = pMojangData->wchSkin;
        }

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

    } else {
        
        
        
        
        
        
        
        
        
        
    }

    


    
}

void Player::rideTick() {
    if (!level->isClientSide && isSneaking()) {
        ride(nullptr);
        setSneaking(false);
        return;
    }

    double preX = x, preY = y, preZ = z;
    float preYRot = yRot, preXRot = xRot;

    LivingEntity::rideTick();
    oBob = bob;
    bob = 0;

    checkRidingStatistiscs(x - preX, y - preY, z - preZ);

    
    if (riding != nullptr && (riding->GetType() & eTYPE_PIG) == eTYPE_PIG) {
        
        
        
        

        std::shared_ptr<Pig> pig = std::dynamic_pointer_cast<Pig>(riding);
        yBodyRot = pig->yBodyRot;

        while (yBodyRot - yBodyRotO < -180) yBodyRotO -= 360;
        while (yBodyRot - yBodyRotO >= 180) yBodyRotO += 360;
    }
}

void Player::resetPos() {
    heightOffset = 1.62f;
    setSize(0.6f, 1.8f);
    LivingEntity::resetPos();
    setHealth(getMaxHealth());
    deathTime = 0;
}

void Player::serverAiStep() {
    LivingEntity::serverAiStep();
    updateSwingTime();
}

void Player::aiStep() {
    if (jumpTriggerTime > 0) jumpTriggerTime--;

    if (level->difficulty == Difficulty::PEACEFUL &&
        getHealth() < getMaxHealth() &&
        level->getGameRules()->getBoolean(
            GameRules::RULE_NATURAL_REGENERATION)) {
        if (tickCount % 20 * 12 == 0) heal(1);
    }
    inventory->tick();
    oBob = bob;

    LivingEntity::aiStep();

    AttributeInstance* speed =
        getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    if (!level->isClientSide) speed->setBaseValue(abilities.getWalkingSpeed());
    flyingSpeed = defaultFlySpeed;
    if (isSprinting()) {
        flyingSpeed += defaultFlySpeed * 0.3f;
    }

    setSpeed((float)speed->getValue());

    float tBob = (float)sqrt(xd * xd + zd * zd);

    
    if ((xd * xd + zd * zd) < 0.00001f) {
        tBob = 0.0f;
    }

    float tTilt = (float)atan(-yd * 0.2f) * 15.0f;
    if (tBob > 0.1f) tBob = 0.1f;
    if (!onGround || getHealth() <= 0) tBob = 0;
    if (onGround || getHealth() <= 0) tTilt = 0;

    bob += (tBob - bob) * 0.4f;

    tilt += (tTilt - tilt) * 0.8f;

    if (getHealth() > 0) {
        AABB pickupArea;
        if (riding != nullptr && !riding->removed) {
            
            
            pickupArea = bb.minmax(riding->bb).grow(1, 0, 1);
        } else {
            pickupArea = bb.grow(1, .5, 1);
        }

        std::vector<std::shared_ptr<Entity>>& entities = m_pickupScratch;
        level->getEntities(shared_from_this(), &pickupArea, entities);
        auto itEnd = entities.end();
        for (auto it = entities.begin(); it != itEnd; it++) {
            std::shared_ptr<Entity> e = *it;  
            if (!e->removed) {
                touch(e);
            }
        }
    }
}

void Player::touch(std::shared_ptr<Entity> entity) {
    entity->playerTouch(std::dynamic_pointer_cast<Player>(shared_from_this()));
}

int Player::getScore() { return entityData->getInteger(DATA_SCORE_ID); }

void Player::setScore(int value) { entityData->set(DATA_SCORE_ID, value); }

void Player::increaseScore(int amount) {
    int score = getScore();
    entityData->set(DATA_SCORE_ID, score + amount);
}

void Player::die(DamageSource* source) {
    LivingEntity::die(source);
    setSize(0.2f, 0.2f);
    setPos(x, y, z);
    yd = 0.1f;

    
    if (app.isXuidNotch(m_xuid)) {
        drop(std::make_shared<ItemInstance>(Item::apple, 1), true);
    }
    if (!level->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY)) {
        inventory->dropAll();
    }

    if (source != nullptr) {
        xd = -cosf((hurtDir + yRot) * std::numbers::pi / 180) * 0.1f;
        zd = -sinf((hurtDir + yRot) * std::numbers::pi / 180) * 0.1f;
    } else {
        xd = zd = 0;
    }
    heightOffset = 0.1f;
}

void Player::awardKillScore(std::shared_ptr<Entity> victim, int awardPoints) {
    increaseScore(awardPoints);
    std::vector<Objective*>* objectives =
        getScoreboard()->findObjectiveFor(ObjectiveCriteria::KILL_COUNT_ALL);

    
    
    
    
    
    
    
    
    

    if (objectives) {
        for (auto it = objectives->begin(); it != objectives->end(); ++it) {
            Objective* objective = *it;
            Score* score =
                getScoreboard()->getPlayerScore(getAName(), objective);
            score->increment();
        }
    }
}

bool Player::isShootable() { return true; }

bool Player::isCreativeModeAllowed() { return true; }

std::shared_ptr<ItemEntity> Player::drop(bool all) {
    return drop(inventory->removeItem(inventory->selected,
                                      all && inventory->getSelected() != nullptr
                                          ? inventory->getSelected()->count
                                          : 1),
                false);
}

std::shared_ptr<ItemEntity> Player::drop(std::shared_ptr<ItemInstance> item) {
    return drop(item, false);
}

std::shared_ptr<ItemEntity> Player::drop(std::shared_ptr<ItemInstance> item,
                                         bool randomly) {
    if (item == nullptr) return nullptr;
    if (item->count == 0) return nullptr;

    std::shared_ptr<ItemEntity> thrownItem = std::shared_ptr<ItemEntity>(
        new ItemEntity(level, x, y - 0.3f + getHeadHeight(), z, item));
    thrownItem->throwTime = 20 * 2;

    thrownItem->setThrower(getName());

    float pow = 0.1f;
    if (randomly) {
        float _pow = random->nextFloat() * 0.5f;
        float dir = random->nextFloat() * std::numbers::pi * 2;
        thrownItem->xd = -sin(dir) * _pow;
        thrownItem->zd = cos(dir) * _pow;
        thrownItem->yd = 0.2f;

    } else {
        pow = 0.3f;
        thrownItem->xd = -sin(yRot / 180 * std::numbers::pi) *
                         cos(xRot / 180 * std::numbers::pi) * pow;
        thrownItem->zd = cos(yRot / 180 * std::numbers::pi) *
                         cos(xRot / 180 * std::numbers::pi) * pow;
        thrownItem->yd = -sin(xRot / 180 * std::numbers::pi) * pow + 0.1f;
        pow = 0.02f;

        float dir = random->nextFloat() * std::numbers::pi * 2;
        pow *= random->nextFloat();
        thrownItem->xd += cos(dir) * pow;
        thrownItem->yd += (random->nextFloat() - random->nextFloat()) * 0.1f;
        thrownItem->zd += sin(dir) * pow;
    }

    reallyDrop(thrownItem);

    return thrownItem;
}

void Player::reallyDrop(std::shared_ptr<ItemEntity> thrownItem) {
    level->addEntity(thrownItem);
}

float Player::getDestroySpeed(Tile* tile, bool hasProperTool) {
    float speed = inventory->getDestroySpeed(tile);

    if (speed > 1) {
        int efficiency = EnchantmentHelper::getDiggingBonus(
            std::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
        std::shared_ptr<ItemInstance> item = inventory->getSelected();

        if (efficiency > 0 && item != nullptr) {
            float boost = efficiency * efficiency + 1;

            if (item->canDestroySpecial(tile) || speed > 1) {
                speed += boost;
            } else {
                speed += boost * 0.08f;
            }
        }
    }

    if (hasEffect(MobEffect::digSpeed)) {
        speed *=
            1.0f + (getEffect(MobEffect::digSpeed)->getAmplifier() + 1) * .2f;
    }
    if (hasEffect(MobEffect::digSlowdown)) {
        speed *= 1.0f -
                 (getEffect(MobEffect::digSlowdown)->getAmplifier() + 1) * .2f;
    }

    if (isUnderLiquid(Material::water) &&
        !EnchantmentHelper::hasWaterWorkerBonus(
            std::dynamic_pointer_cast<LivingEntity>(shared_from_this())))
        speed /= 5;

    
    
    
    

    return speed;
}

bool Player::canDestroy(Tile* tile) { return inventory->canDestroy(tile); }

void Player::readAdditionalSaveData(CompoundTag* entityTag) {
    LivingEntity::readAdditionalSaveData(entityTag);
    ListTag<CompoundTag>* inventoryList =
        (ListTag<CompoundTag>*)entityTag->getList(L"Inventory");
    inventory->load(inventoryList);
    inventory->selected = entityTag->getInt(L"SelectedItemSlot");
    m_isSleeping = entityTag->getBoolean(L"Sleeping");
    sleepCounter = entityTag->getShort(L"SleepTimer");

    experienceProgress = entityTag->getFloat(L"XpP");
    experienceLevel = entityTag->getInt(L"XpLevel");
    totalExperience = entityTag->getInt(L"XpTotal");
    setScore(entityTag->getInt(L"Score"));

    if (m_isSleeping) {
        bedPosition = new Pos(Mth::floor(x), Mth::floor(y), Mth::floor(z));
        stopSleepInBed(true, true, false);
    }

    if (entityTag->contains(L"SpawnX") && entityTag->contains(L"SpawnY") &&
        entityTag->contains(L"SpawnZ")) {
        respawnPosition =
            new Pos(entityTag->getInt(L"SpawnX"), entityTag->getInt(L"SpawnY"),
                    entityTag->getInt(L"SpawnZ"));
        respawnForced = entityTag->getBoolean(L"SpawnForced");
    }

    foodData.readAdditionalSaveData(entityTag);
    abilities.loadSaveData(entityTag);

    if (entityTag->contains(L"EnderItems")) {
        ListTag<CompoundTag>* enderItemsList =
            (ListTag<CompoundTag>*)entityTag->getList(L"EnderItems");
        enderChestInventory->setItemsByTag(enderItemsList);
    }

    
    m_uiGamePrivileges = entityTag->getInt(L"GamePrivileges");
}

void Player::addAdditonalSaveData(CompoundTag* entityTag) {
    LivingEntity::addAdditonalSaveData(entityTag);
    entityTag->put(L"Inventory", inventory->save(new ListTag<CompoundTag>()));
    entityTag->putInt(L"SelectedItemSlot", inventory->selected);
    entityTag->putBoolean(L"Sleeping", m_isSleeping);
    entityTag->putShort(L"SleepTimer", (short)sleepCounter);

    entityTag->putFloat(L"XpP", experienceProgress);
    entityTag->putInt(L"XpLevel", experienceLevel);
    entityTag->putInt(L"XpTotal", totalExperience);
    entityTag->putInt(L"Score", getScore());

    if (respawnPosition != nullptr) {
        entityTag->putInt(L"SpawnX", respawnPosition->x);
        entityTag->putInt(L"SpawnY", respawnPosition->y);
        entityTag->putInt(L"SpawnZ", respawnPosition->z);
        entityTag->putBoolean(L"SpawnForced", respawnForced);
    }

    foodData.addAdditonalSaveData(entityTag);
    abilities.addSaveData(entityTag);

    entityTag->put(L"EnderItems", enderChestInventory->createTag());

    
    entityTag->putInt(L"GamePrivileges", m_uiGamePrivileges);
}

bool Player::openContainer(std::shared_ptr<Container> container) {
    return true;
}

bool Player::openHopper(std::shared_ptr<HopperTileEntity> container) {
    return true;
}

bool Player::openHopper(std::shared_ptr<MinecartHopper> container) {
    return true;
}

bool Player::openHorseInventory(std::shared_ptr<EntityHorse> horse,
                                std::shared_ptr<Container> container) {
    return true;
}

bool Player::startEnchanting(int x, int y, int z, const std::wstring& name) {
    return true;
}

bool Player::startRepairing(int x, int y, int z) { return true; }

bool Player::startCrafting(int x, int y, int z) { return true; }

bool Player::openFireworks(int x, int y, int z) { return true; }

float Player::getHeadHeight() { return 0.12f; }

void Player::setDefaultHeadHeight() { heightOffset = 1.62f; }

bool Player::hurt(DamageSource* source, float dmg) {
    if (isInvulnerable()) return false;
    if (hasInvulnerablePrivilege() ||
        (abilities.invulnerable && !source->isBypassInvul()))
        return false;

    
    
    if (source == DamageSource::fall && isAllowedToFly() && abilities.flying)
        return false;

    noActionTime = 0;
    if (getHealth() <= 0) return false;

    if (isSleeping() && !level->isClientSide) {
        stopSleepInBed(true, true, false);
    }

    if (source->scalesWithDifficulty()) {
        if (level->difficulty == Difficulty::PEACEFUL) dmg = 0;
        if (level->difficulty == Difficulty::EASY) dmg = dmg / 2 + 1;
        if (level->difficulty == Difficulty::HARD) dmg = dmg * 3 / 2;
    }

    if (dmg == 0) return false;

    std::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker != nullptr && attacker->instanceof(eTYPE_ARROW)) {
        std::shared_ptr<Arrow> arrow =
            std::dynamic_pointer_cast<Arrow>(attacker);
        if (arrow->owner != nullptr) {
            attacker = arrow->owner;
        }
    }

    return LivingEntity::hurt(source, dmg);
}

bool Player::canHarmPlayer(std::shared_ptr<Player> target) {
    Team* team = getTeam();
    Team* otherTeam = target->getTeam();

    if (team == nullptr) {
        return true;
    }
    if (!team->isAlliedTo(otherTeam)) {
        return true;
    }
    return team->isAllowFriendlyFire();
}

bool Player::canHarmPlayer(std::wstring targetName) { return true; }

void Player::hurtArmor(float damage) { inventory->hurtArmor(damage); }

int Player::getArmorValue() { return inventory->getArmorValue(); }

float Player::getArmorCoverPercentage() {
    int count = 0;
    for (int i = 0; i < inventory->armor.size(); i++) {
        if (inventory->armor[i] != nullptr) {
            count++;
        }
    }
    return (float)count / (float)inventory->armor.size();
}

void Player::actuallyHurt(DamageSource* source, float dmg) {
    if (isInvulnerable()) return;
    if (!source->isBypassArmor() && isBlocking() && dmg > 0) {
        dmg = (1 + dmg) * .5f;
    }
    dmg = getDamageAfterArmorAbsorb(source, dmg);
    dmg = getDamageAfterMagicAbsorb(source, dmg);

    float originalDamage = dmg;
    dmg = std::max(dmg - getAbsorptionAmount(), 0.0f);
    setAbsorptionAmount(getAbsorptionAmount() - (originalDamage - dmg));
    if (dmg == 0) return;

    causeFoodExhaustion(source->getFoodExhaustion());
    float oldHealth = getHealth();
    setHealth(getHealth() - dmg);
    getCombatTracker()->recordDamage(source, oldHealth, dmg);
}

bool Player::openFurnace(std::shared_ptr<FurnaceTileEntity> container) {
    return true;
}

bool Player::openTrap(std::shared_ptr<DispenserTileEntity> container) {
    return true;
}

void Player::openTextEdit(std::shared_ptr<TileEntity> sign) {}

bool Player::openBrewingStand(
    std::shared_ptr<BrewingStandTileEntity> brewingStand) {
    return true;
}

bool Player::openBeacon(std::shared_ptr<BeaconTileEntity> beacon) {
    return true;
}

bool Player::openTrading(std::shared_ptr<Merchant> traderTarget,
                         const std::wstring& name) {
    return true;
}






void Player::openItemInstanceGui(std::shared_ptr<ItemInstance> itemInstance) {}

bool Player::interact(std::shared_ptr<Entity> entity) {
    std::shared_ptr<Player> thisPlayer =
        std::dynamic_pointer_cast<Player>(shared_from_this());

    std::shared_ptr<ItemInstance> item = getSelectedItem();
    std::shared_ptr<ItemInstance> itemClone =
        (item != nullptr) ? item->copy() : nullptr;
    if (entity->interact(thisPlayer)) {
        
        
        
        if (item != nullptr && item == getSelectedItem()) {
            if (item->count <= 0 && !abilities.instabuild) {
                removeSelectedItem();
            } else if (item->count < itemClone->count && abilities.instabuild) {
                item->count = itemClone->count;
            }
        }
        return true;
    }

    if ((item != nullptr) && entity->instanceof(eTYPE_LIVINGENTITY)) {
        
        
        
        if (this->abilities.instabuild) item = itemClone;
        if (item->interactEnemy(
                thisPlayer, std::dynamic_pointer_cast<LivingEntity>(entity))) {
            
            
            
            if ((item->count <= 0) && !abilities.instabuild) {
                removeSelectedItem();
            }
            return true;
        }
    }
    return false;
}

std::shared_ptr<ItemInstance> Player::getSelectedItem() {
    return inventory->getSelected();
}

void Player::removeSelectedItem() {
    inventory->setItem(inventory->selected, nullptr);
}

double Player::getRidingHeight() { return heightOffset - 0.5f; }

void Player::attack(std::shared_ptr<Entity> entity) {
    if (!entity->isAttackable()) {
        return;
    }

    if (entity->skipAttackInteraction(shared_from_this())) {
        return;
    }

    float dmg =
        (float)getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue();

    int knockback = 0;
    float magicBoost = 0;

    if (entity->instanceof(eTYPE_LIVINGENTITY)) {
        std::shared_ptr<Player> thisPlayer =
            std::dynamic_pointer_cast<Player>(shared_from_this());
        std::shared_ptr<LivingEntity> mob =
            std::dynamic_pointer_cast<LivingEntity>(entity);
        magicBoost = EnchantmentHelper::getDamageBonus(thisPlayer, mob);
        knockback += EnchantmentHelper::getKnockbackBonus(thisPlayer, mob);
    }
    if (isSprinting()) {
        knockback += 1;
    }

    if (dmg > 0 || magicBoost > 0) {
        bool bCrit = fallDistance > 0 && !onGround && !onLadder() &&
                     !isInWater() && !hasEffect(MobEffect::blindness) &&
                     (riding == nullptr) &&
                     entity->instanceof(eTYPE_LIVINGENTITY);
        if (bCrit && dmg > 0) {
            dmg *= 1.5f;
        }
        dmg += magicBoost;

        
        
        bool setOnFireTemporatily = false;
        int fireAspect = EnchantmentHelper::getFireAspect(
            std::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
        if (entity->instanceof(eTYPE_MOB) && fireAspect > 0 &&
            !entity->isOnFire()) {
            setOnFireTemporatily = true;
            entity->setOnFire(1);
        }

        DamageSource* damageSource = DamageSource::playerAttack(
            std::dynamic_pointer_cast<Player>(shared_from_this()));
        bool wasHurt = entity->hurt(damageSource, dmg);
        delete damageSource;
        if (wasHurt) {
            if (knockback > 0) {
                entity->push(
                    -sinf(yRot * std::numbers::pi / 180) * knockback * .5f, 0.1,
                    cosf(yRot * std::numbers::pi / 180) * knockback * .5f);
                xd *= 0.6;
                zd *= 0.6;
                setSprinting(false);
            }

            if (bCrit) {
                crit(entity);
            }
            if (magicBoost > 0) {
                magicCrit(entity);
            }

            if (dmg >= 18) {
                awardStat(GenericStats::overkill(),
                          GenericStats::param_overkill(dmg));
            }
            setLastHurtMob(entity);

            if (entity->instanceof(eTYPE_LIVINGENTITY)) {
                std::shared_ptr<LivingEntity> mob =
                    std::dynamic_pointer_cast<LivingEntity>(entity);
                ThornsEnchantment::doThornsAfterAttack(shared_from_this(), mob,
                                                       random);
            }
        }

        std::shared_ptr<ItemInstance> item = getSelectedItem();
        std::shared_ptr<Entity> hurtTarget = entity;
        if (entity->instanceof(eTYPE_MULTIENTITY_MOB_PART)) {
            std::shared_ptr<Entity> multiMob =
                std::dynamic_pointer_cast<Entity>(
                    (std::dynamic_pointer_cast<MultiEntityMobPart>(entity))
                        ->parentMob.lock());
            if ((multiMob != nullptr) &&
                multiMob->instanceof(eTYPE_LIVINGENTITY)) {
                hurtTarget = std::dynamic_pointer_cast<LivingEntity>(multiMob);
            }
        }
        if ((item != nullptr) && hurtTarget->instanceof(eTYPE_LIVINGENTITY)) {
            item->hurtEnemy(
                std::dynamic_pointer_cast<LivingEntity>(hurtTarget),
                std::dynamic_pointer_cast<Player>(shared_from_this()));
            if (item->count <= 0) {
                removeSelectedItem();
            }
        }
        if (entity->instanceof(eTYPE_LIVINGENTITY)) {
            

            if (fireAspect > 0 && wasHurt) {
                entity->setOnFire(fireAspect * 4);
            } else if (setOnFireTemporatily) {
                entity->clearFire();
            }
        }

        causeFoodExhaustion(FoodConstants::EXHAUSTION_ATTACK);
    }

    
    
    
    
    
}

void Player::crit(std::shared_ptr<Entity> entity) {}

void Player::magicCrit(std::shared_ptr<Entity> entity) {}

void Player::respawn() { deathFadeCounter = 0; }

void Player::animateRespawn(std::shared_ptr<Player> player, Level* level) {
    for (int i = 0; i < 45; i++) {
        float angle = i * std::numbers::pi * 4.0f / 25.0f;
        float xo = cosf(angle) * 0.7f;
        float zo = sinf(angle) * 0.7f;

        level->addParticle(eParticleType_netherportal, player->x + xo,
                           player->y - player->heightOffset + 1.62f - i * .05f,
                           player->z + zo, 0, 0, 0);
    }
}

Slot* Player::getInventorySlot(int slotId) { return nullptr; }

void Player::remove() {
    LivingEntity::remove();
    inventoryMenu->removed(
        std::dynamic_pointer_cast<Player>(shared_from_this()));
    if (containerMenu != nullptr) {
        containerMenu->removed(
            std::dynamic_pointer_cast<Player>(shared_from_this()));
    }
}

bool Player::isInWall() { return !m_isSleeping && LivingEntity::isInWall(); }

bool Player::isLocalPlayer() { return false; }

Player::BedSleepingResult Player::startSleepInBed(int x, int y, int z,
                                                  bool bTestUse) {
    if (!level->isClientSide || bTestUse) {
        if (isSleeping() || !isAlive()) {
            return OTHER_PROBLEM;
        }

        if (!level->dimension->isNaturalDimension()) {
            
            return NOT_POSSIBLE_HERE;
        }

        
        
        
        

        if (abs(this->x - x) > 3 || abs(this->y - y) > 2 ||
            abs(this->z - z) > 3) {
            
            return TOO_FAR_AWAY;
        }

        if (!bTestUse) {
            

            double hRange = 8;
            double vRange = 5;
            AABB monster_bb =
                AABB(x, y, z, x, y, z).grow(hRange, vRange, hRange);
            std::vector<std::shared_ptr<Entity>>* monsters =
                level->getEntitiesOfClass(typeid(Monster), &monster_bb);
            if (!monsters->empty()) {
                delete monsters;
                return NOT_SAFE;
            }
            delete monsters;
        }

        
        
        if (!bTestUse && level->isDay()) {
            
            return NOT_POSSIBLE_NOW;
        }
    }

    if (bTestUse) {
        
        
        return OK;
    }

    if (isRiding()) {
        ride(nullptr);
    }

    setSize(0.2f, 0.2f);
    heightOffset = .2f;
    if (level->hasChunkAt(x, y, z)) {
        int data = level->getData(x, y, z);
        int direction = BedTile::getDirection(data);
        float xo = .5f, zo = .5f;

        switch (direction) {
            case Direction::SOUTH:
                zo = .9f;
                break;
            case Direction::NORTH:
                zo = .1f;
                break;
            case Direction::WEST:
                xo = .1f;
                break;
            case Direction::EAST:
                xo = .9f;
                break;
        }
        setBedOffset(direction);
        setPos(x + xo, y + 15.0f / 16.0f, z + zo);
    } else {
        setPos(x + .5f, y + 15.0f / 16.0f, z + .5f);
    }
    m_isSleeping = true;
    sleepCounter = 0;
    bedPosition = new Pos(x, y, z);
    xd = zd = yd = 0;

    if (!level->isClientSide) {
        level->updateSleepingPlayerList();
    }

    return OK;
}

void Player::setBedOffset(int bedDirection) {
    
    bedOffsetX = 0;
    bedOffsetZ = 0;

    switch (bedDirection) {
        case Direction::SOUTH:
            bedOffsetZ = -1.8f;
            break;
        case Direction::NORTH:
            bedOffsetZ = 1.8f;
            break;
        case Direction::WEST:
            bedOffsetX = 1.8f;
            break;
        case Direction::EAST:
            bedOffsetX = -1.8f;
            break;
    }
}













void Player::stopSleepInBed(bool forcefulWakeUp, bool updateLevelList,
                            bool saveRespawnPoint) {
    setSize(0.6f, 1.8f);
    setDefaultHeadHeight();

    Pos* pos = bedPosition;
    Pos* standUp = bedPosition;
    if (pos != nullptr &&
        level->getTile(pos->x, pos->y, pos->z) == Tile::bed_Id) {
        BedTile::setOccupied(level, pos->x, pos->y, pos->z, false);

        standUp =
            BedTile::findStandUpPosition(level, pos->x, pos->y, pos->z, 0);
        if (standUp == nullptr) {
            standUp = new Pos(pos->x, pos->y + 1, pos->z);
        }
        setPos(standUp->x + .5f, standUp->y + heightOffset + .1f,
               standUp->z + .5f);
    }

    m_isSleeping = false;
    if (!level->isClientSide && updateLevelList) {
        level->updateSleepingPlayerList();
    }
    if (forcefulWakeUp) {
        sleepCounter = 0;
    } else {
        sleepCounter = SLEEP_DURATION;
    }
    if (saveRespawnPoint) {
        setRespawnPosition(bedPosition, false);
    }
}

bool Player::checkBed() {
    return (level->getTile(bedPosition->x, bedPosition->y, bedPosition->z) ==
            Tile::bed_Id);
}

Pos* Player::checkBedValidRespawnPosition(Level* level, Pos* pos, bool forced) {
    
    ChunkSource* chunkSource = level->getChunkSource();
    chunkSource->create((pos->x - 3) >> 4, (pos->z - 3) >> 4);
    chunkSource->create((pos->x + 3) >> 4, (pos->z - 3) >> 4);
    chunkSource->create((pos->x - 3) >> 4, (pos->z + 3) >> 4);
    chunkSource->create((pos->x + 3) >> 4, (pos->z + 3) >> 4);

    
    if (level->getTile(pos->x, pos->y, pos->z) != Tile::bed_Id) {
        Material* bottomMaterial = level->getMaterial(pos->x, pos->y, pos->z);
        Material* topMaterial = level->getMaterial(pos->x, pos->y + 1, pos->z);
        bool freeFeet =
            !bottomMaterial->isSolid() && !bottomMaterial->isLiquid();
        bool freeHead = !topMaterial->isSolid() && !topMaterial->isLiquid();

        if (forced && freeFeet && freeHead) {
            return pos;
        }
        return nullptr;
    }
    
    Pos* standUp =
        BedTile::findStandUpPosition(level, pos->x, pos->y, pos->z, 0);
    return standUp;
}

float Player::getSleepRotation() {
    if (bedPosition != nullptr) {
        int data =
            level->getData(bedPosition->x, bedPosition->y, bedPosition->z);
        int direction = BedTile::getDirection(data);

        switch (direction) {
            case Direction::SOUTH:
                return 90;
            case Direction::WEST:
                return 0;
            case Direction::NORTH:
                return 270;
            case Direction::EAST:
                return 180;
        }
    }
    return 0;
}

bool Player::isSleeping() { return m_isSleeping; }

bool Player::isSleepingLongEnough() {
    return m_isSleeping && sleepCounter >= SLEEP_DURATION;
}

int Player::getSleepTimer() { return sleepCounter; }


int Player::getDeathFadeTimer() { return deathFadeCounter; }

bool Player::getPlayerFlag(int flag) {
    return (entityData->getByte(DATA_PLAYER_FLAGS_ID) & (1 << flag)) != 0;
}

void Player::setPlayerFlag(int flag, bool value) {
    uint8_t currentValue = entityData->getByte(DATA_PLAYER_FLAGS_ID);
    if (value) {
        entityData->set(DATA_PLAYER_FLAGS_ID,
                        (uint8_t)(currentValue | (1 << flag)));
    } else {
        entityData->set(DATA_PLAYER_FLAGS_ID,
                        (uint8_t)(currentValue & ~(1 << flag)));
    }
}






void Player::displayClientMessage(int messageId) {}

Pos* Player::getRespawnPosition() { return respawnPosition; }

bool Player::isRespawnForced() { return respawnForced; }

void Player::setRespawnPosition(Pos* respawnPosition, bool forced) {
    if (respawnPosition != nullptr) {
        this->respawnPosition = new Pos(*respawnPosition);
        respawnForced = forced;
    } else {
        this->respawnPosition = nullptr;
        respawnForced = false;
    }
}

void Player::awardStat(Stat* stat, const std::vector<uint8_t>& paramBlob) {
    
}

void Player::jumpFromGround() {
    LivingEntity::jumpFromGround();

    
    

    if (isSprinting()) {
        causeFoodExhaustion(FoodConstants::EXHAUSTION_SPRINT_JUMP);
    } else {
        causeFoodExhaustion(FoodConstants::EXHAUSTION_JUMP);
    }
}

void Player::travel(float xa, float ya) {
    double preX = x, preY = y, preZ = z;

    if (abilities.flying && riding == nullptr) {
        double ydo = yd;
        float ofs = flyingSpeed;
        flyingSpeed = abilities.getFlyingSpeed();
        LivingEntity::travel(xa, ya);
        yd = ydo * 0.6;
        flyingSpeed = ofs;
    } else {
        LivingEntity::travel(xa, ya);
    }

    checkMovementStatistiscs(x - preX, y - preY, z - preZ);
}

float Player::getSpeed() {
    return (float)getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)
        ->getValue();
}

void Player::checkMovementStatistiscs(double dx, double dy, double dz) {
    if (riding != nullptr) {
        return;
    }
    if (isUnderLiquid(Material::water)) {
        int distance =
            (int)Math::round(sqrt(dx * dx + dy * dy + dz * dz) * 100.0f);
        if (distance > 0) {
            
            causeFoodExhaustion(FoodConstants::EXHAUSTION_SWIM * distance *
                                .01f);
        }
    } else if (isInWater()) {
        int horizontalDistance =
            (int)Math::round(sqrt(dx * dx + dz * dz) * 100.0f);
        if (horizontalDistance > 0) {
            distanceSwim += horizontalDistance;
            if (distanceSwim >= 100) {
                int newDistance = distanceSwim - (distanceSwim % 100);
                distanceSwim -= newDistance;
                awardStat(GenericStats::swimOneM(),
                          GenericStats::param_swim(newDistance / 100));
            }
            causeFoodExhaustion(FoodConstants::EXHAUSTION_SWIM *
                                horizontalDistance * .01f);
        }
    } else if (onLadder()) {
        if (dy > 0) {
            distanceClimb += (int)Math::round(dy * 100.0f);
            if (distanceClimb >= 100) {
                int newDistance = distanceClimb - (distanceClimb % 100);
                distanceClimb -= newDistance;
                awardStat(GenericStats::climbOneM(),
                          GenericStats::param_climb(newDistance / 100));
            }
        }
    } else if (onGround) {
        int horizontalDistance =
            (int)Math::round(sqrt(dx * dx + dz * dz) * 100.0f);
        if (horizontalDistance > 0) {
            distanceWalk += horizontalDistance;
            if (distanceWalk >= 100) {
                int newDistance = distanceWalk - (distanceWalk % 100);
                distanceWalk -= newDistance;
                awardStat(GenericStats::walkOneM(),
                          GenericStats::param_walk(newDistance / 100));
            }
            if (isSprinting()) {
                causeFoodExhaustion(FoodConstants::EXHAUSTION_SPRINT *
                                    horizontalDistance * .01f);
            } else {
                causeFoodExhaustion(FoodConstants::EXHAUSTION_WALK *
                                    horizontalDistance * .01f);
            }
        }
    }
}

void Player::checkRidingStatistiscs(double dx, double dy, double dz) {
    if (riding != nullptr) {
        int distance =
            (int)Math::round(sqrt(dx * dx + dy * dy + dz * dz) * 100.0f);
        if (distance > 0) {
            if (riding->instanceof(eTYPE_MINECART)) {
                distanceMinecart += distance;
                if (distanceMinecart >= 100) {
                    int newDistance =
                        distanceMinecart - (distanceMinecart % 100);
                    distanceMinecart -= newDistance;
                    awardStat(GenericStats::minecartOneM(),
                              GenericStats::param_minecart(newDistance / 100));
                }

                int dist = 0;
                if (minecartAchievementPos == nullptr) {
                    minecartAchievementPos =
                        new Pos(Mth::floor(x), Mth::floor(y), Mth::floor(z));
                }
                
                
                else {
                    
                    
                    int dist = minecartAchievementPos->dist(
                        Mth::floor(x), Mth::floor(y), Mth::floor(z));
                    if ((m_bAwardedOnARail == false) && (dist >= 500)) {
                        awardStat(GenericStats::onARail(),
                                  GenericStats::param_onARail(dist));
                        m_bAwardedOnARail = true;
                    }
                }

            } else if (riding->instanceof(eTYPE_BOAT)) {
                distanceBoat += distance;
                if (distanceBoat >= 100) {
                    int newDistance = distanceBoat - (distanceBoat % 100);
                    distanceBoat -= newDistance;
                    awardStat(GenericStats::boatOneM(),
                              GenericStats::param_boat(newDistance / 100));
                }
            } else if (riding->instanceof(eTYPE_PIG)) {
                distancePig += distance;
                if (distancePig >= 100) {
                    int newDistance = distancePig - (distancePig % 100);
                    distancePig -= newDistance;
                    awardStat(GenericStats::pigOneM(),
                              GenericStats::param_pig(newDistance / 100));
                }
            }
        }
    }
}

void Player::causeFallDamage(float distance) {
    if (abilities.mayfly) return;

    if (distance >= 2) {
        distanceFall += (int)Math::round(distance * 100.0);
        if (distanceFall >= 100) {
            int newDistance = distanceFall - (distanceFall % 100);
            distanceFall -= newDistance;
            awardStat(GenericStats::fallOneM(),
                      GenericStats::param_fall(newDistance / 100));
        }
    }
    LivingEntity::causeFallDamage(distance);
}

void Player::killed(std::shared_ptr<LivingEntity> mob) {
    
    
    
    if (mob->instanceof(eTYPE_ENEMY) || mob->GetType() == eTYPE_GHAST ||
        mob->GetType() == eTYPE_SLIME || mob->GetType() == eTYPE_LAVASLIME ||
        mob->GetType() == eTYPE_ENDERDRAGON) {
        awardStat(GenericStats::killEnemy(), GenericStats::param_noArgs());

        switch (mob->GetType()) {
            case eTYPE_CREEPER:
                awardStat(GenericStats::killsCreeper(),
                          GenericStats::param_noArgs());
                break;
            case eTYPE_SKELETON:
                if (mob->isRiding() && mob->riding->GetType() == eTYPE_SPIDER)
                    awardStat(GenericStats::killsSpiderJockey(),
                              GenericStats::param_noArgs());
                else
                    awardStat(GenericStats::killsSkeleton(),
                              GenericStats::param_noArgs());
                break;
            case eTYPE_SPIDER:
                if (mob->rider.lock() != nullptr &&
                    mob->rider.lock()->GetType() == eTYPE_SKELETON)
                    awardStat(GenericStats::killsSpiderJockey(),
                              GenericStats::param_noArgs());
                else
                    awardStat(GenericStats::killsSpider(),
                              GenericStats::param_noArgs());
                break;
            case eTYPE_ZOMBIE:
                awardStat(GenericStats::killsZombie(),
                          GenericStats::param_noArgs());
                break;
            case eTYPE_PIGZOMBIE:
                if (level->dimension->id == 0)
                    awardStat(GenericStats::killsZombiePigman(),
                              GenericStats::param_noArgs());
                else
                    awardStat(GenericStats::killsNetherZombiePigman(),
                              GenericStats::param_noArgs());
                break;
            case eTYPE_GHAST:
                awardStat(GenericStats::killsGhast(),
                          GenericStats::param_noArgs());
                break;
            case eTYPE_SLIME:
                awardStat(GenericStats::killsSlime(),
                          GenericStats::param_noArgs());
                break;
            case eTYPE_ENDERDRAGON:
                awardStat(GenericStats::killsEnderdragon(),
                          GenericStats::param_noArgs());
                break;
            default:
                break;
        }
    } else if (mob->GetType() == eTYPE_COW) {
        awardStat(GenericStats::killCow(), GenericStats::param_noArgs());
    }
}

void Player::makeStuckInWeb() {
    if (!abilities.flying) LivingEntity::makeStuckInWeb();
}

Icon* Player::getItemInHandIcon(std::shared_ptr<ItemInstance> item, int layer) {
    Icon* icon = LivingEntity::getItemInHandIcon(item, layer);
    if (item->id == Item::fishingRod->id && fishing != nullptr) {
        icon = Item::fishingRod->getEmptyIcon();
    } else if (item->getItem()->hasMultipleSpriteLayers()) {
        return item->getItem()->getLayerIcon(item->getAuxValue(), layer);
    } else if (useItem != nullptr && item->id == Item::bow_Id) {
        int ticksHeld = (item->getUseDuration() - useItemDuration);
        if (ticksHeld >= BowItem::MAX_DRAW_DURATION - 2) {
            return Item::bow->getDrawnIcon(2);
        }
        if (ticksHeld > (2 * BowItem::MAX_DRAW_DURATION) / 3) {
            return Item::bow->getDrawnIcon(1);
        }
        if (ticksHeld > 0) {
            return Item::bow->getDrawnIcon(0);
        }
    }
    return icon;
}

std::shared_ptr<ItemInstance> Player::getArmor(int pos) {
    return inventory->getArmor(pos);
}

void Player::increaseXp(int i) {
    increaseScore(i);
    int max = INT_MAX - totalExperience;
    if (i > max) {
        i = max;
    }
    experienceProgress += (float)i / getXpNeededForNextLevel();
    totalExperience += i;
    while (experienceProgress >= 1) {
        experienceProgress =
            (experienceProgress - 1) * getXpNeededForNextLevel();
        giveExperienceLevels(1);
        experienceProgress /= getXpNeededForNextLevel();
    }
}

void Player::giveExperienceLevels(int amount) {
    experienceLevel += amount;
    if (experienceLevel < 0) {
        experienceLevel = 0;
        experienceProgress = 0;
        totalExperience = 0;
    }

    if (amount > 0 && experienceLevel % 5 == 0 &&
        lastLevelUpTime <
            tickCount - SharedConstants::TICKS_PER_SECOND * 5.0f) {
        float vol = experienceLevel > 30 ? 1 : experienceLevel / 30.0f;
        level->playEntitySound(shared_from_this(), eSoundType_RANDOM_LEVELUP,
                               vol * 0.75f, 1);
        lastLevelUpTime = tickCount;
    }
}

int Player::getXpNeededForNextLevel() {
    
    if (experienceLevel >= 30) {
        return 17 + 15 * 3 + (experienceLevel - 30) * 7;
    }
    if (experienceLevel >= 15) {
        return 17 + (experienceLevel - 15) * 3;
    }
    return 17;
}










void Player::causeFoodExhaustion(float amount) {
    if (isAllowedToIgnoreExhaustion() || (isAllowedToFly() && abilities.flying))
        return;
    if (abilities.invulnerable || hasInvulnerablePrivilege()) return;

    
    
    if (app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0 &&
        getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotBuild) != 0)
        return;

    if (!level->isClientSide) {
        foodData.addExhaustion(amount);
    }
}

FoodData* Player::getFoodData() { return &foodData; }

bool Player::canEat(bool magicalItem) {
    return (magicalItem || foodData.needsFood()) && !abilities.invulnerable &&
           !hasInvulnerablePrivilege();
}

bool Player::isHurt() {
    return getHealth() > 0 && getHealth() < getMaxHealth();
}

void Player::startUsingItem(std::shared_ptr<ItemInstance> instance,
                            int duration) {
    if (instance == useItem) return;
    useItem = instance;
    useItemDuration = duration;
    if (!level->isClientSide) {
        setUsingItemFlag(true);
    }

    
    awardStat(
        GenericStats::itemsUsed(instance->getItem()->id),
        GenericStats::param_itemsUsed(
            std::dynamic_pointer_cast<Player>(shared_from_this()), instance));

#if defined(_EXTENDED_ACHIEVEMENTS)
    if ((instance->getItem()->id == Item::rotten_flesh_Id) &&
        (getFoodData()->getFoodLevel() == 0))
        awardStat(GenericStats::ironBelly(), GenericStats::param_ironBelly());
#endif
}

bool Player::mayDestroyBlockAt(int x, int y, int z) {
    if (abilities.mayBuild) {
        return true;
    }
    int t = level->getTile(x, y, z);
    if (t > 0) {
        Tile* tile = Tile::tiles[t];

        if (tile->material->isDestroyedByHand()) {
            return true;
        } else if (getSelectedItem() != nullptr) {
            std::shared_ptr<ItemInstance> carried = getSelectedItem();

            if (carried->canDestroySpecial(tile) ||
                carried->getDestroySpeed(tile) > 1) {
                return true;
            }
        }
    }
    return false;
}

bool Player::mayUseItemAt(int x, int y, int z, int face,
                          std::shared_ptr<ItemInstance> item) {
    if (abilities.mayBuild) {
        return true;
    }
    if (item != nullptr) {
        return item->mayBePlacedInAdventureMode();
    }
    return false;
}

int Player::getExperienceReward(std::shared_ptr<Player> killedBy) {
    if (level->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY))
        return 0;
    int reward = experienceLevel * 7;
    if (reward > 100) {
        return 100;
    }
    return reward;
}

bool Player::isAlwaysExperienceDropper() {
    
    return true;
}

std::wstring Player::getAName() { return name; }

bool Player::shouldShowName() { return true; }

void Player::restoreFrom(std::shared_ptr<Player> oldPlayer, bool restoreAll) {
    if (restoreAll) {
        inventory->replaceWith(oldPlayer->inventory);

        setHealth(oldPlayer->getHealth());
        foodData = oldPlayer->foodData;

        experienceLevel = oldPlayer->experienceLevel;
        totalExperience = oldPlayer->totalExperience;
        experienceProgress = oldPlayer->experienceProgress;

        setScore(oldPlayer->getScore());
        portalEntranceDir = oldPlayer->portalEntranceDir;
    } else if (level->getGameRules()->getBoolean(
                   GameRules::RULE_KEEPINVENTORY)) {
        inventory->replaceWith(oldPlayer->inventory);

        experienceLevel = oldPlayer->experienceLevel;
        totalExperience = oldPlayer->totalExperience;
        experienceProgress = oldPlayer->experienceProgress;
        setScore(oldPlayer->getScore());
    }
    enderChestInventory = oldPlayer->enderChestInventory;
}

bool Player::makeStepSound() { return !abilities.flying; }

void Player::onUpdateAbilities() {}

void Player::setGameMode(GameType* mode) {}

std::wstring Player::getName() { return name; }

std::wstring Player::getDisplayName() {
    
    const std::wstring& base = m_displayName.size() > 0 ? m_displayName : name;
    Scoreboard* scoreboard =
        level != nullptr ? level->getScoreboard() : nullptr;
    if (scoreboard == nullptr) return base;

    
    
    
    
    if (scoreboard->getRevision() != m_teamNameRevision) {
        m_teamNameRevision = scoreboard->getRevision();
        m_teamFormattedName = PlayerTeam::formatNameForTeam(
            scoreboard->getPlayersTeam(name), base);
    }
    return m_teamFormattedName;
}

std::wstring Player::getNetworkName() {
    
    return name;
}

Level* Player::getCommandSenderWorld() { return level; }

std::shared_ptr<PlayerEnderChestContainer> Player::getEnderChestInventory() {
    return enderChestInventory;
}

std::shared_ptr<ItemInstance> Player::getCarried(int slot) {
    if (slot == 0) return inventory->getSelected();
    return inventory->armor[slot - 1];
}

std::shared_ptr<ItemInstance> Player::getCarriedItem() {
    return inventory->getSelected();
}

void Player::setEquippedSlot(int slot, std::shared_ptr<ItemInstance> item) {
    inventory->armor[slot] = item;
}

bool Player::isInvisibleTo(std::shared_ptr<Player> player) {
    return isInvisible();
}

std::vector<std::shared_ptr<ItemInstance>> Player::getEquipmentSlots() {
    return inventory->armor;
}

bool Player::isCapeHidden() { return getPlayerFlag(FLAG_HIDE_CAPE); }

bool Player::isPushedByWater() { return !abilities.flying; }

Scoreboard* Player::getScoreboard() { return level->getScoreboard(); }

Team* Player::getTeam() { return getScoreboard()->getPlayersTeam(name); }

void Player::setAbsorptionAmount(float absorptionAmount) {
    if (absorptionAmount < 0) absorptionAmount = 0;
    getEntityData()->set(DATA_PLAYER_ABSORPTION_ID, absorptionAmount);
}

float Player::getAbsorptionAmount() {
    return getEntityData()->getFloat(DATA_PLAYER_ABSORPTION_ID);
}

int Player::getTexture() {
    switch (m_skinIndex) {
        case EDefaultSkins::Skin0:
            return TN_MOB_CHAR;  
        case EDefaultSkins::Skin1:
            return TN_MOB_CHAR1;  
        case EDefaultSkins::Skin2:
            return TN_MOB_CHAR2;  
        case EDefaultSkins::Skin3:
            return TN_MOB_CHAR3;  
        case EDefaultSkins::Skin4:
            return TN_MOB_CHAR4;  
        case EDefaultSkins::Skin5:
            return TN_MOB_CHAR5;  
        case EDefaultSkins::Skin6:
            return TN_MOB_CHAR6;  
        case EDefaultSkins::Skin7:
            return TN_MOB_CHAR7;  

        default:
            return TN_MOB_CHAR;  
    }
}

int Player::hash_fnct(const std::shared_ptr<Player> k) {
    
    return (int)std::hash<std::wstring>()(k->name);
}

bool Player::eq_test(const std::shared_ptr<Player> x,
                     const std::shared_ptr<Player> y) {
    
    
    return x->name.compare(y->name) ==
           0;  
}

unsigned int Player::getPlayerGamePrivilege(EPlayerGamePrivileges privilege) {
    return Player::getPlayerGamePrivilege(m_uiGamePrivileges, privilege);
}

unsigned int Player::getPlayerGamePrivilege(unsigned int uiGamePrivileges,
                                            EPlayerGamePrivileges privilege) {
    if (privilege == ePlayerGamePrivilege_All) {
        return uiGamePrivileges;
    } else if (privilege < ePlayerGamePrivilege_MAX) {
        return uiGamePrivileges & (1 << privilege);
    }
    return 0;
}

void Player::setPlayerGamePrivilege(EPlayerGamePrivileges privilege,
                                    unsigned int value) {
    Player::setPlayerGamePrivilege(m_uiGamePrivileges, privilege, value);
}

void Player::setPlayerGamePrivilege(unsigned int& uiGamePrivileges,
                                    EPlayerGamePrivileges privilege,
                                    unsigned int value) {
    if (privilege == ePlayerGamePrivilege_All) {
        uiGamePrivileges = value;
    } else if (privilege == ePlayerGamePrivilege_HOST) {
        if (value == 0) {
            Player::setPlayerGamePrivilege(uiGamePrivileges,
                                           ePlayerGamePrivilege_Op, 0);
        } else {
            Player::setPlayerGamePrivilege(uiGamePrivileges,
                                           ePlayerGamePrivilege_Op, 1);
            Player::setPlayerGamePrivilege(
                uiGamePrivileges, ePlayerGamePrivilege_CanToggleInvisible, 1);
            Player::setPlayerGamePrivilege(
                uiGamePrivileges, ePlayerGamePrivilege_CanToggleFly, 1);
            Player::setPlayerGamePrivilege(
                uiGamePrivileges, ePlayerGamePrivilege_CanToggleClassicHunger,
                1);
            Player::setPlayerGamePrivilege(uiGamePrivileges,
                                           ePlayerGamePrivilege_CanTeleport, 1);
        }
    } else if (privilege < ePlayerGamePrivilege_MAX) {
        if (value != 0) {
            uiGamePrivileges |= (1 << privilege);
        } else {
            
            switch (privilege) {
                case ePlayerGamePrivilege_CanToggleInvisible:
                    Player::setPlayerGamePrivilege(
                        uiGamePrivileges, ePlayerGamePrivilege_Invisible, 0);
                    Player::setPlayerGamePrivilege(
                        uiGamePrivileges, ePlayerGamePrivilege_Invulnerable, 0);
                    break;
                case ePlayerGamePrivilege_CanToggleFly:
                    Player::setPlayerGamePrivilege(
                        uiGamePrivileges, ePlayerGamePrivilege_CanFly, 0);
                    break;
                case ePlayerGamePrivilege_CanToggleClassicHunger:
                    Player::setPlayerGamePrivilege(
                        uiGamePrivileges, ePlayerGamePrivilege_ClassicHunger,
                        0);
                    break;
                case ePlayerGamePrivilege_Op:
                    Player::setPlayerGamePrivilege(
                        uiGamePrivileges,
                        ePlayerGamePrivilege_CanToggleInvisible, 0);
                    Player::setPlayerGamePrivilege(
                        uiGamePrivileges, ePlayerGamePrivilege_CanToggleFly, 0);
                    Player::setPlayerGamePrivilege(
                        uiGamePrivileges,
                        ePlayerGamePrivilege_CanToggleClassicHunger, 0);
                    Player::setPlayerGamePrivilege(
                        uiGamePrivileges, ePlayerGamePrivilege_CanTeleport, 0);
                    break;
                default:
                    break;
            }
            
            uiGamePrivileges &= ~(1 << privilege);
        }
    }
}

bool Player::isAllowedToUse(Tile* tile) {
    bool allowed = true;
    if (tile != nullptr &&
        app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) {
        allowed = false;

        if (getPlayerGamePrivilege(
                Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches) != 0) {
            switch (tile->id) {
                case Tile::door_wood_Id:
                case Tile::button_stone_Id:
                case Tile::button_wood_Id:
                case Tile::lever_Id:
                case Tile::fenceGate_Id:
                case Tile::trapdoor_Id:
                    allowed = true;
                    break;
                default:
                    break;
            }
        }

        if (getPlayerGamePrivilege(
                Player::ePlayerGamePrivilege_CanUseContainers) != 0) {
            switch (tile->id) {
                case Tile::chest_Id:
                case Tile::furnace_Id:
                case Tile::furnace_lit_Id:
                case Tile::dispenser_Id:
                case Tile::brewingStand_Id:
                case Tile::enchantTable_Id:
                case Tile::workBench_Id:
                case Tile::anvil_Id:
                case Tile::enderChest_Id:
                    allowed = true;
                    break;
                default:
                    break;
            }
        }

        if (!allowed && getPlayerGamePrivilege(
                            Player::ePlayerGamePrivilege_CannotBuild) == 0) {
            switch (tile->id) {
                case Tile::door_wood_Id:
                case Tile::button_stone_Id:
                case Tile::button_wood_Id:
                case Tile::lever_Id:
                case Tile::fenceGate_Id:
                case Tile::trapdoor_Id:
                case Tile::chest_Id:
                case Tile::furnace_Id:
                case Tile::furnace_lit_Id:
                case Tile::dispenser_Id:
                case Tile::brewingStand_Id:
                case Tile::enchantTable_Id:
                case Tile::workBench_Id:
                case Tile::anvil_Id:
                case Tile::enderChest_Id:
                    allowed = false;
                    break;
                default:
                    allowed = true;
                    break;
            }
        }
    }

    return allowed;
}

bool Player::isAllowedToUse(std::shared_ptr<ItemInstance> item) {
    bool allowed = true;
    if (item != nullptr &&
        app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) {
        if (getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotBuild) !=
            0) {
            allowed = false;
        }

        
        
        switch (item->id) {
                
            case Item::mushroomStew_Id:
            case Item::apple_Id:
            case Item::bread_Id:
            case Item::porkChop_raw_Id:
            case Item::porkChop_cooked_Id:
            case Item::apple_gold_Id:
            case Item::fish_raw_Id:
            case Item::fish_cooked_Id:
            case Item::cookie_Id:
            case Item::beef_cooked_Id:
            case Item::beef_raw_Id:
            case Item::chicken_cooked_Id:
            case Item::chicken_raw_Id:
            case Item::melon_Id:
            case Item::rotten_flesh_Id:
                
            case Item::bow_Id:
            case Item::sword_diamond_Id:
            case Item::sword_gold_Id:
            case Item::sword_iron_Id:
            case Item::sword_stone_Id:
            case Item::sword_wood_Id:
                allowed = true;
                break;
            default:
                break;
        }
    }

    return allowed;
}

bool Player::isAllowedToInteract(std::shared_ptr<Entity> target) {
    bool allowed = true;
    if (app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) {
        if (target->instanceof(eTYPE_MINECART)) {
            if (getPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_CanUseContainers) == 0) {
                std::shared_ptr<Minecart> minecart =
                    std::dynamic_pointer_cast<Minecart>(target);
                if (minecart->getType() == Minecart::TYPE_CHEST)
                    allowed = false;
            }

        } else {
            if (getPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_CannotBuild) != 0) {
                allowed = false;
            }

            if (getPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_CannotMine) != 0) {
                allowed = false;
            }
        }
    }

    return allowed;
}

bool Player::isAllowedToMine() {
    bool allowed = true;
    if (app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) {
        if (getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotMine) !=
            0) {
            allowed = false;
        }
    }
    return allowed;
}

bool Player::isAllowedToAttackPlayers() {
    bool allowed = true;
    if (hasInvisiblePrivilege() ||
        ((app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) &&
         getPlayerGamePrivilege(
             Player::ePlayerGamePrivilege_CannotAttackPlayers))) {
        allowed = false;
    }
    return allowed;
}

bool Player::isAllowedToAttackAnimals() {
    bool allowed = true;
    if ((app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) &&
        getPlayerGamePrivilege(
            Player::ePlayerGamePrivilege_CannotAttackAnimals)) {
        allowed = false;
    }
    return allowed;
}

bool Player::isAllowedToHurtEntity(std::shared_ptr<Entity> target) {
    bool allowed = true;

    if (!isAllowedToMine()) {
        switch (target->GetType()) {
            case eTYPE_HANGING_ENTITY:
            case eTYPE_PAINTING:
            case eTYPE_ITEM_FRAME:

                
                
                
            case eTYPE_BOAT:
            case eTYPE_MINECART:

                allowed = false;
                break;
            default:
                break;
        };
    }
    return allowed;
}

bool Player::isAllowedToFly() {
    bool allowed = false;
    if (app.GetGameHostOption(eGameHostOption_HostCanFly) != 0 &&
        getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanFly) != 0) {
        allowed = true;
    }
    return allowed;
}

bool Player::isAllowedToIgnoreExhaustion() {
    bool allowed = false;
    if ((app.GetGameHostOption(eGameHostOption_HostCanChangeHunger) != 0 &&
         getPlayerGamePrivilege(Player::ePlayerGamePrivilege_ClassicHunger) !=
             0) ||
        (isAllowedToFly() && abilities.flying)) {
        allowed = true;
    }
    return allowed;
}

bool Player::isAllowedToTeleport() {
    bool allowed = false;
    if (isModerator() &&
        getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanTeleport) != 0) {
        allowed = true;
    }
    return allowed;
}

bool Player::hasInvisiblePrivilege() {
    bool enabled = false;
    if (app.GetGameHostOption(eGameHostOption_HostCanBeInvisible) != 0 &&
        getPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invisible) != 0) {
        enabled = true;
    }
    return enabled;
}

bool Player::hasInvulnerablePrivilege() {
    bool enabled = false;
    if (app.GetGameHostOption(eGameHostOption_HostCanBeInvisible) != 0 &&
        getPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invulnerable) !=
            0) {
        enabled = true;
    }
    return enabled;
}

bool Player::isModerator() {
    return getPlayerGamePrivilege(Player::ePlayerGamePrivilege_Op) != 0;
}

void Player::enableAllPlayerPrivileges(unsigned int& uigamePrivileges,
                                       bool enable) {
    Player::setPlayerGamePrivilege(uigamePrivileges,
                                   Player::ePlayerGamePrivilege_CannotMine,
                                   enable ? 0 : 1);
    Player::setPlayerGamePrivilege(uigamePrivileges,
                                   Player::ePlayerGamePrivilege_CannotBuild,
                                   enable ? 0 : 1);
    Player::setPlayerGamePrivilege(
        uigamePrivileges, Player::ePlayerGamePrivilege_CannotAttackPlayers,
        enable ? 0 : 1);
    Player::setPlayerGamePrivilege(
        uigamePrivileges, Player::ePlayerGamePrivilege_CannotAttackAnimals,
        enable ? 0 : 1);
    Player::setPlayerGamePrivilege(
        uigamePrivileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches,
        enable ? 1 : 0);
    Player::setPlayerGamePrivilege(
        uigamePrivileges, Player::ePlayerGamePrivilege_CanUseContainers,
        enable ? 1 : 0);
}

void Player::enableAllPlayerPrivileges(bool enable) {
    Player::enableAllPlayerPrivileges(m_uiGamePrivileges, enable);
}

bool Player::canCreateParticles() { return !hasInvisiblePrivilege(); }

std::vector<ModelPart*>* Player::GetAdditionalModelParts() {
    if (m_ppAdditionalModelParts == nullptr && !m_bCheckedForModelParts) {
        bool hasCustomTexture = !customTextureUrl.empty();
        bool customTextureIsDefaultSkin =
            customTextureUrl.substr(0, 3).compare(L"def") == 0;

        
        m_ppAdditionalModelParts = app.GetAdditionalModelParts(m_dwSkinId);

        
        
        
        if (!hasCustomTexture || customTextureIsDefaultSkin ||
            m_ppAdditionalModelParts != nullptr ||
            app.IsFileInMemoryTextures(customTextureUrl)) {
            m_bCheckedForModelParts = true;
        }
        if (m_ppAdditionalModelParts == nullptr &&
            !m_bCheckedDLCForModelParts) {
            m_bCheckedDLCForModelParts = true;

            
            app.DebugPrintf(
                "m_bCheckedForModelParts Couldn't get model parts for skin "
                "%X\n",
                m_dwSkinId);

            
            DLCSkinFile* pDLCSkinFile =
                app.m_dlcManager.getSkinFile(this->customTextureUrl);

            if (pDLCSkinFile != nullptr) {
                const int additionalBoxCount =
                    pDLCSkinFile->getAdditionalBoxesCount();
                if (additionalBoxCount != 0) {
                    app.DebugPrintf(
                        "m_bCheckedForModelParts Got model parts from DLCskin "
                        "for skin %X\n",
                        m_dwSkinId);
                    m_ppAdditionalModelParts = app.SetAdditionalSkinBoxes(
                        m_dwSkinId, pDLCSkinFile->getAdditionalBoxes());
                }

                app.SetAnimOverrideBitmask(
                    pDLCSkinFile->getSkinID(),
                    pDLCSkinFile->getAnimOverrideBitmask());

                m_bCheckedForModelParts = true;
            }
        }

        if (m_bCheckedForModelParts)
            setAnimOverrideBitmask(getSkinAnimOverrideBitmask(m_dwSkinId));
    }
    return m_ppAdditionalModelParts;
}

void Player::SetAdditionalModelParts(
    std::vector<ModelPart*>* ppAdditionalModelParts) {
    m_ppAdditionalModelParts = ppAdditionalModelParts;
}
