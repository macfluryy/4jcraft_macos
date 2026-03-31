#pragma once

#include <format>
#include <string>

class Tile;

class User {
public:
    static std::vector<Tile*> allowedTiles;
    static void staticCtor();
    std::wstring name;
    std::wstring sessionId;
    std::wstring mpPassword;

    User(const std::wstring& name, const std::wstring& sessionId);
};