#pragma once
#include <cstddef>
#include <string>

static constexpr size_t kMaxStatusJsonBytes = 256u * 1024u;
static constexpr int kMaxStatusJsonDepth = 64;

struct StatusInfo {
    std::wstring motd;
    int online = -1;
    int max = -1;
    std::wstring version;
    bool valid = false;
};

StatusInfo extractStatus(const std::string& json);