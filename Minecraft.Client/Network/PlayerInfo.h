#pragma once

class PlayerInfo {
public:
    std::wstring name;
    int latency;

    PlayerInfo(const std::wstring& name) {
        this->name = name;
        latency = 0;
    }
};