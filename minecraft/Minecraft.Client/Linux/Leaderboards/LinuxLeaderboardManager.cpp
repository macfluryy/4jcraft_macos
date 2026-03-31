#include "LinuxLeaderboardManager.h"
#include "Minecraft.Client/Common/Source Files/Leaderboards/LeaderboardManager.h"

LeaderboardManager* LeaderboardManager::m_instance =
    new LinuxLeaderboardManager();  // Singleton instance of the
                                    // LeaderboardManager