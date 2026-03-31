#include "PacketListener.h"
#include "Minecraft.World/net/minecraft/network/packet/AddEntityPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/AddExperienceOrbPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/AddGlobalEntityPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/AddMobPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/AddPaintingPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/AddPlayerPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/AnimatePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/AwardStatPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ChatAutoCompletePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ChatPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ChunkTilesUpdatePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ChunkVisibilityAreaPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ChunkVisibilityPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ClientInformationPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ClientProtocolPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ComplexItemDataPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ContainerAckPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ContainerButtonClickPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ContainerClickPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ContainerClosePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ContainerOpenPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ContainerSetContentPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ContainerSetDataPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ContainerSetSlotPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/CraftItemPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/CustomPayloadPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/DebugOptionsPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/EntityActionAtPositionPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/EntityEventPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ExplodePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/GameCommandPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/GameEventPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/GetInfoPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/InteractPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/KeepAlivePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/KickPlayerPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/LevelEventPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/LevelParticlesPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/LevelSoundPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/LoginPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/MoveEntityPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/MoveEntityPacketSmall.h"
#include "Minecraft.World/net/minecraft/network/packet/MovePlayerPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/PlayerAbilitiesPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/PlayerActionPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/PlayerCommandPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/PlayerInfoPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/PlayerInputPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/PreLoginPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/RemoveEntitiesPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/RemoveMobEffectPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/RespawnPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/RotateHeadPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ServerAuthDataPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/ServerSettingsChangedPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetCarriedItemPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetCreativeModeSlotPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetDisplayObjectivePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetEntityDataPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetEntityLinkPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetEntityMotionPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetEquippedItemPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetExperiencePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetHealthPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetObjectivePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetPlayerTeamPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetScorePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetSpawnPositionPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SetTimePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/SignUpdatePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TakeItemEntityPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TeleportEntityPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TextureAndGeometryChangePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TextureAndGeometryPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TextureChangePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TexturePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TileDestructionPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TileEntityDataPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TileEventPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TileUpdatePacket.h"
#include "Minecraft.World/net/minecraft/network/packet/TradeItemPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/UpdateAttributesPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/UpdateGameRuleProgressPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/UpdateMobEffectPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/UpdateProgressPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/UseItemPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/XZPacket.h"

void PacketListener::handleBlockRegionUpdate(
    std::shared_ptr<BlockRegionUpdatePacket> packet) {}

void PacketListener::onUnhandledPacket(std::shared_ptr<Packet> packet) {}

void PacketListener::onDisconnect(DisconnectPacket::eDisconnectReason reason,
                                  void* reasonObjects) {}

