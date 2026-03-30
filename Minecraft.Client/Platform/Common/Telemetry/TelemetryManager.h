#pragma once

// On Linux, the Orbis TelemetryEnum.h is already pulled in via stdafx.h ->
// SentientManager.h -> MinecraftTelemetry.h. Using the Windows64 path would
// cause duplicate enum definitions.
#if defined(__linux__)
#include "../../Minecraft.Client/Platform/Linux/Sentient/TelemetryEnum.h"
#else
#include "../../Minecraft.Client/Platform/Windows64/Sentient/TelemetryEnum.h"
#endif
#include "../UI/UIEnums.h"

class CTelemetryManager
{
public:
	virtual int32_t Init();
	virtual int32_t Tick();
	virtual int32_t Flush();

	virtual bool RecordPlayerSessionStart(int iPad);
	virtual bool RecordPlayerSessionExit(int iPad, int exitStatus);
	virtual bool RecordHeartBeat(int iPad);
	virtual bool RecordLevelStart(int iPad, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, int numberOfLocalPlayers, int numberOfOnlinePlayers);
	virtual bool RecordLevelExit(int iPad, ESen_LevelExitStatus levelExitStatus);
	virtual bool RecordLevelSaveOrCheckpoint(int iPad, int saveOrCheckPointID, int saveSizeInBytes);
	virtual bool RecordLevelResume(int iPad, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, int numberOfLocalPlayers, int numberOfOnlinePlayers, int saveOrCheckPointID);
	virtual bool RecordPauseOrInactive(int iPad);
	virtual bool RecordUnpauseOrActive(int iPad);
	virtual bool RecordMenuShown(int iPad, EUIScene menuID, int optionalMenuSubID);
	virtual bool RecordAchievementUnlocked(int iPad, int achievementID, int achievementGamerscore);
	virtual bool RecordMediaShareUpload(int iPad, ESen_MediaDestination mediaDestination, ESen_MediaType mediaType);
	virtual bool RecordUpsellPresented(int iPad, ESen_UpsellID upsellId, int marketplaceOfferID);
	virtual bool RecordUpsellResponded(int iPad, ESen_UpsellID upsellId, int marketplaceOfferID, ESen_UpsellOutcome upsellOutcome);
	virtual bool RecordPlayerDiedOrFailed(int iPad, int lowResMapX, int lowResMapY, int lowResMapZ, int mapID, int playerWeaponID, int enemyWeaponID, ETelemetryChallenges enemyTypeID);
	virtual bool RecordEnemyKilledOrOvercome(int iPad, int lowResMapX, int lowResMapY, int lowResMapZ, int mapID, int playerWeaponID, int enemyWeaponID, ETelemetryChallenges enemyTypeID);
	virtual bool RecordTexturePackLoaded(int iPad, int texturePackId, bool purchased);

	virtual bool RecordSkinChanged(int iPad, int dwSkinId);
	virtual bool RecordBanLevel(int iPad);
	virtual bool RecordUnBanLevel(int iPad);

	virtual int GetMultiplayerInstanceID();
	virtual int GenerateMultiplayerInstanceId();
	virtual void SetMultiplayerInstanceId(int value);

protected:
	float m_initialiseTime;
	float m_lastHeartbeat;
	bool m_bFirstFlush;

	float m_fLevelStartTime[XUSER_MAX_COUNT];

	int m_multiplayerInstanceID;
	int m_levelInstanceID;

	// Helper functions to get the various common settings
	int GetSecondsSinceInitialize();
	int GetMode(int userId);
	int GetSubMode(int userId);
	int GetLevelId(int userId);
	int GetSubLevelId(int userId);
	int GetTitleBuildId();
	int GetLevelInstanceID();
	int GetSingleOrMultiplayer();
	int GetDifficultyLevel(int diff);
	int GetLicense();
	int GetDefaultGameControls();
	int GetAudioSettings(int userId);
	int GetLevelExitProgressStat1();
	int GetLevelExitProgressStat2();
};

extern CTelemetryManager *TelemetryManager;
