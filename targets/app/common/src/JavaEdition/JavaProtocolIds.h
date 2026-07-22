#pragma once
#include <cstdint>

namespace JavaHandshakePacketId {
constexpr int32_t Handshake = 0x00;
}

namespace JavaLoginClientboundId {
constexpr int32_t Disconnect = 0x00;
constexpr int32_t EncryptionRequest = 0x01;
constexpr int32_t LoginSuccess = 0x02;
constexpr int32_t SetCompression = 0x03;
}
namespace JavaLoginServerboundId {
constexpr int32_t LoginStart = 0x00;
constexpr int32_t EncryptionResponse = 0x01;
}

namespace JavaPlayServerboundId {
constexpr int32_t KeepAlive = 0x00;
constexpr int32_t ChatMessage = 0x01;
constexpr int32_t UseEntity = 0x02;
constexpr int32_t Player = 0x03;
constexpr int32_t PlayerPosition = 0x04;
constexpr int32_t PlayerLook = 0x05;
constexpr int32_t PlayerPositionAndLook = 0x06;
constexpr int32_t PlayerDigging = 0x07;
constexpr int32_t PlayerBlockPlacement = 0x08;
constexpr int32_t HeldItemChange = 0x09;
constexpr int32_t Animation = 0x0A;
constexpr int32_t EntityAction = 0x0B;
constexpr int32_t CloseWindow = 0x0D;
constexpr int32_t ClickWindow = 0x0E;
constexpr int32_t ConfirmTransaction = 0x0F;
constexpr int32_t CreativeInventoryAction = 0x10;
constexpr int32_t UpdateSign = 0x12;
constexpr int32_t ClientSettings = 0x15;
constexpr int32_t ClientStatus = 0x16;
constexpr int32_t PluginMessage = 0x17;
constexpr int32_t ResourcePackStatus = 0x19;
}

namespace JavaPlayClientboundId {
constexpr int32_t KeepAlive = 0x00;
constexpr int32_t JoinGame = 0x01;
constexpr int32_t ChatMessage = 0x02;
constexpr int32_t UpdateTime = 0x03;
constexpr int32_t SpawnObject = 0x0E;
constexpr int32_t EntityEquipment = 0x04;
constexpr int32_t SpawnPosition = 0x05;
constexpr int32_t UpdateHealth = 0x06;
constexpr int32_t Respawn = 0x07;
constexpr int32_t PlayerPositionAndLook = 0x08;
constexpr int32_t HeldItemChange = 0x09;
constexpr int32_t Animation = 0x0B;
constexpr int32_t SpawnPlayer = 0x0C;
constexpr int32_t SpawnMob = 0x0F;
constexpr int32_t EntityVelocity = 0x12;
constexpr int32_t DestroyEntities = 0x13;
constexpr int32_t EntityRelativeMove = 0x15;
constexpr int32_t EntityLook = 0x16;
constexpr int32_t EntityLookAndMove = 0x17;
constexpr int32_t EntityTeleport = 0x18;
constexpr int32_t EntityHeadLook = 0x19;
constexpr int32_t EntityStatus = 0x1A;
constexpr int32_t AttachEntity = 0x1B;
constexpr int32_t EntityMetadata = 0x1C;
constexpr int32_t SetExperience = 0x1F;
constexpr int32_t ChunkData = 0x21;
constexpr int32_t SoundEffect = 0x29;
constexpr int32_t GameStateChange = 0x2B;
constexpr int32_t MultiBlockChange = 0x22;
constexpr int32_t BlockChange = 0x23;
constexpr int32_t BlockAction = 0x24;
constexpr int32_t Explosion = 0x27;
constexpr int32_t MapChunkBulk = 0x26;
constexpr int32_t UpdateSign = 0x33;
constexpr int32_t OpenSignEditor = 0x36;
constexpr int32_t OpenWindow = 0x2D;
constexpr int32_t CloseWindow = 0x2E;
constexpr int32_t WindowItems = 0x30;
constexpr int32_t CraftProgressBar = 0x31;
constexpr int32_t SetSlot = 0x2F;
constexpr int32_t ConfirmTransaction = 0x32;
constexpr int32_t PlayerListItem = 0x38;
constexpr int32_t PlayerAbilities = 0x39;
constexpr int32_t ScoreboardObjective = 0x3B;
constexpr int32_t UpdateScore = 0x3C;
constexpr int32_t DisplayScoreboard = 0x3D;
constexpr int32_t Teams = 0x3E;
constexpr int32_t PluginMessage = 0x3F;
constexpr int32_t Disconnect = 0x40;
constexpr int32_t Title = 0x45;
constexpr int32_t PlayerListHeaderFooter = 0x47;
constexpr int32_t ResourcePackSend = 0x48;
}