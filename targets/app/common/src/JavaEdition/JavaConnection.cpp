#include "app/common/src/JavaEdition/JavaConnection.h"

#include <cstdlib>
#include <cstring>
#include <utility>

#include "app/common/src/JavaEdition/JavaBlockIdMap.h"
#include "app/common/src/JavaEdition/JavaChatJson.h"
#include "app/common/src/JavaEdition/JavaPacketBuffer.h"
#include "app/common/src/JavaEdition/JavaProtocolIds.h"

namespace {

constexpr int32_t kProtocolVersion1_8 = 47;
constexpr int32_t kHandshakeNextStateLogin = 2;
constexpr int kConnectTimeoutMs = 5000;
constexpr size_t kRecvChunkBytes = 4096;
constexpr size_t kMaxOutgoingChatChars = 100;

std::vector<uint8_t> takeBytes(const JavaPacketWriter& w) {
    return w.bytes();
}

std::string clipChatUtf8(const std::string& utf8) {
    size_t i = 0;
    size_t chars = 0;
    while (i < utf8.size() && chars < kMaxOutgoingChatChars) {
        const unsigned char b = static_cast<unsigned char>(utf8[i]);
        size_t step = 1;
        if      ((b & 0x80) == 0x00) step = 1;
        else if ((b & 0xE0) == 0xC0) step = 2;
        else if ((b & 0xF0) == 0xE0) step = 3;
        else if ((b & 0xF8) == 0xF0) step = 4;
        if (i + step > utf8.size()) break;
        i += step;
        ++chars;
    }
    return utf8.substr(0, i);
}

[[maybe_unused]] void p3_skipEntityMetadata(JavaPacketReader& r) {
    while (r.remaining() > 0) {
        const uint8_t key = r.readU8();
        if (key == 0x7F) return;
        const int type = (key >> 5) & 0x07;
        switch (type) {
            case 0: (void)r.readU8();      break;
            case 1: (void)r.readU8(); (void)r.readU8(); break;
            case 2: (void)r.readI32();     break;
            case 3: (void)r.readI32();     break;
            case 4: (void)r.readUtf8(kMaxStringBytes); break;
            case 5: {
                const uint8_t hi = r.readU8();
                const uint8_t lo = r.readU8();
                const int16_t itemId = static_cast<int16_t>((hi << 8) | lo);
                if (itemId != -1) {
                    (void)r.readU8();
                    (void)r.readU8();
                    (void)r.readU8();
                    if (r.remaining() >= 1) {
                        const uint8_t nbt = r.readU8();
                        if (nbt != 0) {
                            throw JavaProtocolError("p3_skipEntityMetadata: NBT in slot");
                        }
                    }
                }
                break;
            }
            case 6: {
                (void)r.readI32();
                (void)r.readI32();
                (void)r.readI32();
                break;
            }
            case 7: {
                (void)r.readI32();
                (void)r.readI32();
                (void)r.readI32();
                break;
            }
            default:
                throw JavaProtocolError("p3_skipEntityMetadata: unknown type");
        }
    }
}

void p3_skipNbtPayload(JavaPacketReader& r, int type) {
    switch (type) {
        case 0:  return;
        case 1:  (void)r.readU8(); return;
        case 2:  (void)r.readU16(); return;
        case 3:  (void)r.readI32(); return;
        case 4:  (void)r.readI64(); return;
        case 5:  (void)r.readI32(); return;
        case 6:  (void)r.readI64(); return;
        case 7: {
            const int32_t n = r.readI32();
            if (n > 0) r.skipBytes(static_cast<size_t>(n));
            return;
        }
        case 8: {
            const uint16_t n = r.readU16();
            if (n > 0) r.skipBytes(n);
            return;
        }
        case 9: {
            const uint8_t childType = r.readU8();
            const int32_t n = r.readI32();
            for (int32_t i = 0; i < n; ++i) p3_skipNbtPayload(r, childType);
            return;
        }
        case 10: {
            for (;;) {
                const uint8_t t = r.readU8();
                if (t == 0) return;
                const uint16_t nameLen = r.readU16();
                if (nameLen > 0) r.skipBytes(nameLen);
                p3_skipNbtPayload(r, t);
            }
        }
        case 11: {
            const int32_t n = r.readI32();
            if (n > 0) r.skipBytes(static_cast<size_t>(n) * 4);
            return;
        }
        default:
            throw JavaProtocolError("p3_skipNbtPayload: unknown NBT tag");
    }
}

std::wstring p3_decodeNbtUtf8(const std::string& s) {
    std::wstring out;
    out.reserve(s.size());
    size_t i = 0;
    const size_t n = s.size();
    while (i < n) {
        const unsigned char c = static_cast<unsigned char>(s[i++]);
        uint32_t cp;
        int extra;
        if (c < 0x80) { cp = c; extra = 0; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; extra = 1; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; extra = 2; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; extra = 3; }
        else { cp = c; extra = 0; }
        for (int k = 0; k < extra && i < n; ++k)
            cp = (cp << 6) | (static_cast<unsigned char>(s[i++]) & 0x3F);
        out.push_back(static_cast<wchar_t>(cp));
    }
    return out;
}

std::wstring p3_readNbtString(JavaPacketReader& r) {
    const uint16_t len = r.readU16();
    std::string bytes(len, '\0');
    if (len > 0) r.readBytes(reinterpret_cast<uint8_t*>(&bytes[0]), len);
    return p3_decodeNbtUtf8(bytes);
}

std::string p3_readNbtName(JavaPacketReader& r) {
    const uint16_t len = r.readU16();
    std::string key(len, '\0');
    if (len > 0) r.readBytes(reinterpret_cast<uint8_t*>(&key[0]), len);
    return key;
}

void p3_readDisplayCompound(JavaPacketReader& r, JavaInvSlot& slot) {
    for (;;) {
        const uint8_t t = r.readU8();
        if (t == 0) return;
        const std::string key = p3_readNbtName(r);
        if (t == 8 && key == "Name") {
            slot.customName = p3_readNbtString(r);
        } else if (t == 9 && key == "Lore") {
            const uint8_t childType = r.readU8();
            const int32_t cnt = r.readI32();
            for (int32_t i = 0; i < cnt && i >= 0; ++i) {
                if (childType == 8) slot.lore.push_back(p3_readNbtString(r));
                else p3_skipNbtPayload(r, childType);
            }
        } else {
            p3_skipNbtPayload(r, t);
        }
    }
}

void p3_readJavaItemNbt(JavaPacketReader& r, JavaInvSlot& slot) {
    if (r.remaining() < 1) return;
    const uint8_t rootType = r.readU8();
    if (rootType == 0) return;
    if (rootType != 10) {
        const uint16_t nl = r.readU16();
        if (nl > 0) r.skipBytes(nl);
        p3_skipNbtPayload(r, rootType);
        return;
    }
    const uint16_t rootNameLen = r.readU16();
    if (rootNameLen > 0) r.skipBytes(rootNameLen);
    for (;;) {
        const uint8_t t = r.readU8();
        if (t == 0) return;
        const std::string key = p3_readNbtName(r);
        if (t == 10 && key == "display") {
            p3_readDisplayCompound(r, slot);
        } else {
            p3_skipNbtPayload(r, t);
        }
    }
}

JavaInvSlot p3_readJavaSlot(JavaPacketReader& r) {
    JavaInvSlot slot;
    slot.id = static_cast<int16_t>(r.readU16());
    if (slot.id != -1) {
        slot.count = r.readU8();
        slot.damage = static_cast<int16_t>(r.readU16());
        p3_readJavaItemNbt(r, slot);
    }
    return slot;
}

int p3_mapWindowType(const std::string& invType) {
    if (invType == "minecraft:crafting_table") return 1;
    if (invType == "minecraft:furnace")        return 2;
    if (invType == "minecraft:dispenser")      return 3;
    if (invType == "minecraft:enchanting_table") return 4;
    if (invType == "minecraft:brewing_stand")  return 5;
    if (invType == "minecraft:villager")       return 6;
    if (invType == "minecraft:beacon")         return 7;
    if (invType == "minecraft:anvil")          return 8;
    if (invType == "minecraft:hopper")         return 9;
    if (invType == "minecraft:dropper")        return 10;
    if (invType == "EntityHorse")              return 11;
    return 0;
}

bool p3_extractMetadataItem(JavaPacketReader& r, JavaInvSlot& outItem,
                            int* outFlags = nullptr,
                            std::string* outEntries = nullptr,
                            std::vector<JavaMetaEntry>* outMeta = nullptr) {
    bool found = false;
    while (r.remaining() > 0) {
        const uint8_t key = r.readU8();
        if (key == 0x7F) break;
        const int type = (key >> 5) & 0x07;
        const int index = key & 0x1F;
        if (outEntries) {
            char tmp[16];
            snprintf(tmp, sizeof(tmp), "%s%d:%d", outEntries->empty() ? "" : ",",
                     index, type);
            outEntries->append(tmp);
        }
        JavaMetaEntry entry;
        entry.index = static_cast<uint8_t>(index);
        entry.type = static_cast<uint8_t>(type);
        bool haveEntry = false;
        switch (type) {
            case 0: {
                const uint8_t v = r.readU8();
                if (index == 0 && outFlags) *outFlags = v;
                entry.intVal = static_cast<int8_t>(v);
                haveEntry = true;
                break;
            }
            case 1:
                entry.intVal = static_cast<int16_t>(r.readU16());
                haveEntry = true;
                break;
            case 2:
                entry.intVal = r.readI32();
                haveEntry = true;
                break;
            case 3: {
                const int32_t bits = r.readI32();
                std::memcpy(&entry.floatVal, &bits, sizeof(entry.floatVal));
                haveEntry = true;
                break;
            }
            case 4:
                entry.strVal = p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                haveEntry = true;
                break;
            case 5: {
                JavaInvSlot s = p3_readJavaSlot(r);
                if (index == 10) { outItem = s; found = true; }
                break;
            }
            case 6:
            case 7:
                (void)r.readI32(); (void)r.readI32(); (void)r.readI32();
                break;
            default:
                throw JavaProtocolError("p3_extractMetadataItem: bad type");
        }
        if (haveEntry && outMeta) outMeta->push_back(std::move(entry));
    }
    return found;
}

int p3_javaMobToLce(uint8_t javaType) {
    if (javaType >= 50 && javaType <= 99) return javaType;
    if (javaType == 120) return 120;
    return -1;
}

bool p3_decodeOneChunk(JavaPacketReader& r,
                       uint16_t primaryBitMask,
                       bool groundUp,
                       bool hasSkyLight,
                       std::vector<uint8_t>& outBuf) {
    outBuf.assign(164096, 0);
    for (int i = 65536 + 32768 + 32768; i < 65536 + 32768 + 32768 + 32768; ++i) {
        outBuf[static_cast<size_t>(i)] = 0xFF;
    }

    int sectionCount = 0;
    for (int s = 0; s < 16; ++s) {
        if (primaryBitMask & (1u << s)) ++sectionCount;
    }

    constexpr int kBlocksOff = 0;
    constexpr int kMetaOff = 65536;
    constexpr int kBlockLightOff = 65536 + 32768;
    constexpr int kSkyLightOff = 65536 + 32768 + 32768;
    constexpr int kBiomesOff = 65536 + 32768 + 32768 + 32768;

    auto loadSection = [&](int sectionY, const uint8_t* secBlocks,
                           const uint8_t* secMeta, const uint8_t* secBlockLight,
                           const uint8_t* secSkyLight) {
        const int baseY = sectionY * 16;
        for (int y = 0; y < 16; ++y) {
            const int columnY = baseY + y;
            for (int z = 0; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    const int srcIdx = (y << 8) | (z << 4) | x;
                    const int dstIdxYZX = (columnY << 8) | (z << 4) | x;
                    outBuf[kBlocksOff + dstIdxYZX] =
                        JavaBlockIdMap::toLce(secBlocks[srcIdx]);
                    const uint8_t metaNib =
                        (secMeta[srcIdx >> 1] >> ((srcIdx & 1) * 4)) & 0x0F;
                    const uint8_t blNib =
                        (secBlockLight[srcIdx >> 1] >> ((srcIdx & 1) * 4)) & 0x0F;
                    const uint8_t slNib = secSkyLight != nullptr
                        ? ((secSkyLight[srcIdx >> 1] >> ((srcIdx & 1) * 4)) & 0x0F)
                        : 0x0F;
                    const int yInHalf = columnY & 127;
                    const int baseOff = (columnY < 128) ? 0 : 16384;
                    const int dstByte =
                        baseOff + (x << 10) + (z << 6) + (yInHalf >> 1);
                    const int dstShift = (columnY & 1) * 4;
                    const uint8_t mask = static_cast<uint8_t>(0x0F << dstShift);
                    outBuf[kMetaOff + dstByte] =
                        (outBuf[kMetaOff + dstByte] & ~mask) |
                        static_cast<uint8_t>(metaNib << dstShift);
                    outBuf[kBlockLightOff + dstByte] =
                        (outBuf[kBlockLightOff + dstByte] & ~mask) |
                        static_cast<uint8_t>(blNib << dstShift);
                    outBuf[kSkyLightOff + dstByte] =
                        (outBuf[kSkyLightOff + dstByte] & ~mask) |
                        static_cast<uint8_t>(slNib << dstShift);
                }
            }
        }
    };

