#include "BasicTreeFeature.h"

#include <stdlib.h>

#include <algorithm>
#include <cmath>

#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "java/Random.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/levelgen/feature/Feature.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/TreeTile.h"

uint8_t BasicTree::axisConversionArray[] = {2, 0, 0, 1, 2, 1};

BasicTree::~BasicTree() {
    delete rnd;

    for (int i = 0; i < foliageCoordsLength; i++) {
        delete[] foliageCoords[i];
    }
    delete[] foliageCoords;
}

BasicTree::BasicTree(bool doUpdate) : Feature(doUpdate) {
    rnd = new Random();
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;
    
    height = 0;
    
    trunkHeight = 0;
    trunkHeightScale = 0.618;
    branchDensity = 1.0;
    branchSlope = 0.381;
    widthScale = 1.0;
    foliageDensity = 1.0;
    trunkWidth = 1;
    heightVariance = 12;
    foliageHeight = 4;
    foliageCoords = nullptr;
    foliageCoordsLength = 0;
}

void BasicTree::prepare() {
    
    
    
    
    trunkHeight = (int)(height * trunkHeightScale);
    if (trunkHeight >= height) trunkHeight = height - 1;
    int clustersPerY = (int)(1.382 + pow(foliageDensity * height / 13.0, 2));
    if (clustersPerY < 1) clustersPerY = 1;
    
    
    int** tempFoliageCoords = new int*[clustersPerY * height];
    for (int i = 0; i < clustersPerY * height; i++) {
        tempFoliageCoords[i] = new int[4];
    }
    int y = origin[1] + height - foliageHeight;
    int clusterCount = 1;
    int trunkTop = origin[1] + trunkHeight;
    int relativeY = y - origin[1];

    tempFoliageCoords[0][0] = origin[0];
    tempFoliageCoords[0][1] = y;
    tempFoliageCoords[0][2] = origin[2];
    tempFoliageCoords[0][3] = trunkTop;
    y--;

    while (relativeY >= 0) {
        int num = 0;

        float shapefac = treeShape(relativeY);
        if (shapefac < 0) {
            y--;
            relativeY--;
            continue;
        }

        
        double originOffset = 0.5;
        while (num < clustersPerY) {
            double radius =
                widthScale * (shapefac * (rnd->nextFloat() + 0.328));
            double angle = rnd->nextFloat() * 2.0 * 3.14159;
            int x = Mth::floor(radius * sin(angle) + origin[0] + originOffset);
            int z = Mth::floor(radius * cos(angle) + origin[2] + originOffset);
            int checkStart[] = {x, y, z};
            int checkEnd[] = {x, y + foliageHeight, z};
            
            if (checkLine(checkStart, checkEnd) == -1) {
                
                
                int checkBranchBase[] = {origin[0], origin[1], origin[2]};
                double distance =
                    sqrt(pow(abs(origin[0] - checkStart[0]), 2.0) +
                         pow(abs(origin[2] - checkStart[2]), 2.0));
                double branchHeight = distance * branchSlope;
                if ((checkStart[1] - branchHeight) > trunkTop) {
                    checkBranchBase[1] = trunkTop;
                } else {
                    checkBranchBase[1] = (int)(checkStart[1] - branchHeight);
                }
                
                if (checkLine(checkBranchBase, checkStart) == -1) {
                    
                    
                    tempFoliageCoords[clusterCount][0] = x;
                    tempFoliageCoords[clusterCount][1] = y;
                    tempFoliageCoords[clusterCount][2] = z;
                    tempFoliageCoords[clusterCount][3] = checkBranchBase[1];
                    clusterCount++;
                }
            }
            num++;
        }
        y--;
        relativeY--;
    }
    
    
    foliageCoordsLength = clusterCount;
    foliageCoords = tempFoliageCoords;
    
    for (int i = clusterCount; i < clustersPerY * height; i++) {
        delete[] tempFoliageCoords[i];
        tempFoliageCoords[i] = nullptr;
    }
    
    
    
    
    
}

void BasicTree::crossection(int x, int y, int z, float radius,
                            uint8_t direction, int material) {
    
    
    
    

    
    
    
    
    
    int rad = (int)(radius + 0.618);
    uint8_t secidx1 = axisConversionArray[direction];
    uint8_t secidx2 = axisConversionArray[direction + 3];
    int center[] = {x, y, z};
    int position[] = {0, 0, 0};
    int offset1 = -rad;
    int offset2 = -rad;
    int thismat;
    position[direction] = center[direction];
    while (offset1 <= rad) {
        position[secidx1] = center[secidx1] + offset1;
        offset2 = -rad;
        while (offset2 <= rad) {
            double thisdistance =
                pow(abs(offset1) + 0.5, 2) + pow(abs(offset2) + 0.5, 2);
            if (thisdistance > radius * radius) {
                offset2++;
                continue;
            }
            position[secidx2] = center[secidx2] + offset2;
            thismat = thisLevel->getTile(position[0], position[1], position[2]);

            if (!((thismat == 0) || (thismat == Tile::leaves_Id))) {
                
                
                offset2++;
                continue;
            }
            placeBlock(thisLevel, position[0], position[1], position[2],
                       material, 0);

            offset2++;
        }
        offset1++;
    }
}

