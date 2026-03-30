/************************************************************************/
/* THIS FILE WAS AUTOMATICALLY GENERATED                                */
/* PLEASE DO NOT MODIFY                                                 */
/************************************************************************/
// Generated from Version: 20, on (6/19/2012 9:21:23 AM)

#pragma once

/************************************************************************/
/* STATS                                                                */
/************************************************************************/

// PlayerSessionStart
// Player signed in or joined
bool SenStatPlayerSessionStart(uint32_t dwUserID,
                               int32_t SecondsSinceInitialize, int32_t ModeID,
                               int32_t OptionalSubModeID, int32_t LevelID,
                               int32_t OptionalSubLevelID, int32_t TitleBuildID,
                               int32_t SkeletonDistanceInInches,
                               int32_t EnrollmentType,
                               int32_t NumberOfSkeletonsInView);

// PlayerSessionExit
// Player signed out or left
bool SenStatPlayerSessionExit(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                              int32_t ModeID, int32_t OptionalSubModeID,
                              int32_t LevelID, int32_t OptionalSubLevelID);

// HeartBeat
// Sent every 60 seconds by title
bool SenStatHeartBeat(uint32_t dwUserID, int32_t SecondsSinceInitialize);

// LevelStart
// Level started
bool SenStatLevelStart(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                       int32_t ModeID, int32_t OptionalSubModeID,
                       int32_t LevelID, int32_t OptionalSubLevelID,
                       int32_t LevelInstanceID, int32_t MultiplayerInstanceID,
                       int32_t SingleOrMultiplayer, int32_t FriendsOrMatch,
                       int32_t CompeteOrCoop, int32_t DifficultyLevel,
                       int32_t NumberOfLocalPlayers,
                       int32_t NumberOfOnlinePlayers, int32_t License,
                       int32_t DefaultGameControls, int32_t AudioSettings,
                       int32_t SkeletonDistanceInInches,
                       int32_t NumberOfSkeletonsInView);

// LevelExit
// Level exited
bool SenStatLevelExit(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                      int32_t ModeID, int32_t OptionalSubModeID,
                      int32_t LevelID, int32_t OptionalSubLevelID,
                      int32_t LevelInstanceID, int32_t MultiplayerInstanceID,
                      int32_t LevelExitStatus, int32_t LevelExitProgressStat1,
                      int32_t LevelExitProgressStat2,
                      int32_t LevelDurationInSeconds);

// LevelSaveOrCheckpoint
// Level saved explicitly or implicitly
bool SenStatLevelSaveOrCheckpoint(
    uint32_t dwUserID, int32_t SecondsSinceInitialize, int32_t ModeID,
    int32_t OptionalSubModeID, int32_t LevelID, int32_t OptionalSubLevelID,
    int32_t LevelInstanceID, int32_t MultiplayerInstanceID,
    int32_t LevelExitProgressStat1, int32_t LevelExitProgressStat2,
    int32_t LevelDurationInSeconds, int32_t SaveOrCheckPointID);

// LevelResume
// Level resumed from a save or restarted at a checkpoint
bool SenStatLevelResume(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                        int32_t ModeID, int32_t OptionalSubModeID,
                        int32_t LevelID, int32_t OptionalSubLevelID,
                        int32_t LevelInstanceID, int32_t MultiplayerInstanceID,
                        int32_t SingleOrMultiplayer, int32_t FriendsOrMatch,
                        int32_t CompeteOrCoop, int32_t DifficultyLevel,
                        int32_t NumberOfLocalPlayers,
                        int32_t NumberOfOnlinePlayers, int32_t License,
                        int32_t DefaultGameControls, int32_t SaveOrCheckPointID,
                        int32_t AudioSettings, int32_t SkeletonDistanceInInches,
                        int32_t NumberOfSkeletonsInView);

// PauseOrInactive
// Player paused game or has become inactive, level and mode are for what the
// player is leaving
bool SenStatPauseOrInactive(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                            int32_t ModeID, int32_t OptionalSubModeID,
                            int32_t LevelID, int32_t OptionalSubLevelID,
                            int32_t LevelInstanceID,
                            int32_t MultiplayerInstanceID);

// UnpauseOrActive
// Player unpaused game or has become active, level and mode are for what the
// player is entering into
bool SenStatUnpauseOrActive(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                            int32_t ModeID, int32_t OptionalSubModeID,
                            int32_t LevelID, int32_t OptionalSubLevelID,
                            int32_t LevelInstanceID,
                            int32_t MultiplayerInstanceID);