void PacketListener::handleDisconnect(
    std::shared_ptr<DisconnectPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleLogin(std::shared_ptr<LoginPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleMovePlayer(
    std::shared_ptr<MovePlayerPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleChunkTilesUpdate(
    std::shared_ptr<ChunkTilesUpdatePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handlePlayerAction(
    std::shared_ptr<PlayerActionPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTileUpdate(
    std::shared_ptr<TileUpdatePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleChunkVisibility(
    std::shared_ptr<ChunkVisibilityPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleAddPlayer(std::shared_ptr<AddPlayerPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleMoveEntity(
    std::shared_ptr<MoveEntityPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleMoveEntitySmall(
    std::shared_ptr<MoveEntityPacketSmall> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTeleportEntity(
    std::shared_ptr<TeleportEntityPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleUseItem(std::shared_ptr<UseItemPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleSetCarriedItem(
    std::shared_ptr<SetCarriedItemPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleRemoveEntity(
    std::shared_ptr<RemoveEntitiesPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTakeItemEntity(
    std::shared_ptr<TakeItemEntityPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleChat(std::shared_ptr<ChatPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleAddEntity(std::shared_ptr<AddEntityPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleAnimate(std::shared_ptr<AnimatePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handlePlayerCommand(
    std::shared_ptr<PlayerCommandPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handlePreLogin(std::shared_ptr<PreLoginPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleAddMob(std::shared_ptr<AddMobPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleSetTime(std::shared_ptr<SetTimePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleSetSpawn(
    std::shared_ptr<SetSpawnPositionPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleSetEntityMotion(
    std::shared_ptr<SetEntityMotionPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleSetEntityData(
    std::shared_ptr<SetEntityDataPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleEntityLinkPacket(
    std::shared_ptr<SetEntityLinkPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleInteract(std::shared_ptr<InteractPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleEntityEvent(
    std::shared_ptr<EntityEventPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleSetHealth(std::shared_ptr<SetHealthPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleRespawn(std::shared_ptr<RespawnPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTexture(std::shared_ptr<TexturePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTextureAndGeometry(
    std::shared_ptr<TextureAndGeometryPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleExplosion(std::shared_ptr<ExplodePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleContainerOpen(
    std::shared_ptr<ContainerOpenPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleContainerClose(
    std::shared_ptr<ContainerClosePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleContainerClick(
    std::shared_ptr<ContainerClickPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleContainerSetSlot(
    std::shared_ptr<ContainerSetSlotPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleContainerContent(
    std::shared_ptr<ContainerSetContentPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleSignUpdate(
    std::shared_ptr<SignUpdatePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleContainerSetData(
    std::shared_ptr<ContainerSetDataPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleSetEquippedItem(
    std::shared_ptr<SetEquippedItemPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleContainerAck(
    std::shared_ptr<ContainerAckPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleAddPainting(
    std::shared_ptr<AddPaintingPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTileEvent(std::shared_ptr<TileEventPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleAwardStat(std::shared_ptr<AwardStatPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleEntityActionAtPosition(
    std::shared_ptr<EntityActionAtPositionPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handlePlayerInput(
    std::shared_ptr<PlayerInputPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleGameEvent(std::shared_ptr<GameEventPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleAddGlobalEntity(
    std::shared_ptr<AddGlobalEntityPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleComplexItemData(
    std::shared_ptr<ComplexItemDataPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleLevelEvent(
    std::shared_ptr<LevelEventPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

// 1.8.2
void PacketListener::handleGetInfo(std::shared_ptr<GetInfoPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateMobEffect(
    std::shared_ptr<UpdateMobEffectPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleRemoveMobEffect(
    std::shared_ptr<RemoveMobEffectPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerInfo(
    std::shared_ptr<PlayerInfoPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleKeepAlive(std::shared_ptr<KeepAlivePacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleSetExperience(
    std::shared_ptr<SetExperiencePacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCreativeModeSlot(
    std::shared_ptr<SetCreativeModeSlotPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleAddExperienceOrb(
    std::shared_ptr<AddExperienceOrbPacket> packet) {
    onUnhandledPacket(packet);
}

// 1.0.1
void PacketListener::handleContainerButtonClick(
    std::shared_ptr<ContainerButtonClickPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleTileEntityData(
    std::shared_ptr<TileEntityDataPacket> tileEntityDataPacket) {
    onUnhandledPacket(tileEntityDataPacket);
}

// 1.1
void PacketListener::handleCustomPayload(
    std::shared_ptr<CustomPayloadPacket> customPayloadPacket) {
    onUnhandledPacket(customPayloadPacket);
}

// 1.2.3
void PacketListener::handleRotateMob(
    std::shared_ptr<RotateHeadPacket> rotateMobPacket) {
    onUnhandledPacket(rotateMobPacket);
}

// 1.3.2
void PacketListener::handleClientProtocolPacket(
    std::shared_ptr<ClientProtocolPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleServerAuthData(
    std::shared_ptr<ServerAuthDataPacket> packet) {
    onUnhandledPacket(packet);
}

// void PacketListener::handleSharedKey(shared_ptr<SharedKeyPacket> packet)
//{
//	onUnhandledPacket(packet);
// }

void PacketListener::handlePlayerAbilities(
    std::shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket) {
    onUnhandledPacket(playerAbilitiesPacket);
}

void PacketListener::handleChatAutoComplete(
    std::shared_ptr<ChatAutoCompletePacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleClientInformation(
    std::shared_ptr<ClientInformationPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleSoundEvent(
    std::shared_ptr<LevelSoundPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleTileDestruction(
    std::shared_ptr<TileDestructionPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleClientCommand(
    std::shared_ptr<ClientCommandPacket> packet) {}

// void PacketListener::handleLevelChunks(shared_ptr<LevelChunksPacket> packet)
//{
//	onUnhandledPacket(packet);
// }

bool PacketListener::canHandleAsyncPackets() { return false; }

// 1.6.4
void PacketListener::handleAddObjective(
    std::shared_ptr<SetObjectivePacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleSetScore(std::shared_ptr<SetScorePacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleSetDisplayObjective(
    std::shared_ptr<SetDisplayObjectivePacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleSetPlayerTeamPacket(
    std::shared_ptr<SetPlayerTeamPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleParticleEvent(
    std::shared_ptr<LevelParticlesPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateAttributes(
    std::shared_ptr<UpdateAttributesPacket> packet) {
    onUnhandledPacket(packet);
}

void PacketListener::handleTileEditorOpen(
    std::shared_ptr<TileEditorOpenPacket> tileEditorOpenPacket) {}

bool PacketListener::isDisconnected() { return false; }

// 4J Added

void PacketListener::handleCraftItem(std::shared_ptr<CraftItemPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTradeItem(std::shared_ptr<TradeItemPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleDebugOptions(
    std::shared_ptr<DebugOptionsPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleServerSettingsChanged(
    std::shared_ptr<ServerSettingsChangedPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleChunkVisibilityArea(
    std::shared_ptr<ChunkVisibilityAreaPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleUpdateProgress(
    std::shared_ptr<UpdateProgressPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTextureChange(
    std::shared_ptr<TextureChangePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleTextureAndGeometryChange(
    std::shared_ptr<TextureAndGeometryChangePacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleUpdateGameRuleProgressPacket(
    std::shared_ptr<UpdateGameRuleProgressPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleKickPlayer(
    std::shared_ptr<KickPlayerPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleXZ(std::shared_ptr<XZPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}

void PacketListener::handleGameCommand(
    std::shared_ptr<GameCommandPacket> packet) {
    onUnhandledPacket((std::shared_ptr<Packet>)packet);
}
