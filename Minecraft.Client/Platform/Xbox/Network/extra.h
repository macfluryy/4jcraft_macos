#pragma once
#include <cstdint>
#include <limits>

constexpr int MINECRAFT_NET_MAX_PLAYERS = 8;

static_assert(MINECRAFT_NET_MAX_PLAYERS <= std::numeric_limits<std::uint8_t>::max(),
              "MINECRAFT_NET_MAX_PLAYERS must fit in the 8-bit network protocol");
