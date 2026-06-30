#include "TpsCommand.h"

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "minecraft/SharedConstants.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/world/level/Level.h"

EGameCommand TpsCommand::getId() { return eGameCommand_Tps; }

int TpsCommand::getPermissionLevel() { return LEVEL_ALL; }

static std::wstring formatFloat(double value, int decimals) {
    std::wostringstream oss;
    oss.precision(decimals);
    oss.setf(std::ios::fixed);
    oss << value;
    return oss.str();
}

void TpsCommand::execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData) {
    MinecraftServer* server = MinecraftServer::getInstance();
    if (server == nullptr) {
        source->sendMessage(L"§cServer not running");
        return;
    }

    int samples = server->m_tickTimesFilled;
    if (samples <= 0) {
        source->sendMessage(L"§eTPS data not yet available");
        return;
    }

    int64_t totalNs = 0;
    int64_t maxNs = 0;
    for (int i = 0; i < samples; ++i) {
        int64_t v = server->m_tickTimesNs[i];
        totalNs += v;
        if (v > maxNs) maxNs = v;
    }

    double avgMs = (double)totalNs / (double)samples / 1.0e6;
    double maxMs = (double)maxNs / 1.0e6;
    double targetMs =
        1000.0 / (double)SharedConstants::TICKS_PER_SECOND;  // 50ms at 20 TPS

    // Effective TPS: never exceeds the target rate. If a tick is slow, the
    // server cannot tick faster than 1000ms / actualTickMs.
    double tps =
        avgMs < targetMs ? (double)SharedConstants::TICKS_PER_SECOND
                         : 1000.0 / avgMs;

    int totalEntities = 0;
    for (size_t i = 0; i < server->levels.size(); ++i) {
        ServerLevel* level = server->levels[i];
        if (level == nullptr) continue;
        totalEntities += (int)((Level*)level)->getAllEntities().size();
    }

    std::wstring tpsColour = tps >= 19.0   ? L"§a"
                             : tps >= 15.0 ? L"§e"
                                           : L"§c";

    source->sendMessage(L"§e--- Server diagnostics ---");
    source->sendMessage(L"§7Players: §f" +
                        std::to_wstring(server->getPlayers()->getPlayerCount()) +
                        L"  §7Entities: §f" + std::to_wstring(totalEntities));
    source->sendMessage(L"§7Avg tick: §f" + formatFloat(avgMs, 2) +
                        L" ms  §7Peak: §f" + formatFloat(maxMs, 2) + L" ms");
    source->sendMessage(L"§7TPS: " + tpsColour + formatFloat(tps, 2) +
                        L"§r §7/ " +
                        std::to_wstring(SharedConstants::TICKS_PER_SECOND));
}
