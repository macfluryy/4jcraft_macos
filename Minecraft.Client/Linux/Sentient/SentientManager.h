#pragma once
#include "../../../Windows64/Source Files/Sentient/MinecraftTelemetry.h"

class CSentientManager {
public:
    enum ETelemetryEvent {
        eTelemetry_PlayerSessionStart,
        eTelemetry_PlayerSessionExit,
        eTelemetry_HeartBeat,
        eTelemetry_LevelStart,
        eTelemetry_LevelExit,
        eTelemetry_LevelSaveOrCheckpoint,
        eTelemetry_PauseOrInactive,
        eTelemetry_UnpauseOrActive,
        eTelemetry_MenuShown,
        eTelemetry_AchievementUnlocked,
        eTelemetry_MediaShareUpload,
        eTelemetry_UpsellPresented,
        eTelemetry_UpsellResponded,
        eTelemetry_PlayerDiedOrFailed,
        eTelemetry_EnemyKilledOrOvercome,
    };

    int32_t Init();
    int32_t Tick();

    int32_t Flush();

    bool RecordPlayerSessionStart(uint32_t dwUserId);
    bool RecordPlayerSessionExit(uint32_t dwUserId, int exitStatus);
    bool RecordHeartBeat(uint32_t dwUserId);
    bool RecordLevelStart(uint32_t dwUserId, ESen_FriendOrMatch friendsOrMatch,
                          ESen_CompeteOrCoop competeOrCoop, int difficulty,
                          uint32_t numberOfLocalPlayers,
                          uint32_t numberOfOnlinePlayers);
    bool RecordLevelExit(uint32_t dwUserId,
                         ESen_LevelExitStatus levelExitStatus);
    bool RecordLevelSaveOrCheckpoint(uint32_t dwUserId,
                                     int32_t saveOrCheckPointID,
                                     int32_t saveSizeInBytes);
    bool RecordLevelResume(uint32_t dwUserId, ESen_FriendOrMatch friendsOrMatch,
                           ESen_CompeteOrCoop competeOrCoop, int difficulty,
                           uint32_t numberOfLocalPlayers,
                           uint32_t numberOfOnlinePlayers,
                           int32_t saveOrCheckPointID);
    bool RecordPauseOrInactive(uint32_t dwUserId);
    bool RecordUnpauseOrActive(uint32_t dwUserId);
    bool RecordMenuShown(uint32_t dwUserId, int32_t menuID,
                         int32_t optionalMenuSubID);
    bool RecordAchievementUnlocked(uint32_t dwUserId, int32_t achievementID,
                                   int32_t achievementGamerscore);
    bool RecordMediaShareUpload(uint32_t dwUserId,
                                ESen_MediaDestination mediaDestination,
                                ESen_MediaType mediaType);
    bool RecordUpsellPresented(uint32_t dwUserId, ESen_UpsellID upsellId,
                               int32_t marketplaceOfferID);
    bool RecordUpsellResponded(uint32_t dwUserId, ESen_UpsellID upsellId,
                               int32_t marketplaceOfferID,
                               ESen_UpsellOutcome upsellOutcome);
    bool RecordPlayerDiedOrFailed(uint32_t dwUserId, int32_t lowResMapX,
                                  int32_t lowResMapY, int32_t lowResMapZ,
                                  int32_t mapID, int32_t playerWeaponID,
                                  int32_t enemyWeaponID,
                                  ETelemetryChallenges enemyTypeID);
    bool RecordEnemyKilledOrOvercome(uint32_t dwUserId, int32_t lowResMapX,
                                     int32_t lowResMapY, int32_t lowResMapZ,
                                     int32_t mapID, int32_t playerWeaponID,
                                     int32_t enemyWeaponID,
                                     ETelemetryChallenges enemyTypeID);

    bool RecordSkinChanged(uint32_t dwUserId, uint32_t dwSkinId);
    bool RecordBanLevel(uint32_t dwUserId);
    bool RecordUnBanLevel(uint32_t dwUserId);

    int32_t GetMultiplayerInstanceID();
    int32_t GenerateMultiplayerInstanceId();
    void SetMultiplayerInstanceId(int32_t value);

private:
    float m_initialiseTime;
    float m_lastHeartbeat;
    bool m_bFirstFlush;

    float m_fLevelStartTime[XUSER_MAX_COUNT];

    int32_t m_multiplayerInstanceID;
    uint32_t m_levelInstanceID;

    // Helper functions to get the various common settings
    int32_t GetSecondsSinceInitialize();
    int32_t GetMode(uint32_t dwUserId);
    int32_t GetSubMode(uint32_t dwUserId);
    int32_t GetLevelId(uint32_t dwUserId);
    int32_t GetSubLevelId(uint32_t dwUserId);
    int32_t GetTitleBuildId();
    int32_t GetLevelInstanceID();
    int32_t GetSingleOrMultiplayer();
    int32_t GetDifficultyLevel(int32_t diff);
    int32_t GetLicense();
    int32_t GetDefaultGameControls();
    int32_t GetAudioSettings(uint32_t dwUserId);
    int32_t GetLevelExitProgressStat1();
    int32_t GetLevelExitProgressStat2();
};

extern CSentientManager SentientManager;