float BasicTree::treeShape(int y) {
    
    
    
    
    
    
    
    if (y < (((float)height) * 0.3)) return (float)-1.618;
    float radius = ((float)height) / ((float)2.0);
    float adjacent = (((float)height) / ((float)2.0)) - y;
    float distance;
    if (adjacent == 0)
        distance = radius;
    else if (abs(adjacent) >= radius)
        distance = (float)0.0;
    else
        distance = (float)sqrt(pow(abs(radius), 2) - pow(abs(adjacent), 2));
    
    distance *= (float)0.5;
    return distance;
}

float BasicTree::foliageShape(int y) {
    
    
    
    
    
    if ((y < 0) || (y >= foliageHeight))
        return (float)-1;
    else if ((y == 0) || (y == (foliageHeight - 1)))
        return (float)2;
    else
        return (float)3;
}

void BasicTree::foliageCluster(int x, int y, int z) {
    
    
    
    int topy = y + foliageHeight;
    int cury = topy - 1;
    float radius;
    
    
    while (cury >= y) {
        radius = foliageShape(cury - y);
        crossection(x, cury, z, radius, (uint8_t)1, Tile::leaves_Id);
        cury--;
    }
}

void BasicTree::limb(int* start, int* end, int material) {
    
    

    
    
    int delta[] = {0, 0, 0};
    uint8_t idx = 0;
    uint8_t primidx = 0;
    while (idx < 3) {
        delta[idx] = end[idx] - start[idx];
        if (abs(delta[idx]) > abs(delta[primidx])) {
            primidx = idx;
        }
        idx++;
    }
    
    if (delta[primidx] == 0) return;
    
    uint8_t secidx1 = axisConversionArray[primidx];
    uint8_t secidx2 = axisConversionArray[primidx + 3];
    
    
    char primsign;
    if (delta[primidx] > 0)
        primsign = 1;
    else
        primsign = -1;
    
    double secfac1 = ((double)delta[secidx1]) / ((double)delta[primidx]);
    double secfac2 = ((double)delta[secidx2]) / ((double)delta[primidx]);
    
    int coordinate[] = {0, 0, 0};
    
    int primoffset = 0;
    int endoffset = delta[primidx] + primsign;
    while (primoffset != endoffset) {
        coordinate[primidx] = Mth::floor(start[primidx] + primoffset + 0.5);
        coordinate[secidx1] =
            Mth::floor(start[secidx1] + (primoffset * secfac1) + 0.5);
        coordinate[secidx2] =
            Mth::floor(start[secidx2] + (primoffset * secfac2) + 0.5);

        int dir = TreeTile::FACING_Y;
        int xdiff = abs(coordinate[0] - start[0]);
        int zdiff = abs(coordinate[2] - start[2]);
        int maxdiff = std::max(xdiff, zdiff);

        if (maxdiff > 0) {
            if (xdiff == maxdiff) {
                dir = TreeTile::FACING_X;
            } else if (zdiff == maxdiff) {
                dir = TreeTile::FACING_Z;
            }
        }
        placeBlock(thisLevel, coordinate[0], coordinate[1], coordinate[2],
                   material, dir);
        primoffset += primsign;
    }
}

void BasicTree::makeFoliage() {
    
    
    int idx = 0;
    int finish = foliageCoordsLength;
    while (idx < finish) {
        int x = foliageCoords[idx][0];
        int y = foliageCoords[idx][1];
        int z = foliageCoords[idx][2];
        foliageCluster(x, y, z);
        idx++;
    }
}

bool BasicTree::trimBranches(int localY) {
    
    
    
    
    
    
    
    if (localY < (height * 0.2))
        return false;
    else
        return true;
}

void BasicTree::makeTrunk() {
    
    int x = origin[0];
    int startY = origin[1];
    int topY = origin[1] + trunkHeight;
    int z = origin[2];
    int startCoord[] = {x, startY, z};
    int endCoord[] = {x, topY, z};
    limb(startCoord, endCoord, Tile::treeTrunk_Id);
    if (trunkWidth == 2) {
        startCoord[0] += 1;
        endCoord[0] += 1;
        limb(startCoord, endCoord, Tile::treeTrunk_Id);
        startCoord[2] += 1;
        endCoord[2] += 1;
        limb(startCoord, endCoord, Tile::treeTrunk_Id);
        startCoord[0] += -1;
        endCoord[0] += -1;
        limb(startCoord, endCoord, Tile::treeTrunk_Id);
    }
}

