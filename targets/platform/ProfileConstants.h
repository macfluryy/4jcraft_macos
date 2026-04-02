#pragma once

#include <cstdint>

inline constexpr int TUTORIAL_PROFILE_STORAGE_BITS = 512;
inline constexpr int TUTORIAL_PROFILE_STORAGE_BYTES =
    TUTORIAL_PROFILE_STORAGE_BITS / 8;

inline constexpr int MAX_FAVORITE_SKINS = 10;

inline constexpr std::uint32_t GAMESETTING_CLOUDS = 0x00000001;
inline constexpr std::uint32_t GAMESETTING_ONLINE = 0x00000002;
inline constexpr std::uint32_t GAMESETTING_FRIENDSOFFRIENDS = 0x00000008;
inline constexpr std::uint32_t GAMESETTING_DISPLAYUPDATEMSG = 0x00000030;
inline constexpr std::uint32_t GAMESETTING_BEDROCKFOG = 0x00000040;
inline constexpr std::uint32_t GAMESETTING_DISPLAYHUD = 0x00000080;
inline constexpr std::uint32_t GAMESETTING_DISPLAYHAND = 0x00000100;
inline constexpr std::uint32_t GAMESETTING_CUSTOMSKINANIM = 0x00000200;
inline constexpr std::uint32_t GAMESETTING_DEATHMESSAGES = 0x00000400;
inline constexpr std::uint32_t GAMESETTING_UISIZE = 0x00001800;
inline constexpr std::uint32_t GAMESETTING_UISIZE_SPLITSCREEN = 0x00006000;
inline constexpr std::uint32_t GAMESETTING_ANIMATEDCHARACTER = 0x00008000;
inline constexpr std::uint32_t GAMESETTING_PS3EULAREAD = 0x00010000;
inline constexpr std::uint32_t GAMESETTING_PSVITANETWORKMODEADHOC = 0x00020000;

inline constexpr unsigned char MINECRAFT_LANGUAGE_DEFAULT = 0x00;