    std::vector<std::vector<uint8_t>> secBlocks(sectionCount,
                                                std::vector<uint8_t>(4096));
    std::vector<std::vector<uint8_t>> secMeta(sectionCount,
                                              std::vector<uint8_t>(2048));
    std::vector<std::vector<uint8_t>> secBlockLight(sectionCount,
                                                    std::vector<uint8_t>(2048));
    std::vector<std::vector<uint8_t>> secSkyLight(
        hasSkyLight ? sectionCount : 0, std::vector<uint8_t>(2048));

    std::vector<uint8_t> chars(8192);
    for (int s = 0; s < sectionCount; ++s) {
        r.readBytes(chars.data(), 8192);
        for (int i = 0; i < 4096; ++i) {
            const uint16_t lo = chars[i * 2];
            const uint16_t hi = chars[i * 2 + 1];
            const uint16_t c = static_cast<uint16_t>(lo | (hi << 8));
            secBlocks[s][i] = static_cast<uint8_t>((c >> 4) & 0xFF);
            const uint8_t meta = static_cast<uint8_t>(c & 0x0F);
            const int nibIdx = i >> 1;
            if (i & 1) {
                secMeta[s][nibIdx] =
                    (secMeta[s][nibIdx] & 0x0F) | (meta << 4);
            } else {
                secMeta[s][nibIdx] =
                    (secMeta[s][nibIdx] & 0xF0) | meta;
            }
        }
    }
    for (int i = 0; i < sectionCount; ++i) {
        r.readBytes(secBlockLight[i].data(), 2048);
    }
    if (hasSkyLight) {
        for (int i = 0; i < sectionCount; ++i) {
            r.readBytes(secSkyLight[i].data(), 2048);
        }
    }
    if (groundUp) {
        r.readBytes(outBuf.data() + kBiomesOff, 256);
    }

    int sIdx = 0;
    for (int s = 0; s < 16; ++s) {
        if (!(primaryBitMask & (1u << s))) continue;
        loadSection(s, secBlocks[sIdx].data(), secMeta[sIdx].data(),
                    secBlockLight[sIdx].data(),
                    hasSkyLight ? secSkyLight[sIdx].data() : nullptr);
        ++sIdx;
    }
    return true;
}

}

JavaConnection::JavaConnection(std::string host, uint16_t port,
                               std::string nicknameUtf8)
    : m_host(std::move(host)),
      m_port(port),
      m_nicknameUtf8(std::move(nicknameUtf8)) {}

JavaConnection::~JavaConnection() {
    requestDisconnect();
    if (m_worker.joinable()) {
        m_worker.join();
    }
}

void JavaConnection::start() {
    if (m_started) return;
    m_started = true;
    m_worker = std::thread([this]() { runWorker(); });
}

