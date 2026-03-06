#pragma once
using namespace std;

#include "Stat.h"

class GeneralStat : public Stat 
{
public:
	GeneralStat(int id, const std::wstring& name, StatFormatter *formatter);
	GeneralStat(int id, const std::wstring& name);
	Stat *postConstruct();
};
