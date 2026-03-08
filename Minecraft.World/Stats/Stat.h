#pragma once


#include "StatFormatter.h"
#include "GenericStats.h"

class DecimalFormat;

class Stat
{
public:
	const int id;
    const std::wstring name;
    bool awardLocallyOnly;

private:
	const StatFormatter *formatter;
	void _init();

public:
	Stat(int id, const std::wstring& name, StatFormatter *formatter);
    Stat(int id, const std::wstring& name);
    Stat *setAwardLocallyOnly();

    virtual Stat *postConstruct();
    virtual bool isAchievement();
    std::wstring format(int value);

private:
	//static NumberFormat *numberFormat;

public:
	class DefaultFormat : public StatFormatter
	{
	public:
		std::wstring format(int value);
    } static *defaultFormatter;

private:
	static DecimalFormat *decimalFormat;

public:

	class TimeFormatter : public StatFormatter
	{
	public:
		std::wstring format(int value);
    } static *timeFormatter;

    class DistanceFormatter : public StatFormatter
	{
	public:
		std::wstring format(int cm);
    } static *distanceFormatter;

    std::wstring toString();

public:
	// 4J-JEV, for Durango stats
	virtual void handleParamBlob(std::shared_ptr<LocalPlayer> plr, byteArray param) { app.DebugPrintf("'Stat.h', Unhandled AwardStat blob.\n"); return; }
};
