#include "SummonCommand.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/EntityIO.h"
#include "minecraft/world/item/SpawnEggItem.h"
#include "minecraft/world/level/Level.h"

EGameCommand SummonCommand::getId() { return eGameCommand_Summon; }

int SummonCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

static std::wstring toLowerWStr(const std::wstring& str) {
    std::wstring result = str;
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](wchar_t c) { return std::tolower(static_cast<unsigned char>(c)); });
    return result;
}



struct MobNameMapEntry {
    const wchar_t* name;
    int id;
};

static const MobNameMapEntry kMobNameMap[] = {
    
    {L"creeper", 50},
    {L"skeleton", 51},
    {L"spider", 52},
    {L"giant", 53},
    {L"zombie", 54},
    {L"slime", 55},
    {L"ghast", 56},
    {L"pigzombie", 57},
    {L"zombiepigman", 57},
    {L"enderman", 58},
    {L"cavespider", 59},
    {L"silverfish", 60},
    {L"blaze", 61},
    {L"lavaslime", 62},
    {L"magmacube", 62},
    {L"enderdragon", 63},
    {L"witherboss", 64},
    {L"wither", 64},
    {L"bat", 65},
    {L"witch", 66},
    
    {L"pig", 90},
    {L"sheep", 91},
    {L"cow", 92},
    {L"chicken", 93},
    {L"squid", 94},
    {L"wolf", 95},
    {L"mushroomcow", 96},
    {L"mooshroom", 96},
    {L"snowman", 97},
    {L"snowgolem", 97},
    {L"ocelot", 98},
    {L"villagergolem", 99},
    {L"irongolem", 99},
    {L"horse", 100},
    {L"villager", 120},
};

int SummonCommand::resolveEntityIdByName(const std::wstring& name) {
    std::wstring lower = toLowerWStr(name);
    for (const auto& entry : kMobNameMap) {
        if (lower == entry.name) return entry.id;
    }
    
    try {
        return std::stoi(name);
    } catch (...) {
        return -1;
    }
}

void SummonCommand::execute(std::shared_ptr<CommandSender> source,
                            std::vector<uint8_t>& commandData) {
    if (commandData.empty()) {
        source->sendMessage(L"§cUsage: /summon <mob> [x] [y] [z]");
        return;
    }

    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);

        std::wstring mobName = dis.readUTF();
        bool hasCoords = dis.readBoolean();
        double x = 0, y = 0, z = 0;
        if (hasCoords) {
            x = dis.readDouble();
            y = dis.readDouble();
            z = dis.readDouble();
        }

        int mobId = resolveEntityIdByName(mobName);
        if (mobId < 0) {
            source->sendMessage(L"§cUnknown mob: " + mobName);
            return;
        }

        auto sourcePlayer =
            std::dynamic_pointer_cast<ServerPlayer>(source);
        if (sourcePlayer == nullptr) {
            source->sendMessage(L"§cOnly players can summon mobs");
            return;
        }

        if (!hasCoords) {
            
            x = sourcePlayer->x;
            y = sourcePlayer->y;
            z = sourcePlayer->z;
        }

        Level* level = sourcePlayer->level;
        if (level == nullptr) {
            source->sendMessage(L"§cInvalid world");
            return;
        }

        int result = 0;
        std::shared_ptr<Entity> spawned =
            SpawnEggItem::spawnMobAt(level, mobId, x, y, z, &result);
        if (spawned == nullptr) {
            source->sendMessage(L"§cFailed to summon " + mobName);
            return;
        }

        source->sendMessage(L"§aSummoned " + mobName + L" at " +
                            std::to_wstring((int)x) + L", " +
                            std::to_wstring((int)y) + L", " +
                            std::to_wstring((int)z));

    } catch (const std::exception&) {
        source->sendMessage(L"§cError executing summon command");
    }
}
