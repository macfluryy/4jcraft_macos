#pragma once

#include <string>

#include "Stat.h"

class StatFormatter;

class GeneralStat : public Stat {
public:
    GeneralStat(int id, const std::wstring& name, StatFormatter* formatter);
    GeneralStat(int id, const std::wstring& name);
    Stat* postConstruct();
};
