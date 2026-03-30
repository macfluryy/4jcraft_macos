#include "Minecraft.World/Header Files/stdafx.h"

#include "WindowsLeaderboardManager.h"

LeaderboardManager* LeaderboardManager::m_instance =
    new WindowsLeaderboardManager();  // Singleton instance of the
                                      // LeaderboardManager