void JavaConnection::sendChat(const std::string& utf8) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendChat;
    cmd.chatUtf8 = utf8;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendPlayerPosition(double x, double y, double z,
                                        float yaw, float pitch, bool onGround) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendPlayerPosition;
    cmd.posX = x;
    cmd.posY = y;
    cmd.posZ = z;
    cmd.posYaw = yaw;
    cmd.posPitch = pitch;
    cmd.onGround = onGround;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendBlockDig(int x, int y, int z,
                                  uint8_t status, uint8_t face) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendBlockDig;
    cmd.blockX = x; cmd.blockY = y; cmd.blockZ = z;
    cmd.digStatus = status;
    cmd.face = face;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendBlockPlace(int x, int y, int z, uint8_t face,
                                    int16_t itemId, uint8_t itemCount,
                                    int16_t itemDamage,
                                    uint8_t cursorX, uint8_t cursorY,
                                    uint8_t cursorZ) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendBlockPlace;
    cmd.blockX = x; cmd.blockY = y; cmd.blockZ = z;
    cmd.face = face;
    cmd.heldItemId = itemId;
    cmd.heldItemCount = itemCount;
    cmd.heldItemDamage = itemDamage;
    cmd.cursorX = cursorX;
    cmd.cursorY = cursorY;
    cmd.cursorZ = cursorZ;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendHeldItem(int16_t slot) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendHeldItem;
    cmd.hotbarSlot = slot;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendCreativeSlot(int16_t slot, int16_t itemId,
                                      uint8_t count, int16_t damage) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendCreativeSlot;
    cmd.hotbarSlot = slot;
    cmd.heldItemId = itemId;
    cmd.heldItemCount = count;
    cmd.heldItemDamage = damage;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendWindowClick(int windowId, int16_t slot, uint8_t button,
                                     int16_t actionNum, uint8_t mode,
                                     int16_t itemId, uint8_t count,
                                     int16_t damage) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendWindowClick;
    cmd.winId = windowId;
    cmd.winSlot = slot;
    cmd.winButton = button;
    cmd.winActionNum = actionNum;
    cmd.winMode = mode;
    cmd.heldItemId = itemId;
    cmd.heldItemCount = count;
    cmd.heldItemDamage = damage;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendCloseWindow(int windowId) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendCloseWindow;
    cmd.winId = windowId;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendClientStatus(int action) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendClientStatus;
    cmd.actionId = action;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendAnimation() {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendAnimation;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendUseEntity(int32_t targetId, uint8_t action) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendUseEntity;
    cmd.targetEntityId = targetId;
    cmd.useEntityAction = action;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendEntityAction(int32_t playerId, int32_t action,
                                      int32_t param) {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendEntityAction;
    cmd.playerEntityId = playerId;
    cmd.actionId = action;
    cmd.actionParam = param;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendClientSettings() {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendClientSettings;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::sendBrand() {
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::SendBrand;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::requestDisconnect() {
    m_shutdownRequested.store(true);
    JavaConnectionCommand cmd;
    cmd.type = JavaConnectionCommandType::Disconnect;
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commandQueue.push_back(std::move(cmd));
}

void JavaConnection::drainEvents(std::vector<JavaConnectionEvent>& out) {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    out.reserve(out.size() + m_eventQueue.size());
    while (!m_eventQueue.empty()) {
        out.push_back(std::move(m_eventQueue.front()));
        m_eventQueue.pop_front();
    }
}

void JavaConnection::pushEvent(JavaConnectionEvent event) {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_eventQueue.push_back(std::move(event));
}

void JavaConnection::setState(JavaConnectionState s) {
    m_observedState.store(s);
    JavaConnectionEvent ev;
    ev.type = JavaConnectionEventType::StateChanged;
    ev.state = s;
    pushEvent(std::move(ev));
}

bool JavaConnection::pumpRecv(int timeoutMs) {
    uint8_t chunk[kRecvChunkBytes];
    const size_t n = m_tcp.recv(chunk, sizeof(chunk), timeoutMs);
    if (n == 0) {
        return false;
    }
    m_recvBuf.insert(m_recvBuf.end(), chunk, chunk + n);
    return true;
}

JavaProtocolCodec::DecodeStatus JavaConnection::nextFrame(
    JavaProtocolCodec::DecodedFrame& outFrame) {
    return m_codec.tryDecodeFrame(m_recvBuf, outFrame);
}

static const char* jc_serverboundName(int32_t id) {
    switch (id) {
        case 0x00: return "KeepAlive";
        case 0x01: return "ChatMessage";
        case 0x02: return "UseEntity";
        case 0x03: return "Player(onGround)";
        case 0x04: return "PlayerPosition";
        case 0x05: return "PlayerLook";
        case 0x06: return "PlayerPositionAndLook";
        case 0x07: return "PlayerDigging";
        case 0x08: return "PlayerBlockPlacement";
        case 0x09: return "HeldItemChange";
        case 0x0A: return "Animation";
        case 0x0B: return "EntityAction";
        case 0x0C: return "SteerVehicle";
        case 0x0D: return "CloseWindow";
        case 0x0E: return "ClickWindow";
        case 0x0F: return "ConfirmTransaction";
        case 0x10: return "CreativeInventoryAction";
        case 0x12: return "UpdateSign";
        case 0x13: return "PlayerAbilities";
        case 0x15: return "ClientSettings";
        case 0x16: return "ClientStatus";
        case 0x17: return "PluginMessage";
        default:   return "?";
    }
}

bool JavaConnection::sendFrame(int32_t packetId,
                               const std::vector<uint8_t>& body) {
    static const bool s_c2s = (getenv("JEPROXY_C2S") != nullptr);
    if (s_c2s && m_observedState.load() == JavaConnectionState::Playing) {
        using namespace std::chrono;
        static const auto t0 = steady_clock::now();
        const long ms =
            duration_cast<milliseconds>(steady_clock::now() - t0).count();
        fprintf(stderr, "[JC2S] t=%ldms C->S id=0x%02X (%s) len=%zu\n", ms,
                static_cast<unsigned>(packetId) & 0xFF,
                jc_serverboundName(packetId), body.size());
    }
    std::vector<uint8_t> wire;
    m_codec.encodeFrame(packetId, body, wire);
    return m_tcp.sendAll(wire.data(), wire.size());
}

bool JavaConnection::handleLoginFrame(
    const JavaProtocolCodec::DecodedFrame& frame) {
    JavaPacketReader r(frame.body.data(), frame.body.size());
    try {
        switch (frame.packetId) {
            case JavaLoginClientboundId::Disconnect: {
                std::string reason = r.readUtf8(kMaxStringBytes);
                fprintf(stderr,
                        "[JLOGIN] S->C Disconnect (server kicked us) "
                        "reason-json=%s\n",
                        reason.c_str());
                std::string low = reason;
                for (char& ch : low)
                    if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>(ch + 32);
                if (!m_forwardingActive &&
                    (low.find("forwarding") != std::string::npos ||
                     low.find("bungeecord") != std::string::npos)) {
                    fprintf(stderr,
                            "[JLOGIN] -> looks like a BungeeCord backend; will "
                            "retry with legacy IP forwarding\n");
                    m_sawBungeeKick = true;
                    return false;
                }
                std::wstring text = flattenChatComponent(reason);
                if (text.empty()) text = L"Server disconnected (login)";
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Disconnected;
                ev.text = text;
                pushEvent(std::move(ev));
                setState(JavaConnectionState::Disconnected);
                return false;
            }
            case JavaLoginClientboundId::EncryptionRequest: {
                fprintf(stderr,
                        "[JLOGIN] S->C EncryptionRequest (server is ONLINE-mode) "
                        "-> disconnecting (offline-only client)\n");
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Disconnected;
                ev.text =
                    L"This server requires a premium (online-mode) login, "
                    L"which this client does not support. Only offline-mode "
                    L"servers work (e.g. Aternos with online-mode=false, or an "
                    L"offline Bungee/Velocity/LAN server).";
                pushEvent(std::move(ev));
                setState(JavaConnectionState::Disconnected);
                return false;
            }
            case JavaLoginClientboundId::LoginSuccess: {
                std::string uuid = r.readUtf8(kMaxStringBytes);
                std::string uname = r.readUtf8(kMaxStringBytes);
                fprintf(stderr,
                        "[JLOGIN] S->C LoginSuccess uuid=%s name=%s -> entering "
                        "PLAY state\n",
                        uuid.c_str(), uname.c_str());
                setState(JavaConnectionState::Playing);
                JavaConnectionEvent sys;
                sys.type = JavaConnectionEventType::SystemMessage;
                sys.text = L"Logged in. Waiting for world stream...";
                pushEvent(std::move(sys));
                return true;
            }
            case JavaLoginClientboundId::SetCompression: {
                const int32_t threshold = r.readVarInt();
                fprintf(stderr,
                        "[JLOGIN] S->C SetCompression threshold=%d\n", threshold);
                m_codec.enableCompression(threshold);
                return true;
            }
            default:
                return true;
        }
    } catch (const JavaProtocolError& e) {
        JavaConnectionEvent ev;
        ev.type = JavaConnectionEventType::Disconnected;
        ev.text = L"Protocol error during login";
        pushEvent(std::move(ev));
        setState(JavaConnectionState::Disconnected);
        return false;
    }
}

namespace {

std::string readUuidBytes(JavaPacketReader& r) {
    std::string out;
    out.resize(16);
    int64_t hi = r.readI64();
    int64_t lo = r.readI64();
    uint64_t uhi = static_cast<uint64_t>(hi);
    uint64_t ulo = static_cast<uint64_t>(lo);
    for (int i = 0; i < 8; ++i) {
        out[i] = static_cast<char>((uhi >> (56 - i * 8)) & 0xFF);
    }
    for (int i = 0; i < 8; ++i) {
        out[8 + i] = static_cast<char>((ulo >> (56 - i * 8)) & 0xFF);
    }
    return out;
}

std::string p3_uuidHex(const std::string& uuid) {
    static const char* hx = "0123456789abcdef";
    std::string out;
    out.reserve(uuid.size() * 2);
    for (unsigned char c : uuid) {
        out.push_back(hx[c >> 4]);
        out.push_back(hx[c & 0x0F]);
    }
    return out;
}

std::string p3_base64Decode(const std::string& in) {
    auto dec = [](unsigned char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return -1;
    };
    std::string out;
    int val = 0, bits = -8;
    for (unsigned char c : in) {
        if (c == '=') break;
        const int d = dec(c);
        if (d < 0) continue;
        val = (val << 6) | d;
        bits += 6;
        if (bits >= 0) {
            out.push_back(static_cast<char>((val >> bits) & 0xFF));
            bits -= 8;
        }
    }
    return out;
}

std::string p3_extractSkinUrl(const std::string& json) {
    const size_t skin = json.find("\"SKIN\"");
    if (skin == std::string::npos) return "";
    const size_t urlKey = json.find("\"url\"", skin);
    if (urlKey == std::string::npos) return "";
    size_t q1 = json.find('"', json.find(':', urlKey));
    if (q1 == std::string::npos) return "";
    ++q1;
    std::string url;
    for (size_t i = q1; i < json.size(); ++i) {
        const char c = json[i];
        if (c == '"') break;
        if (c == '\\' && i + 1 < json.size()) {
            url.push_back(json[++i]);
        } else {
            url.push_back(c);
        }
    }
    return url;
}

void applyPlayerListItem(JavaPacketReader& r,
                         std::vector<JavaTabListEntry>& list) {
    const int32_t action = r.readVarInt();
    const int32_t count = r.readVarInt();
    if (count < 0) return;
    for (int32_t i = 0; i < count; ++i) {
        std::string uuid = readUuidBytes(r);
        switch (action) {
            case 0: {
                std::string name = r.readUtf8(kMaxStringBytes);
                const int32_t props = r.readVarInt();
                fprintf(stderr, "[SKIN]\n");
                fprintf(stderr, "[SKIN] uuid=%s\n", p3_uuidHex(uuid).c_str());
                fprintf(stderr, "[SKIN] username=%s\n", name.c_str());
                fprintf(stderr, "[SKIN] propertyCount=%d\n", props);
                std::string skinB64;
                for (int32_t p = 0; p < props && p >= 0; ++p) {
                    const std::string propName = r.readUtf8(kMaxStringBytes);
                    const std::string propValue = r.readUtf8(kMaxStringBytes);
                    const uint8_t isSigned = r.readU8();
                    if (isSigned) {
                        (void)r.readUtf8(kMaxStringBytes);
                    }
                    fprintf(stderr, "[SKIN]   name=%s\n", propName.c_str());
                    fprintf(stderr, "[SKIN]   value(first 64 chars)=%.64s\n",
                            propValue.c_str());
                    fprintf(stderr, "[SKIN]   signed=%s\n",
                            isSigned ? "yes" : "no");
                    if (propName == "textures") {
                        skinB64 = propValue;
                        const std::string decoded = p3_base64Decode(propValue);
                        fprintf(stderr, "[SKIN]   decoded JSON=%s\n",
                                decoded.c_str());
                        fprintf(stderr, "[SKIN]   textures.SKIN.url=%s\n",
                                p3_extractSkinUrl(decoded).c_str());
                    }
                }
                const int32_t gamemode = r.readVarInt();
                const int32_t ping = r.readVarInt();
                const uint8_t hasDisplay = r.readU8();
                if (hasDisplay) {
                    (void)r.readUtf8(kMaxStringBytes);
                }
                JavaTabListEntry entry;
                entry.uuid = uuid;
                entry.name.reserve(name.size());
                for (unsigned char c : name) entry.name.push_back((wchar_t)c);
                entry.ping = ping;
                if (!skinB64.empty()) {
                    entry.skinUrl =
                        p3_extractSkinUrl(p3_base64Decode(skinB64));
                }
                fprintf(stderr,
                        "[TABDBG] PlayerListItem ADD plainName='%s' "
                        "hadDisplayName=%d uuid=%02x%02x%02x%02x..\n",
                        name.c_str(), (int)(hasDisplay != 0),
                        (unsigned char)uuid[0], (unsigned char)uuid[1],
                        (unsigned char)uuid[2], (unsigned char)uuid[3]);
                (void)gamemode;
                bool replaced = false;
                for (auto& e : list) {
                    if (e.uuid == uuid) {
                        e.name = entry.name;
                        e.ping = entry.ping;
                        if (!entry.skinUrl.empty()) e.skinUrl = entry.skinUrl;
                        replaced = true;
                        break;
                    }
                }
                if (!replaced) list.push_back(std::move(entry));
                break;
            }
            case 1: {
                (void)r.readVarInt();
                break;
            }
            case 2: {
                const int32_t ping = r.readVarInt();
                for (auto& e : list) {
                    if (e.uuid == uuid) {
                        e.ping = ping;
                        break;
                    }
                }
                break;
            }
            case 3: {
                const uint8_t hasDisplay = r.readU8();
                if (hasDisplay) {
                    (void)r.readUtf8(kMaxStringBytes);
                }
                break;
            }
            case 4: {
                for (auto it = list.begin(); it != list.end(); ++it) {
                    if (it->uuid == uuid) {
                        list.erase(it);
                        break;
                    }
                }
                break;
            }
            default:
                return;
        }
    }
}

}

bool JavaConnection::handlePlayFrame(
    const JavaProtocolCodec::DecodedFrame& frame) {
    JavaPacketReader r(frame.body.data(), frame.body.size());
    try {
        switch (frame.packetId) {
            case JavaPlayClientboundId::KeepAlive: {
                const int32_t id = r.readVarInt();
                JavaPacketWriter w;
                w.writeVarInt(id);
                if (!sendFrame(JavaPlayServerboundId::KeepAlive,
                               takeBytes(w))) {
                    fprintf(stderr,
                            "[JDISC] PROXY-initiated close: KeepAlive echo write "
                            "to server failed (serverbound socket broke)\n");
                    return false;
                }
                return true;
            }
            case JavaPlayClientboundId::JoinGame: {
                m_javaPlayerEntityId = r.readI32();
                uint8_t gamemode = r.readU8();
                int8_t dimension =
                    static_cast<int8_t>(r.readU8());
                (void)r.readU8();
                (void)r.readU8();
                (void)r.readUtf8(kMaxStringBytes);
                if (r.remaining() >= 1) {
                    (void)r.readU8();
                }
                m_dimension = dimension;
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::JoinGame;
                ev.joinGameMode = gamemode & 0x07;
                ev.joinDimension = dimension;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::ChatMessage: {
                std::string json = r.readUtf8(kMaxStringBytes);
                uint8_t position = 0;
                if (r.remaining() >= 1) {
                    position = r.readU8();
                }
                if (position == 2) return true;
                std::wstring text = flattenChatComponent(json);
                fprintf(stderr, "[JCHAT] raw JSON len=%zu: %s\n", json.size(),
                        json.c_str());
                fprintf(stderr,
                        "[JCHAT] flattened len=%zu first=U+%04X: %ls\n",
                        text.size(),
                        text.empty() ? 0u
                                     : static_cast<unsigned>(text[0]) & 0xFFFFu,
                        text.c_str());
                if (text.empty()) return true;
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Chat;
                ev.text = text;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::PlayerPositionAndLook: {
                int64_t xRaw = r.readI64();
                int64_t yRaw = r.readI64();
                int64_t zRaw = r.readI64();
                int32_t yawRaw = r.readI32();
                int32_t pitchRaw = r.readI32();
                uint8_t posFlags = r.readU8();

                double xD, yD, zD;
                memcpy(&xD, &xRaw, 8);
                memcpy(&yD, &yRaw, 8);
                memcpy(&zD, &zRaw, 8);
                float yawF, pitchF;
                memcpy(&yawF, &yawRaw, 4);
                memcpy(&pitchF, &pitchRaw, 4);

                if (m_havePlayerPos) {
                    if (posFlags & 0x01) xD += m_playerX;
                    if (posFlags & 0x02) yD += m_playerY;
                    if (posFlags & 0x04) zD += m_playerZ;
                    if (posFlags & 0x08) yawF += m_playerYaw;
                    if (posFlags & 0x10) pitchF += m_playerPitch;
                }
                m_playerX = xD;
                m_playerY = yD;
                m_playerZ = zD;
                m_playerYaw = yawF;
                m_playerPitch = pitchF;
                m_havePlayerPos = true;

                {
                    JavaConnectionEvent ev;
                    ev.type = JavaConnectionEventType::PlayerTeleport;
                    ev.posX = xD;
                    ev.posY = yD;
                    ev.posZ = zD;
                    ev.posYaw = yawF;
                    ev.posPitch = pitchF;
                    ev.posFlags = 0;
                    pushEvent(std::move(ev));
                }

                JavaPacketWriter w;
                int64_t xOut, yOut, zOut;
                int32_t yawOut, pitchOut;
                memcpy(&xOut, &xD, 8);
                memcpy(&yOut, &yD, 8);
                memcpy(&zOut, &zD, 8);
                memcpy(&yawOut, &yawF, 4);
                memcpy(&pitchOut, &pitchF, 4);
                w.writeI64(xOut);
                w.writeI64(yOut);
                w.writeI64(zOut);
                w.writeI32(yawOut);
                w.writeI32(pitchOut);
                w.writeU8(0x01);
                return sendFrame(JavaPlayServerboundId::PlayerPositionAndLook,
                                 takeBytes(w));
            }
            case JavaPlayClientboundId::MapChunkBulk: {
                const uint8_t skyLightSent = r.readU8();
                const int32_t columnCount = r.readVarInt();
                if (columnCount < 0 || columnCount > 1024) return true;
                struct ColMeta { int32_t x, z; uint16_t mask; };
                std::vector<ColMeta> metas(static_cast<size_t>(columnCount));
                for (int i = 0; i < columnCount; ++i) {
                    metas[i].x = r.readI32();
                    metas[i].z = r.readI32();
                    metas[i].mask = r.readU16();
                }
                for (int i = 0; i < columnCount; ++i) {
                    JavaConnectionEvent ev;
                    ev.type = JavaConnectionEventType::ChunkData;
                    ev.chunkX = metas[i].x;
                    ev.chunkZ = metas[i].z;
                    if (!p3_decodeOneChunk(r, metas[i].mask, true,
                                           skyLightSent != 0,
                                           ev.chunkBuffer)) {
                        return true;
                    }
                    pushEvent(std::move(ev));
                }
                return true;
            }
            case JavaPlayClientboundId::ChunkData: {
                const int32_t chunkX = r.readI32();
                const int32_t chunkZ = r.readI32();
                const uint8_t groundUp = r.readU8();
                const uint16_t primaryMask = r.readU16();
                const int32_t dataLen = r.readVarInt();
                if (dataLen < 0 ||
                    static_cast<size_t>(dataLen) > r.remaining()) {
                    return true;
                }
                if (primaryMask == 0 && dataLen == 0) {
                    return true;
                }
                const bool hasSkyLight = (m_dimension == 0);
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::ChunkData;
                ev.chunkX = chunkX;
                ev.chunkZ = chunkZ;
                if (!p3_decodeOneChunk(r, primaryMask, groundUp != 0,
                                       hasSkyLight, ev.chunkBuffer)) {
                    return true;
                }
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::SpawnMob: {
                const int32_t eid = r.readVarInt();
                const uint8_t jtype = r.readU8();
                const int32_t x = r.readI32();
                const int32_t y = r.readI32();
                const int32_t z = r.readI32();
                const int8_t yaw = static_cast<int8_t>(r.readU8());
                const int8_t pitch = static_cast<int8_t>(r.readU8());
                const int8_t headYaw = static_cast<int8_t>(r.readU8());
                const int16_t vx = static_cast<int16_t>(r.readU16());
                const int16_t vy = static_cast<int16_t>(r.readU16());
                const int16_t vz = static_cast<int16_t>(r.readU16());
                std::vector<JavaMetaEntry> mdList;
                {
                    JavaInvSlot mdItem;
                    int mdFlags = -1;
                    std::string mdEntries;
                    p3_extractMetadataItem(r, mdItem, &mdFlags, &mdEntries,
                                           &mdList);
                    fprintf(stderr,
                            "[JNPC] SpawnMob eid=%d javaType=%u -> lceType=%d%s "
                            "pos=(%.1f,%.1f,%.1f) flags=%d invisible=%d "
                            "entries=[%s]\n",
                            eid, jtype, p3_javaMobToLce(jtype),
                            p3_javaMobToLce(jtype) < 0
                                ? " (unsupported: no LCE spawn)" : "",
                            x / 32.0, y / 32.0, z / 32.0, mdFlags,
                            mdFlags >= 0 ? ((mdFlags & 0x20) != 0) : -1,
                            mdEntries.c_str());
                }
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::SpawnMob;
                ev.entity.id = eid;
                ev.entity.entityType = p3_javaMobToLce(jtype);
                ev.entity.rawJavaType = jtype;
                ev.entity.x = x;
                ev.entity.y = y;
                ev.entity.z = z;
                ev.entity.yaw = yaw;
                ev.entity.pitch = pitch;
                ev.entity.headYaw = headYaw;
                ev.entity.vx = vx;
                ev.entity.vy = vy;
                ev.entity.vz = vz;
                pushEvent(std::move(ev));
                if (!mdList.empty()) {
                    JavaConnectionEvent mev;
                    mev.type = JavaConnectionEventType::EntityMeta;
                    mev.entity.id = eid;
                    mev.metaEntries = std::move(mdList);
                    pushEvent(std::move(mev));
                }
                return true;
            }
            case JavaPlayClientboundId::SpawnObject: {
                const int32_t eid = r.readVarInt();
                const uint8_t jtype = r.readU8();
                const int32_t x = r.readI32();
                const int32_t y = r.readI32();
                const int32_t z = r.readI32();
                const int8_t pitch = static_cast<int8_t>(r.readU8());
                const int8_t yaw = static_cast<int8_t>(r.readU8());
                const int32_t rawData = r.readI32();
                int16_t vx = 0, vy = 0, vz = 0;
                if (rawData != 0) {
                    vx = static_cast<int16_t>(r.readU16());
                    vy = static_cast<int16_t>(r.readU16());
                    vz = static_cast<int16_t>(r.readU16());
                }
                int data = rawData;
                if (jtype == 2) {
                    data = 1;
                } else if (jtype == 70) {
                    const int jb = rawData & 0xFFF;
                    const int meta = (rawData >> 12) & 0xF;
                    data = JavaBlockIdMap::toLce(static_cast<uint8_t>(jb)) |
                           (meta << 16);
                }
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::SpawnObject;
                ev.entity.id = eid;
                ev.entity.entityType = jtype;
                ev.entity.x = x;
                ev.entity.y = y;
                ev.entity.z = z;
                ev.entity.yaw = yaw;
                ev.entity.pitch = pitch;
                ev.entity.vx = vx;
                ev.entity.vy = vy;
                ev.entity.vz = vz;
                ev.entity.objectData = data;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::DestroyEntities: {
                const uint8_t count = r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityDestroy;
                ev.destroyIds.reserve(count);
                for (int i = 0; i < count; ++i) {
                    ev.destroyIds.push_back(r.readVarInt());
                }
                {
                    std::string ids;
                    for (int32_t d : ev.destroyIds) {
                        char tmp[16];
                        snprintf(tmp, sizeof(tmp), "%s%d",
                                 ids.empty() ? "" : ",", d);
                        ids.append(tmp);
                    }
                    fprintf(stderr, "[JNPC] DestroyEntities count=%u ids=[%s]\n",
                            count, ids.c_str());
                }
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::EntityRelativeMove: {
                const int32_t eid = r.readVarInt();
                const int8_t dx = static_cast<int8_t>(r.readU8());
                const int8_t dy = static_cast<int8_t>(r.readU8());
                const int8_t dz = static_cast<int8_t>(r.readU8());
                if (r.remaining() >= 1) (void)r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityMove;
                ev.entity.id = eid;
                ev.entity.dx = dx;
                ev.entity.dy = dy;
                ev.entity.dz = dz;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::EntityLookAndMove: {
                const int32_t eid = r.readVarInt();
                const int8_t dx = static_cast<int8_t>(r.readU8());
                const int8_t dy = static_cast<int8_t>(r.readU8());
                const int8_t dz = static_cast<int8_t>(r.readU8());
                const int8_t yaw = static_cast<int8_t>(r.readU8());
                const int8_t pitch = static_cast<int8_t>(r.readU8());
                if (r.remaining() >= 1) (void)r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityMove;
                ev.entity.id = eid;
                ev.entity.dx = dx;
                ev.entity.dy = dy;
                ev.entity.dz = dz;
                ev.entity.yaw = yaw;
                ev.entity.pitch = pitch;
                ev.entityHasRot = true;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::EntityTeleport: {
                const int32_t eid = r.readVarInt();
                const int32_t x = r.readI32();
                const int32_t y = r.readI32();
                const int32_t z = r.readI32();
                const int8_t yaw = static_cast<int8_t>(r.readU8());
                const int8_t pitch = static_cast<int8_t>(r.readU8());
                if (r.remaining() >= 1) (void)r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityTeleport;
                ev.entity.id = eid;
                ev.entity.x = x;
                ev.entity.y = y;
                ev.entity.z = z;
                ev.entity.yaw = yaw;
                ev.entity.pitch = pitch;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::Disconnect: {
                std::string json = r.readUtf8(kMaxStringBytes);
                fprintf(stderr,
                        "[JDISC] SERVER-initiated: Disconnect packet (PLAY) "
                        "reason-json=%s\n",
                        json.c_str());
                std::wstring text = flattenChatComponent(json);
                if (text.empty()) text = L"Server disconnected";
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Disconnected;
                ev.text = text;
                pushEvent(std::move(ev));
                setState(JavaConnectionState::Disconnected);
                return false;
            }
            case JavaPlayClientboundId::PlayerListItem: {
                applyPlayerListItem(r, m_tabList);
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::TabListReplaced;
                ev.tabList = m_tabList;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::PlayerListHeaderFooter: {
                std::string headerJson = r.readUtf8(kMaxStringBytes);
                std::string footerJson = r.readUtf8(kMaxStringBytes);
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::TabHeaderFooter;
                ev.tabHeader = flattenChatComponent(headerJson);
                ev.tabFooter = flattenChatComponent(footerJson);
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::SpawnPlayer: {
                const int32_t eid = r.readVarInt();
                uint8_t uuidBytes[16];
                r.readBytes(uuidBytes, 16);
                const int32_t x = r.readI32();
                const int32_t y = r.readI32();
                const int32_t z = r.readI32();
                const int8_t yaw = static_cast<int8_t>(r.readU8());
                const int8_t pitch = static_cast<int8_t>(r.readU8());
                const uint8_t hiHand = r.readU8();
                const uint8_t loHand = r.readU8();
                const int16_t handItem = static_cast<int16_t>((hiHand << 8) | loHand);
                int spFlags = -1;
                std::string spEntries;
                std::vector<JavaMetaEntry> spMeta;
                {
                    JavaInvSlot mdItem;
                    p3_extractMetadataItem(r, mdItem, &spFlags, &spEntries,
                                           &spMeta);
                }
                std::wstring playerName;
                std::string skinUrl;
                bool nameFound = false;
                {
                    const std::string uuidStr(
                        reinterpret_cast<const char*>(uuidBytes), 16);
                    for (const auto& e : m_tabList) {
                        if (e.uuid == uuidStr) {
                            playerName = e.name;
                            skinUrl = e.skinUrl;
                            nameFound = true;
                            break;
                        }
                    }
                }
                fprintf(stderr,
                        "[TABDBG] SpawnPlayer eid=%d resolvedName='%ls' "
                        "found=%d tabSize=%zu uuid=%02x%02x%02x%02x..\n",
                        eid, playerName.c_str(), (int)nameFound,
                        m_tabList.size(), uuidBytes[0], uuidBytes[1],
                        uuidBytes[2], uuidBytes[3]);
                fprintf(stderr,
                        "[JNPC] SpawnPlayer eid=%d uuid=%s name='%ls' "
                        "skinUrl=%d flags=%d invisible=%d entries=[%s]\n",
                        eid,
                        p3_uuidHex(std::string(
                                       reinterpret_cast<const char*>(uuidBytes),
                                       16))
                            .c_str(),
                        playerName.c_str(),
                        (int)!skinUrl.empty(), spFlags,
                        spFlags >= 0 ? ((spFlags & 0x20) != 0) : -1,
                        spEntries.c_str());
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::SpawnPlayer;
                ev.entity.playerName = playerName;
                ev.entity.skinUrl = skinUrl;
                ev.entity.id = eid;
                ev.entity.x = x;
                ev.entity.y = y;
                ev.entity.z = z;
                ev.entity.yaw = yaw;
                ev.entity.pitch = pitch;
                ev.entity.headYaw = yaw;
                ev.equippedItemId = handItem;
                pushEvent(std::move(ev));
                if (!spMeta.empty()) {
                    JavaConnectionEvent mev;
                    mev.type = JavaConnectionEventType::EntityMeta;
                    mev.entity.id = eid;
                    mev.metaEntries = std::move(spMeta);
                    pushEvent(std::move(mev));
                }
                return true;
            }
            case JavaPlayClientboundId::EntityLook: {
                const int32_t eid = r.readVarInt();
                const int8_t yaw = static_cast<int8_t>(r.readU8());
                const int8_t pitch = static_cast<int8_t>(r.readU8());
                if (r.remaining() >= 1) (void)r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityLook;
                ev.entity.id = eid;
                ev.entity.yaw = yaw;
                ev.entity.pitch = pitch;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::EntityHeadLook: {
                const int32_t eid = r.readVarInt();
                const int8_t headYaw = static_cast<int8_t>(r.readU8());
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityHeadLook;
                ev.entity.id = eid;
                ev.entity.headYaw = headYaw;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::EntityVelocity: {
                const int32_t eid = r.readVarInt();
                const int16_t vx = static_cast<int16_t>(r.readU16());
                const int16_t vy = static_cast<int16_t>(r.readU16());
                const int16_t vz = static_cast<int16_t>(r.readU16());
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityVelocity;
                ev.entity.id = eid;
                ev.entity.vx = vx;
                ev.entity.vy = vy;
                ev.entity.vz = vz;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::EntityEquipment: {
                const int32_t eid = r.readVarInt();
                const uint16_t slot = r.readU16();
                const uint8_t hi = r.readU8();
                const uint8_t lo = r.readU8();
                const int16_t itemId = static_cast<int16_t>((hi << 8) | lo);
                uint8_t itemCount = 0;
                int16_t itemDamage = 0;
                if (itemId >= 0) {
                    itemCount = r.readU8();
                    const uint8_t dh = r.readU8();
                    const uint8_t dl = r.readU8();
                    itemDamage = static_cast<int16_t>((dh << 8) | dl);
                    if (r.remaining() >= 1) {
                        const uint8_t nbt = r.readU8();
                        if (nbt != 0) return true;
                    }
                }
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityEquipment;
                ev.entity.id = eid;
                ev.equippedSlot = static_cast<int>(slot);
                ev.equippedItemId = itemId;
                ev.equippedItemCount = itemCount;
                ev.equippedItemDamage = itemDamage;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::EntityMetadata: {
                const int32_t eid = r.readVarInt();
                JavaInvSlot item;
                int metaFlags = -1;
                std::string metaEntries;
                std::vector<JavaMetaEntry> metaList;
                const bool gotItem = p3_extractMetadataItem(
                    r, item, &metaFlags, &metaEntries, &metaList);
                fprintf(stderr,
                        "[JNPC] EntityMetadata eid=%d flags=%d invisible=%d "
                        "entries=[%s]\n",
                        eid, metaFlags,
                        metaFlags >= 0 ? ((metaFlags & 0x20) != 0) : -1,
                        metaEntries.c_str());
                if (!metaList.empty()) {
                    JavaConnectionEvent mev;
                    mev.type = JavaConnectionEventType::EntityMeta;
                    mev.entity.id = eid;
                    mev.entityIsSelf = (eid == m_javaPlayerEntityId);
                    mev.metaEntries = std::move(metaList);
                    pushEvent(std::move(mev));
                }
                if (gotItem && item.id != -1) {
                    JavaConnectionEvent ev;
                    ev.type = JavaConnectionEventType::EntityItemData;
                    ev.entity.id = eid;
                    ev.invSlots.push_back(item);
                    pushEvent(std::move(ev));
                }
                return true;
            }
            case JavaPlayClientboundId::Animation: {
                const int32_t eid = r.readVarInt();
                const uint8_t action = r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Animation;
                ev.entity.id = eid;
                ev.animAction = static_cast<int8_t>(action);
                ev.entityIsSelf = (eid == m_javaPlayerEntityId);
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::EntityStatus: {
                const int32_t eid = r.readI32();
                const uint8_t status = r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::EntityStatus;
                ev.entity.id = eid;
                ev.entityStatus = status;
                ev.entityIsSelf = (eid == m_javaPlayerEntityId);
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::SoundEffect: {
                std::string name = r.readUtf8(kMaxStringBytes);
                const int32_t sx = r.readI32();
                const int32_t sy = r.readI32();
                const int32_t sz = r.readI32();
                const int32_t volBits = r.readI32();
                float vol;
                std::memcpy(&vol, &volBits, sizeof(vol));
                const uint8_t pitch = r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::SoundEffect;
                ev.soundName = std::move(name);
                ev.soundX = sx;
                ev.soundY = sy;
                ev.soundZ = sz;
                ev.soundVolume = vol;
                ev.soundPitch = pitch;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::UpdateTime: {
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::UpdateTime;
                ev.timeAge = r.readI64();
                ev.timeDay = r.readI64();
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::Respawn: {
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Respawn;
                ev.joinDimension = r.readI32();
                m_dimension = ev.joinDimension;
                ev.respawnDifficulty = r.readU8();
                ev.joinGameMode = r.readU8() & 0x07;
                (void)r.readUtf8(kMaxStringBytes);
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::SetExperience: {
                int32_t barBits = r.readI32();
                float bar;
                std::memcpy(&bar, &barBits, sizeof(bar));
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::SetExperience;
                ev.xpBar = bar;
                ev.xpLevel = r.readVarInt();
                ev.xpTotal = r.readVarInt();
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::GameStateChange: {
                const uint8_t reason = r.readU8();
                int32_t valBits = r.readI32();
                float value;
                std::memcpy(&value, &valBits, sizeof(value));
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::GameStateChange;
                ev.gameStateReason = reason;
                ev.gameStateGameMode = static_cast<int>(value) & 0x07;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::PlayerAbilities: {
                const uint8_t flags = r.readU8();
                int32_t fsBits = r.readI32();
                int32_t wsBits = r.readI32();
                float fs, ws;
                std::memcpy(&fs, &fsBits, sizeof(fs));
                std::memcpy(&ws, &wsBits, sizeof(ws));
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::PlayerAbilities;
                ev.abilityFlags = flags;
                ev.flySpeed = fs;
                ev.walkSpeed = ws;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::BlockAction: {
                const int64_t pos = r.readI64();
                const uint8_t b1 = r.readU8();
                const uint8_t b2 = r.readU8();
                const int32_t blockType = r.readVarInt();
                int x = static_cast<int>(pos >> 38);
                int y = static_cast<int>((pos >> 26) & 0xFFF);
                int z = static_cast<int>(pos << 38 >> 38);
                if (x & (1 << 25)) x |= ~((1 << 26) - 1);
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::BlockAction;
                ev.blockX = x;
                ev.blockY = y;
                ev.blockZ = z;
                ev.blockActionB0 = b1;
                ev.blockActionB1 = b2;
                ev.blockId = blockType & 0xFFF;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::Explosion: {
                auto readF32bits = [&]() {
                    int32_t bits = r.readI32();
                    float f;
                    std::memcpy(&f, &bits, sizeof(f));
                    return f;
                };
                const float ex = readF32bits();
                const float ey = readF32bits();
                const float ez = readF32bits();
                const float er = readF32bits();
                const int32_t count = r.readI32();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Explosion;
                ev.explodeX = ex;
                ev.explodeY = ey;
                ev.explodeZ = ez;
                ev.explodeRadius = er;
                if (count < 0 || count > 65536) return true;
                ev.explodeOffsets.reserve(static_cast<size_t>(count) * 3);
                for (int i = 0; i < count; ++i) {
                    ev.explodeOffsets.push_back(
                        static_cast<int8_t>(r.readU8()));
                    ev.explodeOffsets.push_back(
                        static_cast<int8_t>(r.readU8()));
                    ev.explodeOffsets.push_back(
                        static_cast<int8_t>(r.readU8()));
                }
                ev.explodeMotX = readF32bits();
                ev.explodeMotY = readF32bits();
                ev.explodeMotZ = readF32bits();
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::ScoreboardObjective: {
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::ScoreObjective;
                ev.scoreObjective =
                    p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                ev.scoreMode = r.readU8();
                if (ev.scoreMode == 0 || ev.scoreMode == 2) {
                    ev.scoreDisplayName =
                        p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                    (void)r.readUtf8(kMaxStringBytes);
                }
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::UpdateScore: {
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::ScoreUpdate;
                ev.scoreOwner = p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                ev.scoreMode = r.readU8();
                ev.scoreObjective =
                    p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                if (ev.scoreMode != 1) ev.scoreValue = r.readVarInt();
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::DisplayScoreboard: {
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::ScoreDisplay;
                ev.scoreSlot = r.readU8();
                ev.scoreObjective =
                    p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::Teams: {
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::ScoreTeam;
                ev.scoreObjective =
                    p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                ev.scoreMode = r.readU8();
                if (ev.scoreMode == 0 || ev.scoreMode == 2) {
                    ev.scoreDisplayName =
                        p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                    ev.teamPrefix =
                        p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                    ev.teamSuffix =
                        p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes));
                    ev.scoreValue = r.readU8();         // friendly-fire bits
                    (void)r.readUtf8(kMaxStringBytes);  // name tag visibility
                    (void)r.readU8();                   // color (prefix wins)
                }
                if (ev.scoreMode == 0 || ev.scoreMode == 3 ||
                    ev.scoreMode == 4) {
                    const int32_t count = r.readVarInt();
                    for (int32_t i = 0; i < count; ++i)
                        ev.teamPlayers.push_back(
                            p3_decodeNbtUtf8(r.readUtf8(kMaxStringBytes)));
                }
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::AttachEntity: {
                const int32_t rider = r.readI32();
                const int32_t vehicle = r.readI32();
                const bool leash = r.readU8() != 0;
                fprintf(stderr,
                        "[JNPC] AttachEntity rider=%d vehicle=%d leash=%d\n",
                        rider, vehicle, (int)leash);
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::AttachEntity;
                ev.entity.id = rider;
                ev.entityIsSelf = (rider == m_javaPlayerEntityId);
                ev.vehicleId = vehicle;
                ev.attachLeash = leash;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::UpdateSign: {
                const int64_t pos = r.readI64();
                int x = static_cast<int>(pos >> 38);
                int y = static_cast<int>((pos >> 26) & 0xFFF);
                int z = static_cast<int>(pos << 38 >> 38);
                if (x & (1 << 25)) x |= ~((1 << 26) - 1);
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::UpdateSign;
                ev.blockX = x;
                ev.blockY = y;
                ev.blockZ = z;
                ev.signLines.reserve(4);
                for (int i = 0; i < 4; ++i) {
                    std::string json = r.readUtf8(kMaxStringBytes);
                    ev.signLines.push_back(flattenChatComponent(json));
                }
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::OpenSignEditor: {
                const int64_t pos = r.readI64();
                int x = static_cast<int>(pos >> 38);
                int y = static_cast<int>((pos >> 26) & 0xFFF);
                int z = static_cast<int>(pos << 38 >> 38);
                if (x & (1 << 25)) x |= ~((1 << 26) - 1);
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::OpenSignEditor;
                ev.blockX = x;
                ev.blockY = y;
                ev.blockZ = z;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::BlockChange: {
                const int64_t pos = r.readI64();
                const int32_t blockState = r.readVarInt();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::BlockChange;
                int x = static_cast<int>(pos >> 38);
                int y = static_cast<int>((pos >> 26) & 0xFFF);
                int z = static_cast<int>(pos << 38 >> 38);
                if (x & (1 << 25)) x |= ~((1 << 26) - 1);
                ev.blockX = x; ev.blockY = y; ev.blockZ = z;
                ev.blockId = (blockState >> 4) & 0xFFF;
                ev.blockMeta = static_cast<uint8_t>(blockState & 0x0F);
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::MultiBlockChange: {
                const int32_t chunkX = r.readI32();
                const int32_t chunkZ = r.readI32();
                const int32_t recordCount = r.readVarInt();
                if (recordCount < 0 || recordCount > 4096) return true;
                for (int i = 0; i < recordCount; ++i) {
                    const uint8_t xz = r.readU8();
                    const uint8_t y = r.readU8();
                    const int32_t blockState = r.readVarInt();
                    const int relX = (xz >> 4) & 0xF;
                    const int relZ = xz & 0xF;
                    JavaConnectionEvent ev;
                    ev.type = JavaConnectionEventType::BlockChange;
                    ev.blockX = chunkX * 16 + relX;
                    ev.blockY = y;
                    ev.blockZ = chunkZ * 16 + relZ;
                    ev.blockId = (blockState >> 4) & 0xFFF;
                    ev.blockMeta = static_cast<uint8_t>(blockState & 0x0F);
                    pushEvent(std::move(ev));
                }
                return true;
            }
            case JavaPlayClientboundId::UpdateHealth: {
                int32_t hRaw = r.readI32();
                float h; memcpy(&h, &hRaw, 4);
                const int32_t food = r.readVarInt();
                int32_t sRaw = r.readI32();
                float s; memcpy(&s, &sRaw, 4);
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::UpdateHealth;
                ev.healthVal = h;
                ev.foodVal = food;
                ev.saturationVal = s;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::HeldItemChange: {
                const uint8_t slot = r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::HeldItemChange;
                ev.slotIdx = static_cast<int8_t>(slot);
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::WindowItems: {
                const uint8_t windowId = r.readU8();
                const int16_t count = static_cast<int16_t>(r.readU16());
                if (count < 0 || count > 4096) return true;
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::WindowItems;
                ev.winId = windowId;
                ev.invSlots.reserve(static_cast<size_t>(count));
                for (int16_t i = 0; i < count; ++i) {
                    ev.invSlots.push_back(p3_readJavaSlot(r));
                }
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::SetSlot: {
                const int8_t windowId = static_cast<int8_t>(r.readU8());
                const int16_t slot = static_cast<int16_t>(r.readU16());
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::SetSlot;
                ev.winId = windowId;
                ev.slotIndex = slot;
                ev.invSlots.push_back(p3_readJavaSlot(r));
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::OpenWindow: {
                const uint8_t windowId = r.readU8();
                const std::string invType = r.readUtf8(kMaxStringBytes);
                const std::string titleJson = r.readUtf8(kMaxStringBytes);
                const uint8_t slotCount = r.readU8();
                int32_t horseEntityId = 0;
                if (invType == "EntityHorse" && r.remaining() >= 4) {
                    horseEntityId = r.readI32();
                }
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::OpenWindow;
                ev.winId = windowId;
                ev.winType = p3_mapWindowType(invType);
                ev.winIsFurnace = (invType == "minecraft:furnace");
                ev.winSlotCount = slotCount;
                ev.winEntityId = horseEntityId;
                ev.text = flattenChatComponent(titleJson);
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::CloseWindow: {
                const uint8_t windowId = r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::CloseWindow;
                ev.winId = windowId;
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::ConfirmTransaction: {
                const uint8_t windowId = r.readU8();
                const int16_t action = static_cast<int16_t>(r.readU16());
                const uint8_t accepted = r.readU8();
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::ConfirmTransaction;
                ev.winId = windowId;
                ev.slotIndex = action;
                ev.winAccepted = (accepted != 0);
                pushEvent(std::move(ev));
                return true;
            }
            case JavaPlayClientboundId::CraftProgressBar: {
                const uint8_t windowId = r.readU8();
                const int16_t property = static_cast<int16_t>(r.readU16());
                const int16_t value = static_cast<int16_t>(r.readU16());
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::CraftProgress;
                ev.winId = windowId;
                ev.craftProperty = property;
                ev.craftValue = value;
                pushEvent(std::move(ev));
                return true;
            }
            default:
                return true;
        }
    } catch (const JavaProtocolError& e) {
        fprintf(stderr,
                "[JDISC] proxy parse error swallowed on PLAY id=0x%02X: %s "
                "(session continues)\n",
                static_cast<unsigned>(frame.packetId) & 0xFF, e.what());
        return true;
    }
}

bool JavaConnection::drainCommands() {
    std::deque<JavaConnectionCommand> local;
    {
        std::lock_guard<std::mutex> lock(m_commandMutex);
        local.swap(m_commandQueue);
    }
    while (!local.empty()) {
        JavaConnectionCommand cmd = std::move(local.front());
        local.pop_front();
        switch (cmd.type) {
            case JavaConnectionCommandType::SendChat: {
                if (m_observedState.load() != JavaConnectionState::Playing) {
                    JavaConnectionEvent ev;
                    ev.type = JavaConnectionEventType::SystemMessage;
                    ev.text = L"Not connected; chat not sent.";
                    pushEvent(std::move(ev));
                    continue;
                }
                JavaPacketWriter w;
                w.writeUtf8(clipChatUtf8(cmd.chatUtf8));
                if (!sendFrame(JavaPlayServerboundId::ChatMessage,
                               takeBytes(w))) {
                    return false;
                }
                break;
            }
            case JavaConnectionCommandType::SendPlayerPosition: {
                if (m_observedState.load() != JavaConnectionState::Playing) {
                    continue;
                }
                JavaPacketWriter w;
                int64_t xRaw; double xD = cmd.posX; memcpy(&xRaw, &xD, 8);
                int64_t yRaw; double yD = cmd.posY; memcpy(&yRaw, &yD, 8);
                int64_t zRaw; double zD = cmd.posZ; memcpy(&zRaw, &zD, 8);
                int32_t yawRaw; float yF = cmd.posYaw; memcpy(&yawRaw, &yF, 4);
                int32_t pitchRaw; float pF = cmd.posPitch; memcpy(&pitchRaw, &pF, 4);
                w.writeI64(xRaw);
                w.writeI64(yRaw);
                w.writeI64(zRaw);
                w.writeI32(yawRaw);
                w.writeI32(pitchRaw);
                w.writeU8(cmd.onGround ? 0x01 : 0x00);
                if (!sendFrame(JavaPlayServerboundId::PlayerPositionAndLook,
                               takeBytes(w))) {
                    return false;
                }
                m_playerX = cmd.posX;
                m_playerY = cmd.posY;
                m_playerZ = cmd.posZ;
                m_playerYaw = cmd.posYaw;
                m_playerPitch = cmd.posPitch;
                m_havePlayerPos = true;
                break;
            }
            case JavaConnectionCommandType::SendBlockDig: {
                JavaPacketWriter w;
                w.writeU8(cmd.digStatus);
                const uint64_t packed =
                    ((static_cast<uint64_t>(cmd.blockX) & 0x3FFFFFFull) << 38) |
                    ((static_cast<uint64_t>(cmd.blockY) & 0xFFFull) << 26) |
                    (static_cast<uint64_t>(cmd.blockZ) & 0x3FFFFFFull);
                w.writeI64(static_cast<int64_t>(packed));
                w.writeU8(cmd.face <= 5 ? cmd.face : 1);
                {
                    bool ok = sendFrame(JavaPlayServerboundId::PlayerDigging,
                                        takeBytes(w));
                    if (!ok) return false;
                }
                break;
            }
            case JavaConnectionCommandType::SendBlockPlace: {
                JavaPacketWriter w;
                const uint64_t packed =
                    ((static_cast<uint64_t>(cmd.blockX) & 0x3FFFFFFull) << 38) |
                    ((static_cast<uint64_t>(cmd.blockY) & 0xFFFull) << 26) |
                    (static_cast<uint64_t>(cmd.blockZ) & 0x3FFFFFFull);
                w.writeI64(static_cast<int64_t>(packed));
                w.writeU8(cmd.face);
                w.writeU16(static_cast<uint16_t>(cmd.heldItemId));
                if (cmd.heldItemId >= 0) {
                    w.writeU8(cmd.heldItemCount);
                    w.writeU16(static_cast<uint16_t>(cmd.heldItemDamage));
                    w.writeU8(0);
                }
                w.writeU8(cmd.cursorX);
                w.writeU8(cmd.cursorY);
                w.writeU8(cmd.cursorZ);
                {
                    bool ok = sendFrame(
                        JavaPlayServerboundId::PlayerBlockPlacement,
                        takeBytes(w));
                    if (!ok) return false;
                }
                break;
            }
            case JavaConnectionCommandType::SendHeldItem: {
                JavaPacketWriter w;
                w.writeU16(static_cast<uint16_t>(cmd.hotbarSlot));
                if (!sendFrame(JavaPlayServerboundId::HeldItemChange,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendCreativeSlot: {
                JavaPacketWriter w;
                w.writeU16(static_cast<uint16_t>(cmd.hotbarSlot));
                w.writeU16(static_cast<uint16_t>(cmd.heldItemId));
                if (cmd.heldItemId >= 0) {
                    w.writeU8(cmd.heldItemCount);
                    w.writeU16(static_cast<uint16_t>(cmd.heldItemDamage));
                    w.writeU8(0);
                }
                if (!sendFrame(JavaPlayServerboundId::CreativeInventoryAction,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendWindowClick: {
                JavaPacketWriter w;
                w.writeU8(static_cast<uint8_t>(cmd.winId));
                w.writeU16(static_cast<uint16_t>(cmd.winSlot));
                w.writeU8(cmd.winButton);
                w.writeU16(static_cast<uint16_t>(cmd.winActionNum));
                w.writeU8(cmd.winMode);
                w.writeU16(static_cast<uint16_t>(cmd.heldItemId));
                if (cmd.heldItemId >= 0) {
                    w.writeU8(cmd.heldItemCount);
                    w.writeU16(static_cast<uint16_t>(cmd.heldItemDamage));
                    w.writeU8(0);
                }
                if (!sendFrame(JavaPlayServerboundId::ClickWindow,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendCloseWindow: {
                JavaPacketWriter w;
                w.writeU8(static_cast<uint8_t>(cmd.winId));
                if (!sendFrame(JavaPlayServerboundId::CloseWindow,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendClientStatus: {
                JavaPacketWriter w;
                w.writeVarInt(cmd.actionId);
                if (!sendFrame(JavaPlayServerboundId::ClientStatus,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendAnimation: {
                JavaPacketWriter w;
                if (!sendFrame(JavaPlayServerboundId::Animation,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendUseEntity: {
                JavaPacketWriter w;
                w.writeVarInt(cmd.targetEntityId);
                w.writeVarInt(cmd.useEntityAction);
                if (!sendFrame(JavaPlayServerboundId::UseEntity,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendEntityAction: {
                JavaPacketWriter w;
                w.writeVarInt(cmd.playerEntityId);
                w.writeVarInt(cmd.actionId);
                w.writeVarInt(cmd.actionParam);
                if (!sendFrame(JavaPlayServerboundId::EntityAction,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendClientSettings: {
                JavaPacketWriter w;
                w.writeUtf8("en_US");
                w.writeU8(10);
                w.writeU8(0);
                w.writeU8(1);
                w.writeU8(0x7F);
                if (!sendFrame(JavaPlayServerboundId::ClientSettings,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::SendBrand: {
                JavaPacketWriter w;
                w.writeUtf8("MC|Brand");
                w.writeUtf8("LCE Cross Play");
                if (!sendFrame(JavaPlayServerboundId::PluginMessage,
                               takeBytes(w))) return false;
                break;
            }
            case JavaConnectionCommandType::Disconnect:
                return false;
        }
    }
    return true;
}

namespace {
const char* jc_loginPacketName(int32_t id) {
    switch (id) {
        case 0x00: return "Disconnect";
        case 0x01: return "EncryptionRequest";
        case 0x02: return "LoginSuccess";
        case 0x03: return "SetCompression";
        default:   return "Unknown";
    }
}
const char* jc_playPacketName(int32_t id) {
    switch (id) {
        case 0x00: return "KeepAlive";
        case 0x01: return "JoinGame";
        case 0x02: return "ChatMessage";
        case 0x03: return "UpdateTime";
        case 0x05: return "SpawnPosition";
        case 0x08: return "PlayerPositionAndLook";
        case 0x3F: return "PluginMessage";
        case 0x40: return "Disconnect";
        default:   return "(other)";
    }
}

void jc_md5(const uint8_t* msg, size_t len, uint8_t out[16]) {
    static const uint32_t T[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
        0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
        0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
        0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
        0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
        0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
    static const int S[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12,
                              17, 22, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5,
                              9, 14, 20, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16,
                              23, 4, 11, 16, 23, 6, 10, 15, 21, 6, 10, 15, 21, 6,
                              10, 15, 21, 6, 10, 15, 21};
    uint32_t h0 = 0x67452301, h1 = 0xefcdab89, h2 = 0x98badcfe, h3 = 0x10325476;
    std::vector<uint8_t> m(msg, msg + len);
    const uint64_t bits = static_cast<uint64_t>(len) * 8;
    m.push_back(0x80);
    while ((m.size() % 64) != 56) m.push_back(0);
    for (int i = 0; i < 8; ++i) m.push_back(static_cast<uint8_t>(bits >> (8 * i)));
    auto rol = [](uint32_t x, int c) {
        return static_cast<uint32_t>((x << c) | (x >> (32 - c)));
    };
    for (size_t off = 0; off < m.size(); off += 64) {
        uint32_t M[16];
        for (int i = 0; i < 16; ++i) {
            M[i] = static_cast<uint32_t>(m[off + i * 4]) |
                   (static_cast<uint32_t>(m[off + i * 4 + 1]) << 8) |
                   (static_cast<uint32_t>(m[off + i * 4 + 2]) << 16) |
                   (static_cast<uint32_t>(m[off + i * 4 + 3]) << 24);
        }
        uint32_t A = h0, B = h1, C = h2, D = h3;
        for (int i = 0; i < 64; ++i) {
            uint32_t F;
            int g;
            if (i < 16) { F = (B & C) | (~B & D); g = i; }
            else if (i < 32) { F = (D & B) | (~D & C); g = (5 * i + 1) & 15; }
            else if (i < 48) { F = B ^ C ^ D; g = (3 * i + 5) & 15; }
            else { F = C ^ (B | ~D); g = (7 * i) & 15; }
            const uint32_t tmp = D;
            D = C;
            C = B;
            B = B + rol(A + F + T[i] + M[g], S[i]);
            A = tmp;
        }
        h0 += A; h1 += B; h2 += C; h3 += D;
    }
    const uint32_t hs[4] = {h0, h1, h2, h3};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            out[i * 4 + j] = static_cast<uint8_t>(hs[i] >> (8 * j));
}

std::string jc_offlineUuidHex(const std::string& name) {
    const std::string in = "OfflinePlayer:" + name;
    uint8_t h[16];
    jc_md5(reinterpret_cast<const uint8_t*>(in.data()), in.size(), h);
    h[6] = static_cast<uint8_t>((h[6] & 0x0f) | 0x30);
    h[8] = static_cast<uint8_t>((h[8] & 0x3f) | 0x80);
    static const char* hx = "0123456789abcdef";
    std::string out;
    out.reserve(32);
    for (int i = 0; i < 16; ++i) {
        out.push_back(hx[h[i] >> 4]);
        out.push_back(hx[h[i] & 0x0f]);
    }
    return out;
}

std::string jc_bungeeForwardHost(const std::string& host,
                                 const std::string& name) {
    std::string out = host;
    out.push_back('\0');
    out += "127.0.0.1";
    out.push_back('\0');
    out += jc_offlineUuidHex(name);
    out.push_back('\0');
    out += "[]";
    return out;
}
}

void JavaConnection::runWorker() {
    for (int attempt = 0; attempt < 2; ++attempt) {
        m_forwardingActive = (attempt == 1);
        m_sawBungeeKick = false;
        m_codec = JavaProtocolCodec();
        m_recvBuf.clear();

        setState(JavaConnectionState::Connecting);
        if (!m_tcp.connect(m_host, m_port, kConnectTimeoutMs)) {
            JavaConnectionEvent ev;
            ev.type = JavaConnectionEventType::Disconnected;
            ev.text = L"Could not reach server";
            pushEvent(std::move(ev));
            setState(JavaConnectionState::Disconnected);
            m_workerFinished.store(true);
            return;
        }

        setState(JavaConnectionState::LoggingIn);

        const std::string hsHost =
            m_forwardingActive ? jc_bungeeForwardHost(m_host, m_nicknameUtf8)
                               : m_host;
        {
            JavaPacketWriter handshake;
            handshake.writeVarInt(kProtocolVersion1_8);
            handshake.writeUtf8(hsHost);
            handshake.writeU16(m_port);
            handshake.writeVarInt(kHandshakeNextStateLogin);
            fprintf(stderr,
                    "[JLOGIN] C->S Handshake proto=%d host=%s port=%u "
                    "nextState=%d%s\n",
                    kProtocolVersion1_8, m_host.c_str(), (unsigned)m_port,
                    kHandshakeNextStateLogin,
                    m_forwardingActive ? " [+BungeeCord legacy forwarding]" : "");
            if (!sendFrame(JavaHandshakePacketId::Handshake,
                           takeBytes(handshake))) {
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Disconnected;
                ev.text = L"Connection closed before handshake";
                pushEvent(std::move(ev));
                setState(JavaConnectionState::Disconnected);
                m_tcp.close();
                m_workerFinished.store(true);
                return;
            }

            JavaPacketWriter loginStart;
            loginStart.writeUtf8(m_nicknameUtf8);
            fprintf(stderr, "[JLOGIN] C->S LoginStart name=%s\n",
                    m_nicknameUtf8.c_str());
            if (!sendFrame(JavaLoginServerboundId::LoginStart,
                           takeBytes(loginStart))) {
                JavaConnectionEvent ev;
                ev.type = JavaConnectionEventType::Disconnected;
                ev.text = L"Connection closed before login";
                pushEvent(std::move(ev));
                setState(JavaConnectionState::Disconnected);
                m_tcp.close();
                m_workerFinished.store(true);
                return;
            }
        }

        bool inLoginState = true;
        bool keepRunning = true;
        int playLogged = 0;
        while (keepRunning && !m_shutdownRequested.load()) {
            if (!drainCommands()) {
                keepRunning = false;
                break;
            }

            if (!pumpRecv(50)) {
            }

            while (true) {
                JavaProtocolCodec::DecodedFrame frame;
                const JavaProtocolCodec::DecodeStatus st = nextFrame(frame);
                if (st == JavaProtocolCodec::DecodeStatus::NeedMoreData) break;
                if (st == JavaProtocolCodec::DecodeStatus::ProtocolError ||
                    st == JavaProtocolCodec::DecodeStatus::ConnectionBroken) {
                    fprintf(stderr, "[JDISC] %s in %s state -> %s\n",
                            st == JavaProtocolCodec::DecodeStatus::ProtocolError
                                ? "PROXY codec ProtocolError (framing/"
                                  "compression desync)"
                                : "server closed socket (EOF, connection broken)",
                            inLoginState ? "LOGIN" : "PLAY",
                            st == JavaProtocolCodec::DecodeStatus::ProtocolError
                                ? "PROXY-initiated close"
                                : "SERVER-initiated close");
                    JavaConnectionEvent ev;
                    ev.type = JavaConnectionEventType::Disconnected;
                    ev.text = L"Protocol error";
                    pushEvent(std::move(ev));
                    setState(JavaConnectionState::Disconnected);
                    keepRunning = false;
                    break;
                }
                if (inLoginState) {
                    fprintf(stderr,
                            "[JLOGIN] S->C LOGIN id=0x%02X (%s) len=%zu\n",
                            static_cast<unsigned>(frame.packetId) & 0xFF,
                            jc_loginPacketName(frame.packetId),
                            frame.body.size());
                } else if (playLogged < 24) {
                    ++playLogged;
                    fprintf(stderr,
                            "[JLOGIN] S->C PLAY  id=0x%02X (%s) len=%zu\n",
                            static_cast<unsigned>(frame.packetId) & 0xFF,
                            jc_playPacketName(frame.packetId), frame.body.size());
                }
                const bool cont = inLoginState ? handleLoginFrame(frame)
                                               : handlePlayFrame(frame);
                if (!cont) {
                    keepRunning = false;
                    break;
                }
                if (inLoginState &&
                    m_observedState.load() == JavaConnectionState::Playing) {
                    inLoginState = false;
                }
            }
        }

        m_codec.notifyConnectionClosed(m_recvBuf);
        m_tcp.close();

        if (!m_forwardingActive && m_sawBungeeKick &&
            !m_shutdownRequested.load()) {
            fprintf(stderr,
                    "[JLOGIN] reconnecting with BungeeCord legacy IP forwarding "
                    "(offline uuid=%s)\n",
                    jc_offlineUuidHex(m_nicknameUtf8).c_str());
            continue;
        }
        break;
    }

    if (m_observedState.load() != JavaConnectionState::Disconnected) {
        JavaConnectionEvent ev;
        ev.type = JavaConnectionEventType::Disconnected;
        ev.text = L"Disconnected.";
        pushEvent(std::move(ev));
        setState(JavaConnectionState::Disconnected);
    }
    m_workerFinished.store(true);
}