void BasicTree::makeBranches() {
    
    
    
    int idx = 0;
    int finish = foliageCoordsLength;
    int baseCoord[] = {origin[0], origin[1], origin[2]};
    while (idx < finish) {
        int* coordValues = foliageCoords[idx];
        int endCoord[] = {coordValues[0], coordValues[1], coordValues[2]};
        baseCoord[1] = coordValues[3];
        int localY = baseCoord[1] - origin[1];
        if (trimBranches(localY)) {
            limb(baseCoord, endCoord, Tile::treeTrunk_Id);
        }
        idx++;
    }
}

int BasicTree::checkLine(int* start, int* end) {
    
    
    
    
    

    int delta[] = {0, 0, 0};
    uint8_t idx = 0;
    uint8_t primidx = 0;
    while (idx < 3) {
        delta[idx] = end[idx] - start[idx];
        if (abs(delta[idx]) > abs(delta[primidx])) {
            primidx = idx;
        }
        idx++;
    }
    
    if (delta[primidx] == 0) return -1;
    
    uint8_t secidx1 = axisConversionArray[primidx];
    uint8_t secidx2 = axisConversionArray[primidx + 3];
    
    
    char primsign;  
                    
                    
    if (delta[primidx] > 0)
        primsign = 1;
    else
        primsign = -1;
    
    double secfac1 = ((double)delta[secidx1]) / ((double)delta[primidx]);
    double secfac2 = ((double)delta[secidx2]) / ((double)delta[primidx]);
    
    int coordinate[] = {0, 0, 0};
    
    int primoffset = 0;
    int endoffset = delta[primidx] + primsign;
    int thismat;
    while (primoffset != endoffset) {
        coordinate[primidx] = start[primidx] + primoffset;
        coordinate[secidx1] =
            Mth::floor(start[secidx1] + (primoffset * secfac1));
        coordinate[secidx2] =
            Mth::floor(start[secidx2] + (primoffset * secfac2));
        thismat =
            thisLevel->getTile(coordinate[0], coordinate[1], coordinate[2]);
        if (!((thismat == 0) || (thismat == Tile::leaves_Id))) {
            
            
            break;
        }
        primoffset += primsign;
    }
    
    if (primoffset == endoffset) {
        return -1;
    }
    
    else {
        return abs(primoffset);
    }
}

bool BasicTree::checkLocation() {
    
    

    
    
    
    
    
    
    int startPosition[] = {origin[0], origin[1], origin[2]};
    int endPosition[] = {origin[0], origin[1] + height - 1, origin[2]};

    
    
    if (app.getLevelGenerationOptions() != nullptr) {
        LevelGenerationOptions* levelGenOptions =
            app.getLevelGenerationOptions();
        bool intersects = levelGenOptions->checkIntersects(
            startPosition[0], startPosition[1], startPosition[2],
            endPosition[0], endPosition[1], endPosition[2]);
        if (intersects) {
            
            
            return false;
        }
    }

    
    int baseMaterial = thisLevel->getTile(origin[0], origin[1] - 1, origin[2]);
    if (!((baseMaterial == 2) || (baseMaterial == 3))) {
        return false;
    }
    int allowedHeight = checkLine(startPosition, endPosition);
    
    if (allowedHeight == -1) {
        return true;
    }
    
    else if (allowedHeight < 6) {
        return false;
    }
    
    
    else {
        height = allowedHeight;
        
        return true;
    }
}

void BasicTree::init(double heightInit, double widthInit,
                     double foliageDensityInit) {
    
    
    
    
    
    
    
    
    heightVariance = (int)(heightInit * 12);
    if (heightInit > 0.5) foliageHeight = 5;
    widthScale = widthInit;
    foliageDensity = foliageDensityInit;
}

bool BasicTree::place(Level* level, Random* random, int x, int y, int z) {
    
    
    
    
    
    
    

    
    thisLevel = level;
    int64_t seed = random->nextLong();
    rnd->setSeed(seed);
    
    origin[0] = x;
    origin[1] = y;
    origin[2] = z;
    
    if (height == 0) {
        height = 5 + rnd->nextInt(heightVariance);
    }
    if (!(checkLocation())) {
        
        return false;
    }
    
    
    
    prepare();

    
    makeFoliage();

    
    makeTrunk();

    
    makeBranches();

    

    return true;
}