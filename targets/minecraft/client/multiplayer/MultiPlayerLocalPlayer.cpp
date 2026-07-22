#include "MultiPlayerLocalPlayer.h"

#include <wchar.h>

#include <cmath>

#include "ClientConnection.h"
#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/Tutorial/TutorialEnum.h"
#include "app/common/src/Tutorial/TutorialMode.h"
#include "app/mac/MacGame.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/MultiPlayerGameMode.h"
#include "minecraft/client/player/Input.h"
#include "minecraft/client/player/LocalPlayer.h"
#include "minecraft/network/packet/AnimatePacket.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/network/packet/ClientCommandPacket.h"
#include "minecraft/network/packet/ContainerClosePacket.h"
#include "minecraft/network/packet/MovePlayerPacket.h"
#include "minecraft/network/packet/PlayerAbilitiesPacket.h"
#include "minecraft/network/packet/PlayerActionPacket.h"
#include "minecraft/network/packet/PlayerCommandPacket.h"
#include "minecraft/network/packet/PlayerInputPacket.h"
#include "minecraft/network/packet/TextureAndGeometryChangePacket.h"
#include "minecraft/network/packet/TextureChangePacket.h"
#include "minecraft/stats/Stat.h"
#include "minecraft/world/effect/MobEffect.h"
#include "minecraft/world/effect/MobEffectInstance.h"
#include "minecraft/world/entity/player/Abilities.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/inventory/AbstractContainerMenu.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/phys/AABB.h"

class User;
class ItemEntity;


#if defined(STRESS_TEST_MOVE)
volatile bool stressTestEnabled = true;
#endif

MultiplayerLocalPlayer::MultiplayerLocalPlayer(Minecraft* minecraft,
                                               Level* level, User* user,
                                               ClientConnection* connection)
    : LocalPlayer(minecraft, level, user, level->dimension->id) {
    
    flashOnSetHealth = false;
    xLast = yLast1 = yLast2 = zLast = 0;
    yRotLast = xRotLast = 0;
    lastOnGround = false;
    lastSneaked = false;
    lastIdle = false;
    lastSprinting = false;
    positionReminder = 0;

    this->connection = connection;
}

bool MultiplayerLocalPlayer::hurt(DamageSource* source, float dmg) {
    return false;
}

void MultiplayerLocalPlayer::heal(float heal) {}

void MultiplayerLocalPlayer::tick() {
    
    
    
    

    











    if (!level->hasChunkAt(std::floor(x), 0, std::floor(z))) return;

    double tempX = x, tempY = y, tempZ = z;

    LocalPlayer::tick();

    
#if defined(STRESS_TEST_MOVE)
    if (stressTestEnabled) {
        StressTestMove(&tempX, &tempY, &tempZ);
    }
#endif

    
    
    
    if (minecraft->localgameModes[m_iPad]->getTutorial()->canMoveToPosition(
            tempX, tempY, tempZ, x, y, z)) {
        if (isRiding()) {
            connection->send(std::make_shared<MovePlayerPacket::Rot>(
                yRot, xRot, onGround, abilities.flying));
            connection->send(std::make_shared<PlayerInputPacket>(
                xxa, yya, input->jumping, input->sneaking));
        } else {
            sendPosition();
        }
    } else {
        
        
        this->setPos(tempX, y, tempZ);
    }
}

void MultiplayerLocalPlayer::sendPosition() {
    bool sprinting = isSprinting();
    if (sprinting != lastSprinting) {
        if (sprinting)
            connection->send(std::make_shared<PlayerCommandPacket>(
                shared_from_this(), PlayerCommandPacket::START_SPRINTING));
        else
            connection->send(std::make_shared<PlayerCommandPacket>(
                shared_from_this(), PlayerCommandPacket::STOP_SPRINTING));

        lastSprinting = sprinting;
    }

    bool sneaking = isSneaking();
    if (sneaking != lastSneaked) {
        if (sneaking)
            connection->send(std::make_shared<PlayerCommandPacket>(
                shared_from_this(), PlayerCommandPacket::START_SNEAKING));
        else
            connection->send(std::make_shared<PlayerCommandPacket>(
                shared_from_this(), PlayerCommandPacket::STOP_SNEAKING));

        lastSneaked = sneaking;
    }

    bool idle = isIdle();
    if (idle != lastIdle) {
        if (idle)
            connection->send(std::make_shared<PlayerCommandPacket>(
                shared_from_this(), PlayerCommandPacket::START_IDLEANIM));
        else
            connection->send(std::make_shared<PlayerCommandPacket>(
                shared_from_this(), PlayerCommandPacket::STOP_IDLEANIM));

        lastIdle = idle;
    }

    double xdd = x - xLast;
    double ydd1 = bb.y0 - yLast1;
    double zdd = z - zLast;

    double rydd = yRot - yRotLast;
    double rxdd = xRot - xRotLast;

    bool move = (xdd * xdd + ydd1 * ydd1 + zdd * zdd) > 0.03 * 0.03 ||
                positionReminder >= POSITION_REMINDER_INTERVAL;
    bool rot = rydd != 0 || rxdd != 0;
    if (riding != nullptr) {
        connection->send(std::make_shared<MovePlayerPacket::PosRot>(
            xd, -999, -999, zd, yRot, xRot, onGround, abilities.flying));
        move = false;
    } else {
        if (move && rot) {
            connection->send(std::make_shared<MovePlayerPacket::PosRot>(
                x, bb.y0, y, z, yRot, xRot, onGround, abilities.flying));
        } else if (move) {
            connection->send(std::make_shared<MovePlayerPacket::Pos>(
                x, bb.y0, y, z, onGround, abilities.flying));
        } else if (rot) {
            connection->send(std::make_shared<MovePlayerPacket::Rot>(
                yRot, xRot, onGround, abilities.flying));
        } else {
            connection->send(std::shared_ptr<MovePlayerPacket>(
                new MovePlayerPacket(onGround, abilities.flying)));
        }
    }

    positionReminder++;
    lastOnGround = onGround;

    if (move) {
        xLast = x;
        yLast1 = bb.y0;
        yLast2 = y;
        zLast = z;
        positionReminder = 0;
    }
    if (rot) {
        yRotLast = yRot;
        xRotLast = xRot;
    }
}

