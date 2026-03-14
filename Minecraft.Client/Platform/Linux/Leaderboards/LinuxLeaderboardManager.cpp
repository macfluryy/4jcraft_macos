#include "../../../../Minecraft.World/Platform/stdafx.h"

#include "LinuxLeaderboardManager.h"

LeaderboardManager* LeaderboardManager::m_instance =
    new LinuxLeaderboardManager();  // Singleton instance of the
                                    // LeaderboardManager