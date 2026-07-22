#include "Direction.h"

#include "Facing.h"
#include "minecraft/util/Mth.h"

const int Direction::STEP_X[] = {0, -1, 0, 1};

const int Direction::STEP_Z[] = {1, 0, -1, 0};

const std::wstring Direction::NAMES[] = {L"SOUTH", L"WEST", L"NORTH", L"EAST"};


int Direction::DIRECTION_FACING[4] = {Facing::SOUTH, Facing::WEST,
                                      Facing::NORTH, Facing::EAST};


int Direction::FACING_DIRECTION[] = {UNDEFINED, UNDEFINED, NORTH,
                                     SOUTH,     WEST,      EAST};

int Direction::DIRECTION_OPPOSITE[4] = {NORTH, EAST, SOUTH, WEST};


int Direction::DIRECTION_CLOCKWISE[] = {WEST, NORTH, EAST, SOUTH};


int Direction::DIRECTION_COUNTER_CLOCKWISE[] = {EAST, SOUTH, WEST, NORTH};

int Direction::RELATIVE_DIRECTION_FACING[4][6] = {
    
    {Facing::UP, Facing::DOWN, Facing::SOUTH, Facing::NORTH, Facing::EAST,
     Facing::WEST},
    
    {Facing::UP, Facing::DOWN, Facing::EAST, Facing::WEST, Facing::NORTH,
     Facing::SOUTH},
    
    {Facing::UP, Facing::DOWN, Facing::NORTH, Facing::SOUTH, Facing::WEST,
     Facing::EAST},
    
    {Facing::UP, Facing::DOWN, Facing::WEST, Facing::EAST, Facing::SOUTH,
     Facing::NORTH}};

int Direction::getDirection(double xd, double zd) {
    if (Mth::abs((float)xd) > Mth::abs((float)zd)) {
        if (xd > 0) {
            return WEST;
        } else {
            return EAST;
        }
    } else {
        if (zd > 0) {
            return NORTH;
        } else {
            return SOUTH;
        }
    }
}

int Direction::getDirection(int x0, int z0, int x1, int z1) {
    int xd = x0 - x1;
    int zd = z0 - z1;

    return getDirection(xd, zd);
}