// MenuShown
// A menu screen or major menu area has been shown
bool SenStatMenuShown(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                      int32_t ModeID, int32_t OptionalSubModeID,
                      int32_t LevelID, int32_t OptionalSubLevelID,
                      int32_t MenuID, int32_t OptionalMenuSubID,
                      int32_t LevelInstanceID, int32_t MultiplayerInstanceID);

// AchievementUnlocked
// An achievement was unlocked
bool SenStatAchievementUnlocked(
    uint32_t dwUserID, int32_t SecondsSinceInitialize, int32_t ModeID,
    int32_t OptionalSubModeID, int32_t LevelID, int32_t OptionalSubLevelID,
    int32_t LevelInstanceID, int32_t MultiplayerInstanceID,
    int32_t AchievementID, int32_t AchievementGamerscore);

// MediaShareUpload
// The user uploaded something to Kinect Share
bool SenStatMediaShareUpload(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                             int32_t ModeID, int32_t OptionalSubModeID,
                             int32_t LevelID, int32_t OptionalSubLevelID,
                             int32_t LevelInstanceID,
                             int32_t MultiplayerInstanceID,
                             int32_t MediaDestination, int32_t MediaType);

// UpsellPresented
// The user is shown an upsell to purchase something
bool SenStatUpsellPresented(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                            int32_t ModeID, int32_t OptionalSubModeID,
                            int32_t LevelID, int32_t OptionalSubLevelID,
                            int32_t LevelInstanceID,
                            int32_t MultiplayerInstanceID, int32_t UpsellID,
                            int32_t MarketplaceOfferID);

// UpsellResponded
// The user responded to the upsell
bool SenStatUpsellResponded(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                            int32_t ModeID, int32_t OptionalSubModeID,
                            int32_t LevelID, int32_t OptionalSubLevelID,
                            int32_t LevelInstanceID,
                            int32_t MultiplayerInstanceID, int32_t UpsellID,
                            int32_t MarketplaceOfferID, int32_t UpsellOutcome);

// PlayerDiedOrFailed
// The player died or failed a challenge - can be used for many types of failure
bool SenStatPlayerDiedOrFailed(
    uint32_t dwUserID, int32_t ModeID, int32_t OptionalSubModeID,
    int32_t LevelID, int32_t OptionalSubLevelID, int32_t LevelInstanceID,
    int32_t MultiplayerInstanceID, int32_t LowResMapX, int32_t LowResMapY,
    int32_t LowResMapZ, int32_t MapID, int32_t PlayerWeaponID,
    int32_t EnemyWeaponID, int32_t EnemyTypeID, int32_t SecondsSinceInitialize,
    int32_t CopyOfSecondsSinceInitialize);

// EnemyKilledOrOvercome
// The player killed an enemy or overcame or solved a major challenge
bool SenStatEnemyKilledOrOvercome(
    uint32_t dwUserID, int32_t ModeID, int32_t OptionalSubModeID,
    int32_t LevelID, int32_t OptionalSubLevelID, int32_t LevelInstanceID,
    int32_t MultiplayerInstanceID, int32_t LowResMapX, int32_t LowResMapY,
    int32_t LowResMapZ, int32_t MapID, int32_t PlayerWeaponID,
    int32_t EnemyWeaponID, int32_t EnemyTypeID, int32_t SecondsSinceInitialize,
    int32_t CopyOfSecondsSinceInitialize);

// SkinChanged
// The player has changed their skin, level and mode are for what the player is
// currently in
bool SenStatSkinChanged(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                        int32_t ModeID, int32_t OptionalSubModeID,
                        int32_t LevelID, int32_t OptionalSubLevelID,
                        int32_t LevelInstanceID, int32_t MultiplayerInstanceID,
                        int32_t SkinID);

// BanLevel
// The player has banned a level, level and mode are for what the player is
// currently in and banning
bool SenStatBanLevel(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                     int32_t ModeID, int32_t OptionalSubModeID, int32_t LevelID,
                     int32_t OptionalSubLevelID, int32_t LevelInstanceID,
                     int32_t MultiplayerInstanceID);

// UnBanLevel
// The player has ubbanned a level, level and mode are for what the player is
// currently in and unbanning
bool SenStatUnBanLevel(uint32_t dwUserID, int32_t SecondsSinceInitialize,
                       int32_t ModeID, int32_t OptionalSubModeID,
                       int32_t LevelID, int32_t OptionalSubLevelID,
                       int32_t LevelInstanceID, int32_t MultiplayerInstanceID);
