#pragma once

#include "../../../../Minecraft.World/Platform/stdafx.h"

namespace P = Platform;
namespace PC = Platform::Collections;
namespace WF = Windows::Foundation;
namespace WFC = Windows::Foundation::Collections;
namespace MXSL = Microsoft::Xbox::Services::Leaderboard;

class StatParam
{
private:
	std::wstring m_base;
	int m_numArgs;
	
	std::vector<int> m_args;

public:
	StatParam(const std::wstring &base);

	void addArgs(int v1, ...);

	std::vector<std::wstring> *getStats();

};


class DurangoStatsDebugger
{
protected:
	static DurangoStatsDebugger *instance;

	enum 
	{
		ioid_Arrow = 10,

		ioid_Spiderjocky = 49,
		ioid_Creeper = 50,
		ioid_Skeleton,
		ioid_Spider,
		ioid_Giant,
		ioid_Zombie,
		ioid_Slime,
		ioid_Ghast,
		ioid_PigZombie,
		ioid_Enderman,
		ioid_CaveSpider,
		ioid_Silverfish,
		ioid_Blaze,
		ioid_LavaSlime,
		ioid_EnderDragon,

		ioid_Pig  = 90,
		ioid_Sheep,
		ioid_Cow,
		ioid_Chicken,
		ioid_Squid,
		ioid_Wolf,
		ioid_MushroomCow,
		ioid_SnowMan,
		ioid_Ozelot,
		ioid_VillagerGolem,
	};

	DurangoStatsDebugger();

	std::vector<StatParam *> m_stats;

	std::vector<std::wstring> *getStats();

public:
	static DurangoStatsDebugger *Initialize();

	static void PrintStats(int iPad);

private:
	std::vector<std::wstring> m_printQueue;

	void retrieveStats(int iPad);

	typedef struct 
	{
		int			m_iPad;
		std::wstring		m_statName;
		std::wstring		m_score;
	} StatResult;

	CRITICAL_SECTION m_retrievedStatsLock;

	std::vector<StatResult> m_retrievedStats;

	void addRetrievedStat(StatResult result);
};