std::shared_ptr<ItemEntity> MultiplayerLocalPlayer::drop() {
    connection->send(std::shared_ptr<PlayerActionPacket>(
        new PlayerActionPacket(PlayerActionPacket::DROP_ITEM, 0, 0, 0, 0)));
    return nullptr;
}

void MultiplayerLocalPlayer::reallyDrop(
    std::shared_ptr<ItemEntity> itemEntity) {}

void MultiplayerLocalPlayer::chat(const std::wstring& message) {
    
    
    
    if (connection == nullptr) return;
    connection->send(std::make_shared<ChatPacket>(message));
}

void MultiplayerLocalPlayer::swing() {
    LocalPlayer::swing();
    connection->send(std::shared_ptr<AnimatePacket>(
        new AnimatePacket(shared_from_this(), AnimatePacket::SWING)));
}

void MultiplayerLocalPlayer::respawn() {
    connection->send(std::shared_ptr<ClientCommandPacket>(
        new ClientCommandPacket(ClientCommandPacket::PERFORM_RESPAWN)));
}

void MultiplayerLocalPlayer::actuallyHurt(DamageSource* source, float dmg) {
    if (isInvulnerable()) return;
    setHealth(getHealth() - dmg);
}


void MultiplayerLocalPlayer::completeUsingItem() {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (useItem != nullptr && pMinecraft->localgameModes[m_iPad] != nullptr) {
        TutorialMode* gameMode =
            (TutorialMode*)pMinecraft->localgameModes[m_iPad];
        Tutorial* tutorial = gameMode->getTutorial();
        tutorial->completeUsingItem(useItem);
    }
    Player::completeUsingItem();
}

void MultiplayerLocalPlayer::onEffectAdded(MobEffectInstance* effect) {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft->localgameModes[m_iPad] != nullptr) {
        TutorialMode* gameMode =
            (TutorialMode*)pMinecraft->localgameModes[m_iPad];
        Tutorial* tutorial = gameMode->getTutorial();
        tutorial->onEffectChanged(MobEffect::effects[effect->getId()]);
    }
    Player::onEffectAdded(effect);
}

void MultiplayerLocalPlayer::onEffectUpdated(MobEffectInstance* effect,
                                             bool doRefreshAttributes) {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft->localgameModes[m_iPad] != nullptr) {
        TutorialMode* gameMode =
            (TutorialMode*)pMinecraft->localgameModes[m_iPad];
        Tutorial* tutorial = gameMode->getTutorial();
        tutorial->onEffectChanged(MobEffect::effects[effect->getId()]);
    }
    Player::onEffectUpdated(effect, doRefreshAttributes);
}

void MultiplayerLocalPlayer::onEffectRemoved(MobEffectInstance* effect) {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft->localgameModes[m_iPad] != nullptr) {
        TutorialMode* gameMode =
            (TutorialMode*)pMinecraft->localgameModes[m_iPad];
        Tutorial* tutorial = gameMode->getTutorial();
        tutorial->onEffectChanged(MobEffect::effects[effect->getId()], true);
    }
    Player::onEffectRemoved(effect);
}

void MultiplayerLocalPlayer::closeContainer() {
    connection->send(std::shared_ptr<ContainerClosePacket>(
        new ContainerClosePacket(containerMenu->containerId)));
    clientSideCloseContainer();
}


void MultiplayerLocalPlayer::clientSideCloseContainer() {
    inventory->setCarried(nullptr);
    LocalPlayer::closeContainer();
}

void MultiplayerLocalPlayer::hurtTo(float newHealth, uint8_t damageSource) {
    if (flashOnSetHealth) {
        LocalPlayer::hurtTo(newHealth, damageSource);
    } else {
        setHealth(newHealth);
        flashOnSetHealth = true;
    }
}

void MultiplayerLocalPlayer::awardStat(Stat* stat,
                                       const std::vector<uint8_t>& param) {
    if (stat == nullptr) {
        return;
    }

    if (stat->awardLocallyOnly) {
        LocalPlayer::awardStat(stat, param);
    } else {
        return;
    }
}

void MultiplayerLocalPlayer::awardStatFromServer(Stat* stat,
                                                 std::vector<uint8_t>& param) {
    if (stat != nullptr && !stat->awardLocallyOnly) {
        LocalPlayer::awardStat(stat, param);
    }
}

void MultiplayerLocalPlayer::onUpdateAbilities() {
    connection->send(std::shared_ptr<PlayerAbilitiesPacket>(
        new PlayerAbilitiesPacket(&abilities)));
}

bool MultiplayerLocalPlayer::isLocalPlayer() { return true; }

void MultiplayerLocalPlayer::sendRidingJump() {
    connection->send(std::make_shared<PlayerCommandPacket>(
        shared_from_this(), PlayerCommandPacket::RIDING_JUMP,
        (int)(getJumpRidingScale() * 100.0f)));
}

void MultiplayerLocalPlayer::sendOpenInventory() {
    connection->send(std::make_shared<PlayerCommandPacket>(
        shared_from_this(), PlayerCommandPacket::OPEN_INVENTORY));
}

void MultiplayerLocalPlayer::ride(std::shared_ptr<Entity> e) {
    bool wasRiding = riding != nullptr;
    LocalPlayer::ride(e);
    bool isRiding = riding != nullptr;

    
    if (wasRiding && !isRiding) {
        setSneaking(false);
        input->sneaking = false;
    }

    updateRichPresence();

    Minecraft* pMinecraft = Minecraft::GetInstance();

    if (pMinecraft->localgameModes[m_iPad] != nullptr) {
        TutorialMode* gameMode =
            (TutorialMode*)pMinecraft->localgameModes[m_iPad];
        if (wasRiding && !isRiding) {
            gameMode->getTutorial()->changeTutorialState(
                e_Tutorial_State_Gameplay);
        } else if (!wasRiding && isRiding) {
            gameMode->getTutorial()->onRideEntity(e);
        }
    }
}

void MultiplayerLocalPlayer::StopSleeping() {
    connection->send(std::make_shared<PlayerCommandPacket>(
        shared_from_this(), PlayerCommandPacket::STOP_SLEEPING));
}


void MultiplayerLocalPlayer::setAndBroadcastCustomSkin(std::uint32_t skinId) {
    std::uint32_t oldSkinIndex = getCustomSkin();
    LocalPlayer::setCustomSkin(skinId);
#if !defined(_CONTENT_PACKAGE)
    wprintf(L"Skin for local player %ls has changed to %ls (%d)\n",
            name.c_str(), customTextureUrl.c_str(), getPlayerDefaultSkin());
#endif
    if (getCustomSkin() != oldSkinIndex)
        connection->send(std::shared_ptr<TextureAndGeometryChangePacket>(
            new TextureAndGeometryChangePacket(
                shared_from_this(), app.GetPlayerSkinName(GetXboxPad()))));
}

void MultiplayerLocalPlayer::setAndBroadcastCustomCape(std::uint32_t capeId) {
    std::uint32_t oldCapeIndex = getCustomCape();
    LocalPlayer::setCustomCape(capeId);
#if !defined(_CONTENT_PACKAGE)
    wprintf(L"Cape for local player %ls has changed to %ls\n", name.c_str(),
            customTextureUrl2.c_str());
#endif
    if (getCustomCape() != oldCapeIndex)
        connection->send(std::make_shared<TextureChangePacket>(
            shared_from_this(), TextureChangePacket::e_TextureChange_Cape,
            app.GetPlayerCapeName(GetXboxPad())));
}









#if defined(STRESS_TEST_MOVE)
void MultiplayerLocalPlayer::StressTestMove(double* tempX, double* tempY,
                                            double* tempZ) {
    static volatile int64_t lastChangeTime = 0;
    static volatile int64_t lastTeleportTime = 0;
    static int lastCount = 0;
    static int stressTestCount = 0;
    const int dirChangeTickCount = 200;

    int64_t currentTime = System::currentTimeMillis();

    bool faultFound = false;
    int count = Minecraft::GetInstance()->levelRenderer->checkAllPresentChunks(
        &faultFound);

    






    if (count != lastCount) {
        lastChangeTime = currentTime;
        lastCount = count;
    }

    static float angle = 30.0;
    static float dx = cos(30.0);
    static float dz = sin(30.0);

    float nx = x + (dx * 1.2);
    float nz = z + (dz * 1.2);
    float ny = y;
    if (ny < 140.0f) ny += 0.5f;
    if (nx > 2539.0) {
        nx = 2539.0;
        dx = -dx;
    }
    if (nz > 2539.0) {
        nz = 2539.0;
        dz = -dz;
    }
    if (nx < -2550.0) {
        nx = -2550.0;
        dx = -dx;
    }

    if (nz < -2550.0) {
        nz = -2550.0;
        dz = -dz;
    }
    absMoveTo(nx, ny, nz, yRot, xRot);
    stressTestCount++;
}
#endif
