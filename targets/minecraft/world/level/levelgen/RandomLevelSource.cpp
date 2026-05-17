#include "RandomLevelSource.h"

#include <stdlib.h>
#include <string.h>

#include <cstdint>

#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "util/Timer.h"
#include "java/Random.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/entity/MobCategory.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/MobSpawner.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/biome/BiomeSource.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/levelgen/CanyonFeature.h"
#include "minecraft/world/level/levelgen/LargeCaveFeature.h"
#include "minecraft/world/level/levelgen/LargeFeature.h"
#include "minecraft/world/level/levelgen/feature/LakeFeature.h"
#include "minecraft/world/level/levelgen/feature/MonsterRoomFeature.h"
#include "minecraft/world/level/levelgen/structure/MineShaftFeature.h"
#include "minecraft/world/level/levelgen/structure/RandomScatteredLargeFeature.h"
#include "minecraft/world/level/levelgen/structure/StrongholdFeature.h"
#include "minecraft/world/level/levelgen/structure/VillageFeature.h"
#include "minecraft/world/level/levelgen/synth/PerlinNoise.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/tile/HeavyTile.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/phys/Vec3.h"

const double RandomLevelSource::SNOW_SCALE = 0.3;
const double RandomLevelSource::SNOW_CUTOFF = 0.5;

RandomLevelSource::RandomLevelSource(Level* level, int64_t seed,
                                     bool generateStructures)
    : generateStructures(generateStructures) {
    m_XZSize = level->getLevelData()->getXZSize();
#if defined(_LARGE_WORLDS)
    level->getLevelData()->getMoatFlags(&m_classicEdgeMoat, &m_smallEdgeMoat,
                                        &m_mediumEdgeMoat);
#endif
    caveFeature = new LargeCaveFeature();
    strongholdFeature = new StrongholdFeature();
    villageFeature = new VillageFeature(m_XZSize);
    mineShaftFeature = new MineShaftFeature();
    scatteredFeature = new RandomScatteredLargeFeature();
    canyonFeature = new CanyonFeature();

    this->level = level;
    amplified =
        level->getLevelData()->getGenerator() == LevelType::lvl_amplified;

    random = new Random(seed);
    pprandom = new Random(
        seed);  // 4J - added, so that we can have a separate random for doing
                // post-processing in parallel with creation
    lperlinNoise1 = new PerlinNoise(random, 16);
    lperlinNoise2 = new PerlinNoise(random, 16);
    perlinNoise1 = new PerlinNoise(random, 8);
    perlinNoise3 = new PerlinNoise(random, 4);

    scaleNoise = new PerlinNoise(random, 10);
    depthNoise = new PerlinNoise(random, 16);

    if (FLOATING_ISLANDS) {
        floatingIslandScale = new PerlinNoise(random, 10);
        floatingIslandNoise = new PerlinNoise(random, 16);
    } else {
        floatingIslandScale = nullptr;
        floatingIslandNoise = nullptr;
    }

    forestNoise = new PerlinNoise(random, 8);
}

RandomLevelSource::~RandomLevelSource() {
    delete caveFeature;
    delete strongholdFeature;
    delete villageFeature;
    delete mineShaftFeature;
    delete scatteredFeature;
    delete canyonFeature;

    delete random;
    ;
    delete lperlinNoise1;
    delete lperlinNoise2;
    delete perlinNoise1;
    delete perlinNoise3;

    delete scaleNoise;
    delete depthNoise;

    if (FLOATING_ISLANDS) {
        delete floatingIslandScale;
        delete floatingIslandNoise;
    }

    delete forestNoise;
}

int g_numPrepareHeightCalls = 0;
time_util::clock::duration g_totalPrepareHeightsTime{};
time_util::clock::duration g_averagePrepareHeightsTime{};

#if defined(_LARGE_WORLDS)

int RandomLevelSource::getMinDistanceToEdge(int xxx, int zzz, int worldSize,
                                            float falloffStart) {
    // Get distance to edges of world in x
    // we have to do a proper line dist check here
    int min = -worldSize / 2;
    int max = (worldSize / 2) - 1;

    // 	// only check if either x or z values are within the falloff
    // 	if(xxx > (min - falloffStart)

    Vec3 topLeft(min, 0, min);
    Vec3 topRight(max, 0, min);
    Vec3 bottomLeft(min, 0, max);
    Vec3 bottomRight(max, 0, max);

    float closest = falloffStart;
    float dist;
    // make sure we're in range of the edges before we do a full distance check
    if ((xxx > (min - falloffStart) && xxx < (min + falloffStart)) ||
        (xxx > (max - falloffStart) && xxx < (max + falloffStart))) {
        Vec3 point(xxx, 0, zzz);

        if (xxx > 0)
            dist = point.distanceFromLine(topRight, bottomRight);
        else
            dist = point.distanceFromLine(topLeft, bottomLeft);
        closest = dist;
    }

    // make sure we're in range of the edges before we do a full distance check
    if ((zzz > (min - falloffStart) && zzz < (min + falloffStart)) ||
        (zzz > (max - falloffStart) && zzz < (max + falloffStart))) {
        Vec3 point(xxx, 0, zzz);

        if (zzz > 0)
            dist = point.distanceFromLine(bottomLeft, bottomRight);
        else
            dist = point.distanceFromLine(topLeft, topRight);
        if (dist < closest) closest = dist;
    }

    return closest;
}

float RandomLevelSource::getHeightFalloff(int xxx, int zzz, int* pEMin) {
    ///////////////////////////////////////////////////////////////////
    // 4J - add this chunk of code to make land "fall-off" at the edges of
    // a finite world - size of that world is currently hard-coded in here
    const int worldSize = m_XZSize * 16;
    const int falloffStart =
        32;  // chunks away from edge were we start doing fall-off
    const float falloffMax =
        128.0f;  // max value we need to get to falloff by the edge of the map

    float comp = 0.0f;
    int emin = getMinDistanceToEdge(xxx, zzz, worldSize, falloffStart);
    // check if we have a larger world that should have moats
    int expandedWorldSizes[3] = {LEVEL_WIDTH_CLASSIC * 16,
                                 LEVEL_WIDTH_SMALL * 16,
                                 LEVEL_WIDTH_MEDIUM * 16};
    bool expandedMoatValues[3] = {m_classicEdgeMoat, m_smallEdgeMoat,
                                  m_mediumEdgeMoat};
    for (int i = 0; i < 3; i++) {
        if (expandedMoatValues[i] && (worldSize > expandedWorldSizes[i])) {
            // this world has been expanded, with moat settings, so we need
            // fallofs at this edges too
            int eminMoat = getMinDistanceToEdge(xxx, zzz, expandedWorldSizes[i],
                                                falloffStart);
            if (eminMoat < emin) {
                emin = eminMoat;
            }
        }
    }

    // Calculate how much we want the world to fall away, if we're in the
    // defined region to do so
    if (emin < falloffStart) {
        int falloff = falloffStart - emin;
        comp = ((float)falloff / (float)falloffStart) * falloffMax;
    }
    *pEMin = emin;
    return comp;
    // 4J - end of extra code
    ///////////////////////////////////////////////////////////////////
}

#else

// MGH  - go back to using the simpler version for PS3/vita/360, as it was
// causing a lot of slow down on the tuturial generation
float RandomLevelSource::getHeightFalloff(int xxx, int zzz, int* pEMin) {
    ///////////////////////////////////////////////////////////////////
    // 4J - add this chunk of code to make land "fall-off" at the edges of
    // a finite world - size of that world is currently hard-coded in here
    const int worldSize = m_XZSize * 16;
    const int falloffStart =
        32;  // chunks away from edge were we start doing fall-off
    const float falloffMax =
        128.0f;  // max value we need to get to falloff by the edge of the map

    // Get distance to edges of world in x
    int xxx0 = xxx + (worldSize / 2);
    if (xxx0 < 0) xxx0 = 0;
    int xxx1 = ((worldSize / 2) - 1) - xxx;
    if (xxx1 < 0) xxx1 = 0;

    // Get distance to edges of world in z
    int zzz0 = zzz + (worldSize / 2);
    if (zzz0 < 0) zzz0 = 0;
    int zzz1 = ((worldSize / 2) - 1) - zzz;
    if (zzz1 < 0) zzz1 = 0;

    // Get min distance to any edge
    int emin = xxx0;
    if (xxx1 < emin) emin = xxx1;
    if (zzz0 < emin) emin = zzz0;
    if (zzz1 < emin) emin = zzz1;

    float comp = 0.0f;

    // Calculate how much we want the world to fall away, if we're in the
    // defined region to do so
    if (emin < falloffStart) {
        int falloff = falloffStart - emin;
        comp = ((float)falloff / (float)falloffStart) * falloffMax;
    }
    // 4J - end of extra code
    ///////////////////////////////////////////////////////////////////
    *pEMin = emin;
    return comp;
}

#endif

void RandomLevelSource::prepareHeights(int xOffs, int zOffs,
                                       std::vector<uint8_t>& blocks) {
    int xChunks = 16 / CHUNK_WIDTH;
    int yChunks = Level::genDepth / CHUNK_HEIGHT;
    int waterHeight = level->seaLevel;

    int xSize = xChunks + 1;
    int ySize = Level::genDepth / CHUNK_HEIGHT + 1;
    int zSize = xChunks + 1;

    std::vector<Biome*> biomes;  // 4J created locally here for thread safety,
                                 // java has this as a class member

    level->getBiomeSource()->getRawBiomeBlock(biomes, xOffs * CHUNK_WIDTH - 2,
                                              zOffs * CHUNK_WIDTH - 2,
                                              xSize + 5, zSize + 5);

    std::vector<double>
        buffer;  // 4J - used to be declared with class level scope but
                 // tidying up for thread safety reasons
    buffer = getHeights(buffer, xOffs * xChunks, 0, zOffs * xChunks, xSize,
                        ySize, zSize, biomes);

    time_util::Timer timer;
    for (int xc = 0; xc < xChunks; xc++) {
        for (int zc = 0; zc < xChunks; zc++) {
            for (int yc = 0; yc < yChunks; yc++) {
                double yStep = 1 / (double)CHUNK_HEIGHT;
                double s0 =
                    buffer[((xc + 0) * zSize + (zc + 0)) * ySize + (yc + 0)];
                double s1 =
                    buffer[((xc + 0) * zSize + (zc + 1)) * ySize + (yc + 0)];
                double s2 =
                    buffer[((xc + 1) * zSize + (zc + 0)) * ySize + (yc + 0)];
                double s3 =
                    buffer[((xc + 1) * zSize + (zc + 1)) * ySize + (yc + 0)];

                double s0a =
                    (buffer[((xc + 0) * zSize + (zc + 0)) * ySize + (yc + 1)] -
                     s0) *
                    yStep;
                double s1a =
                    (buffer[((xc + 0) * zSize + (zc + 1)) * ySize + (yc + 1)] -
                     s1) *
                    yStep;
                double s2a =
                    (buffer[((xc + 1) * zSize + (zc + 0)) * ySize + (yc + 1)] -
                     s2) *
                    yStep;
                double s3a =
                    (buffer[((xc + 1) * zSize + (zc + 1)) * ySize + (yc + 1)] -
                     s3) *
                    yStep;

                for (int y = 0; y < CHUNK_HEIGHT; y++) {
                    double xStep = 1 / (double)CHUNK_WIDTH;

                    double _s0 = s0;
                    double _s1 = s1;
                    double _s0a = (s2 - s0) * xStep;
                    double _s1a = (s3 - s1) * xStep;

                    for (int x = 0; x < CHUNK_WIDTH; x++) {
                        int offs = (unsigned)(x + (unsigned)xc * CHUNK_WIDTH)
                                       << Level::genDepthBitsPlusFour |
                                   ((unsigned)zc * CHUNK_WIDTH)
                                       << Level::genDepthBits |
                                   (yc * CHUNK_HEIGHT + y);
                        int step = 1 << Level::genDepthBits;
                        offs -= step;
                        double zStep = 1 / (double)CHUNK_WIDTH;

                        double val = _s0;
                        double vala = (_s1 - _s0) * zStep;
                        val -= vala;
                        for (int z = 0; z < CHUNK_WIDTH; z++) {
                            // 4J Stu - I have removed all uses of the new
                            // getHeightFalloff function for now as we had some
                            // problems with PS3/PSVita world generation I have
                            // fixed the non large worlds method, however we
                            // will be happier if the current builds go out with
                            // completely old code We can put the new code back
                            // in mid-november 2014 once those PS3/Vita builds
                            // are gone (and the PS4 doesn't have world
                            // enlarging in these either anyway)
                            int xxx = ((xOffs * 16) + x + (xc * CHUNK_WIDTH));
                            int zzz = ((zOffs * 16) + z + (zc * CHUNK_WIDTH));
                            int emin;
                            float comp = getHeightFalloff(xxx, zzz, &emin);

                            // 4J - slightly rearranged this code (as of
                            // java 1.0.1 merge) to better fit with changes
                            // we've made edge-of-world things - original sets
                            // blocks[offs += step] directly here rather than
                            // setting a tileId
                            int tileId = 0;
                            // 4J - this comparison used to just be with 0.0f
                            // but is now varied by block above
                            if ((val += vala) > comp) {
                                tileId = (uint8_t)Tile::stone_Id;
                            } else if (yc * CHUNK_HEIGHT + y < waterHeight) {
                                tileId = (uint8_t)Tile::calmWater_Id;
                            }

                            // 4J - more extra code to make sure that the column
                            // at the edge of the world is just water & rock, to
                            // match the infinite sea that continues on after
                            // the edge of the world.

                            if (emin == 0) {
                                // This matches code in MultiPlayerChunkCache
                                // that makes the geometry which continues at
                                // the edge of the world
                                if (yc * CHUNK_HEIGHT + y <=
                                    (level->getSeaLevel() - 10))
                                    tileId = Tile::stone_Id;
                                else if (yc * CHUNK_HEIGHT + y <
                                         level->getSeaLevel())
                                    tileId = Tile::calmWater_Id;
                            }

                            blocks[offs += step] = tileId;
                        }
                        _s0 += _s0a;
                        _s1 += _s1a;
                    }

                    s0 += s0a;
                    s1 += s1a;
                    s2 += s2a;
                    s3 += s3a;
                }
            }
        }
    }
    g_numPrepareHeightCalls++;
    g_totalPrepareHeightsTime += timer.elapsed();
    g_averagePrepareHeightsTime =
        g_totalPrepareHeightsTime / g_numPrepareHeightCalls;
}

void RandomLevelSource::buildSurfaces(int xOffs, int zOffs,
                                      std::vector<uint8_t>& blocks,
                                      std::vector<Biome*>& biomes) {
    int waterHeight = level->seaLevel;

    double s = 1 / 32.0;

    std::vector<double> depthBuffer(
        16 * 16);  // 4J - used to be declared with class level
                   // scope but moved here for thread safety

    depthBuffer = perlinNoise3->getRegion(depthBuffer, xOffs * 16, zOffs * 16,
                                          0, 16, 16, 1, s * 2, s * 2, s * 2);

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            Biome* b = biomes[z + x * 16];
            float temp = b->getTemperature();
            int runDepth = (int)(depthBuffer[x + z * 16] / 3 + 3 +
                                 random->nextDouble() * 0.25);

            int run = -1;

            uint8_t top = b->topMaterial;
            uint8_t material = b->material;

            LevelGenerationOptions* lgo = app.getLevelGenerationOptions();
            if (lgo != nullptr) {
                lgo->getBiomeOverride(b->id, material, top);
            }

            for (int y = Level::genDepthMinusOne; y >= 0; y--) {
                int offs = (z * 16 + x) * Level::genDepth + y;

                if (y <= 1 + random->nextInt(
                                 2))  // 4J - changed to make the bedrock not
                                      // have bits you can get stuck in
                                      //                if (y <= 0 +
                                      //                random->nextInt(5))
                {
                    blocks[offs] = (uint8_t)Tile::unbreakable_Id;
                } else {
                    int old = blocks[offs];

                    if (old == 0) {
                        run = -1;
                    } else if (old == Tile::stone_Id) {
                        // Amplified high alpine: keep bare stone on the very
                        // top of the mountains so the peaks read as rocky
                        // (and so the high-altitude snow cap can sit on
                        // stone rather than grass / dirt slopes).
                        uint8_t useTop = top;
                        uint8_t useMaterial = material;
                        if (amplified && y > waterHeight + 50) {
                            useTop = (uint8_t)Tile::stone_Id;
                            useMaterial = (uint8_t)Tile::stone_Id;
                        }

                        if (run == -1) {
                            if (runDepth <= 0) {
                                useTop = 0;
                                useMaterial = (uint8_t)Tile::stone_Id;
                            } else if (y >= waterHeight - 4 &&
                                       y <= waterHeight + 1) {
                                useTop = b->topMaterial;
                                useMaterial = b->material;
                                if (lgo != nullptr) {
                                    lgo->getBiomeOverride(b->id, useMaterial,
                                                          useTop);
                                }
                            }

                            if (y < waterHeight && useTop == 0) {
                                if (temp < 0.15f)
                                    useTop = (uint8_t)Tile::ice_Id;
                                else
                                    useTop = (uint8_t)Tile::calmWater_Id;
                            }

                            run = runDepth;
                            top = useTop;
                            material = useMaterial;
                            if (y >= waterHeight - 1)
                                blocks[offs] = useTop;
                            else
                                blocks[offs] = useMaterial;
                        } else if (run > 0) {
                            run--;
                            blocks[offs] = useMaterial;

                            // place a few sandstone blocks beneath sand runs
                            if (run == 0 && useMaterial == Tile::sand_Id) {
                                run = random->nextInt(4);
                                material = (uint8_t)Tile::sandStone_Id;
                            }
                        }
                    }
                }
            }
        }
    }
}

LevelChunk* RandomLevelSource::create(int x, int z) { return getChunk(x, z); }

LevelChunk* RandomLevelSource::getChunk(int xOffs, int zOffs) {
    random->setSeed(xOffs * 341873128712l + zOffs * 132897987541l);

    // 4J - now allocating this with a physical alloc & bypassing general memory
    // management so that it will get cleanly freed
    int blocksSize = Level::genDepth * 16 * 16;
    uint8_t* tileData = (uint8_t*)malloc(blocksSize);
    memset(tileData, 0, blocksSize);
    std::vector<uint8_t> blocks =
        std::vector<uint8_t>(tileData, tileData + blocksSize);
    //    std::vector<uint8_t> blocks = std::vector<uint8_t>(16 * level->depth *
    //    16);

    // LevelChunk *levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
    // // 4J - moved to below

    prepareHeights(xOffs, zOffs, blocks);

    // 4J - Some changes made here to how biomes, temperatures and downfalls are
    // passed around for thread safety
    std::vector<Biome*> biomes;
    level->getBiomeSource()->getBiomeBlock(biomes, xOffs * 16, zOffs * 16, 16,
                                           16, true);

    buildSurfaces(xOffs, zOffs, blocks, biomes);
    addAmplifiedFloatingIslands(xOffs, zOffs, blocks, biomes);

    caveFeature->apply(this, level, xOffs, zOffs, blocks);
    // 4J Stu Design Change - 1.8 gen goes stronghold, mineshaft, village,
    // canyon this changed in 1.2 to canyon, mineshaft, village, stronghold This
    // change makes sense as it stops canyons running through other structures
    canyonFeature->apply(this, level, xOffs, zOffs, blocks);
    if (generateStructures) {
        mineShaftFeature->apply(this, level, xOffs, zOffs, blocks);
        villageFeature->apply(this, level, xOffs, zOffs, blocks);
        strongholdFeature->apply(this, level, xOffs, zOffs, blocks);
        scatteredFeature->apply(this, level, xOffs, zOffs, blocks);
    }
    //        canyonFeature.apply(this, level, xOffs, zOffs, blocks);
    // townFeature.apply(this, level, xOffs, zOffs, blocks);
    // addCaves(xOffs, zOffs, blocks);
    // addTowns(xOffs, zOffs, blocks);

    //    levelChunk->recalcHeightmap();		// 4J - removed & moved
    //    into its own method

    // 4J - this now creates compressed block data from the blocks array passed
    // in, so moved it until after the blocks are actually finalised. We also
    // now need to free the passed in blocks as the LevelChunk doesn't use the
    // passed in allocation anymore.
    LevelChunk* levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
    free(tileData);

    return levelChunk;
}

void RandomLevelSource::addAmplifiedFloatingIslands(
    int xOffs, int zOffs, std::vector<uint8_t>& blocks,
    std::vector<Biome*>& biomes) {
    // Three styles of floating land. Mostly rare so the sky stays open
    // and traversable, but each occurrence is dramatic.
    //   0 - small "shard": narrow rock chunk, sometimes with a stalactite.
    //   1 - medium plateau: classic flat-bottomed sky island.
    //   2 - massive sky bastion: huge plateau with a deep stalactite trail.
    if (!amplified) return;

    int roll = random->nextInt(280);
    int style;
    if (roll < 1) {
        style = 2;  // ~0.36% chance per chunk - rare colossal island
    } else if (roll < 6) {
        style = 1;  // ~1.8% chance per chunk
    } else if (roll < 20) {
        style = 0;  // ~5% chance per chunk
    } else {
        return;
    }

    int minY = level->seaLevel + 48;
    int maxY = Level::genDepth - 12;
    if (maxY <= minY) return;

    int cx = 5 + random->nextInt(6);
    int cz = 5 + random->nextInt(6);
    Biome* biome = biomes[cz + cx * 16];

    // Avoid placing islands directly above oceans / rivers / swamps so the
    // huge structures read as continental sky bastions rather than weird
    // water artefacts.
    if (biome == Biome::ocean || biome == Biome::frozenOcean ||
        biome == Biome::river || biome == Biome::frozenRiver ||
        biome == Biome::swampland) {
        return;
    }

    int cy = random->nextInt(minY, maxY);
    int rx, rz, ry;
    int stalactiteDepth = 0;
    double edgeJitter = 0.35;
    switch (style) {
        case 0:
            rx = random->nextInt(4, 7);
            rz = random->nextInt(4, 7);
            ry = random->nextInt(3, 5);
            stalactiteDepth = random->nextInt(0, 6);
            edgeJitter = 0.45;
            break;
        case 1:
            rx = random->nextInt(8, 13);
            rz = random->nextInt(8, 13);
            ry = random->nextInt(3, 5);
            stalactiteDepth = random->nextInt(4, 12);
            edgeJitter = 0.30;
            break;
        case 2:
        default:
            rx = random->nextInt(13, 18);
            rz = random->nextInt(13, 18);
            ry = random->nextInt(5, 8);
            stalactiteDepth = random->nextInt(12, 24);
            edgeJitter = 0.25;
            break;
    }

    int columnTop = 0;
    for (int y = Level::genDepthMinusOne; y >= 0; y--) {
        if (blocks[(cx * 16 + cz) * Level::genDepth + y] != 0) {
            columnTop = y;
            break;
        }
    }
    // Keep enough vertical clearance below the island so it really hangs in
    // the air rather than touching the mountain top.
    if (columnTop > cy - ry - 8) return;

    // ---- Place the main ellipsoidal mass of rock ----
    for (int dx = -rx; dx <= rx; dx++) {
        int xx = cx + dx;
        if (xx < 0 || xx >= 16) continue;
        for (int dz = -rz; dz <= rz; dz++) {
            int zz = cz + dz;
            if (zz < 0 || zz >= 16) continue;
            double horiz = (double)(dx * dx) / (double)(rx * rx) +
                           (double)(dz * dz) / (double)(rz * rz);
            for (int dy = -ry; dy <= ry; dy++) {
                int yy = cy + dy;
                if (yy <= 1 || yy >= Level::genDepth) continue;

                double ny = (double)dy / (double)(ry + 1);
                double edgeNoise = random->nextDouble() * edgeJitter;
                if (horiz + ny * ny > 1.0 + edgeNoise) continue;

                int offs = (xx * 16 + zz) * Level::genDepth + yy;
                if (blocks[offs] == 0) {
                    blocks[offs] = (uint8_t)Tile::stone_Id;
                }
            }

            // Trailing stalactite under the bottom of the island so it has
            // a dramatic silhouette from below.
            if (stalactiteDepth > 0) {
                double t = horiz;
                if (t < 0.55) {
                    int reach =
                        (int)((1.0 - t / 0.55) * stalactiteDepth +
                              random->nextDouble() * 2.0);
                    int baseY = cy - ry;
                    for (int s = 1; s < reach; s++) {
                        int yy = baseY - s;
                        if (yy <= 2) break;
                        if (s > stalactiteDepth - 2 &&
                            random->nextInt(3) == 0) {
                            break;
                        }
                        int offs = (xx * 16 + zz) * Level::genDepth + yy;
                        if (blocks[offs] == 0) {
                            blocks[offs] = (uint8_t)Tile::stone_Id;
                        }
                    }
                }
            }
        }
    }

    // ---- Surface the top of the island with biome materials ----
    uint8_t top = biome->topMaterial;
    uint8_t material = biome->material;
    for (int dx = -rx; dx <= rx; dx++) {
        int xx = cx + dx;
        if (xx < 0 || xx >= 16) continue;
        for (int dz = -rz; dz <= rz; dz++) {
            int zz = cz + dz;
            if (zz < 0 || zz >= 16) continue;
            int materialRun = 0;
            for (int yy = cy + ry; yy >= cy - ry; yy--) {
                if (yy <= 1 || yy >= Level::genDepth) continue;
                int offs = (xx * 16 + zz) * Level::genDepth + yy;
                if (blocks[offs] == Tile::stone_Id) {
                    if (materialRun == 0) {
                        blocks[offs] = top;
                    } else if (materialRun < 4) {
                        blocks[offs] = material;
                    }
                    materialRun++;
                }
            }
        }
    }
}

// 4J - removed & moved into its own method from getChunk, so we can call
// recalcHeightmap after the chunk is added into the cache. Without doing this,
// then loads of the lightgaps() calls will fail to add any lights, because
// adding a light checks if the cache has this chunk in. lightgaps also does
// light 1 block into the neighbouring chunks, and maybe that is somehow enough
// to get lighting to propagate round the world, but this just doesn't seem
// right - this isn't a new fault in the 360 version, have checked that java
// does the same.
void RandomLevelSource::lightChunk(LevelChunk* lc) { lc->recalcHeightmap(); }

std::vector<double> RandomLevelSource::getHeights(std::vector<double>& buffer,
                                                  int x, int y, int z,
                                                  int xSize, int ySize,
                                                  int zSize,
                                                  std::vector<Biome*>& biomes) {
    if (buffer.empty()) {
        buffer = std::vector<double>(xSize * ySize * zSize);
    }
    if (pows.empty()) {
        pows = std::vector<float>(5 * 5);
        for (int xb = -2; xb <= 2; xb++) {
            for (int zb = -2; zb <= 2; zb++) {
                float ppp = 10.0f / Mth::sqrt(xb * xb + zb * zb + 0.2f);
                pows[xb + 2 + (zb + 2) * 5] = ppp;
            }
        }
    }

    double s = 1 * 684.412;
    double hs = 1 * 684.412;

    std::vector<double> pnr, ar, br, sr, dr, fi,
        fis;  // 4J - used to be declared with class level scope but moved here
              // for thread safety

    if (FLOATING_ISLANDS) {
        fis = floatingIslandScale->getRegion(fis, x, y, z, xSize, 1, zSize, 1.0,
                                             0, 1.0);
        fi = floatingIslandNoise->getRegion(fi, x, y, z, xSize, 1, zSize, 500.0,
                                            0, 500.0);
    }

    sr = scaleNoise->getRegion(sr, x, z, xSize, zSize, 1.121, 1.121, 0.5);
    dr = depthNoise->getRegion(dr, x, z, xSize, zSize, 200.0, 200.0, 0.5);
    pnr = perlinNoise1->getRegion(pnr, x, y, z, xSize, ySize, zSize, s / 80.0,
                                  hs / 160.0, s / 80.0);
    ar = lperlinNoise1->getRegion(ar, x, y, z, xSize, ySize, zSize, s, hs, s);
    br = lperlinNoise2->getRegion(br, x, y, z, xSize, ySize, zSize, s, hs, s);

    x = z = 0;

    int p = 0;
    int pp = 0;

    for (int xx = 0; xx < xSize; xx++) {
        for (int zz = 0; zz < zSize; zz++) {
            float sss = 0;
            float ddd = 0;
            float pow = 0;

            int rr = 2;

            Biome* mb = biomes[(xx + 2) + (zz + 2) * (xSize + 5)];
            for (int xb = -rr; xb <= rr; xb++) {
                for (int zb = -rr; zb <= rr; zb++) {
                    Biome* b =
                        biomes[(xx + xb + 2) + (zz + zb + 2) * (xSize + 5)];
                    float biomeDepth = b->depth;
                    float biomeScale = b->scale;
                    if (amplified && biomeDepth > 0.0f) {
                        biomeDepth = 1.0f + biomeDepth * 2.0f;
                        biomeScale = 1.0f + biomeScale * 4.0f;
                    }

                    float ppp =
                        pows[xb + 2 + (zb + 2) * 5] / (biomeDepth + 2);
                    if (b->depth > mb->depth) {
                        ppp /= 2;
                    }
                    sss += biomeScale * ppp;
                    ddd += biomeDepth * ppp;
                    pow += ppp;
                }
            }
            sss /= pow;
            ddd /= pow;

            sss = sss * 0.9f + 0.1f;
            ddd = (ddd * 4 - 1) / 8.0f;

            bool amplifiedLand = amplified && mb->depth > 0.0f;
            double amplifiedRidge = 0.0;
            double amplifiedValley = 0.0;
            double amplifiedRegion = 0.0;
            if (amplifiedLand) {
                double terrainNoise = sr[pp];
                if (terrainNoise < -1.0) terrainNoise = -1.0;
                if (terrainNoise > 1.0) terrainNoise = 1.0;

                // Sharp ridge: only the high crests rise dramatically, with
                // a steeper exponent so the peaks read as cliff-like rather
                // than rounded hills.
                amplifiedRidge = terrainNoise - 0.02;
                if (amplifiedRidge < 0.0) amplifiedRidge = 0.0;
                amplifiedRidge =
                    amplifiedRidge * amplifiedRidge * amplifiedRidge * 2.4;
                if (amplifiedRidge > 1.6) amplifiedRidge = 1.6;

                // Deep valley: cuts canyons where noise is strongly negative.
                amplifiedValley = -terrainNoise - 0.30;
                if (amplifiedValley < 0.0) amplifiedValley = 0.0;
                amplifiedValley = amplifiedValley * amplifiedValley * 1.6;
                if (amplifiedValley > 1.2) amplifiedValley = 1.2;

                // Macro region modulation: use the slow depth noise to mark
                // "super mountain" zones where the lift is amplified even
                // further. Range roughly [-1, +1], remapped to [0, 1.4]
                // so flat regions still exist between mountain bands.
                double region = dr[pp] / 7000.0;
                if (region < -1.0) region = -1.0;
                if (region > 1.0) region = 1.0;
                amplifiedRegion = (region + 0.2) * 0.7;
                if (amplifiedRegion < 0.0) amplifiedRegion = 0.0;
                if (amplifiedRegion > 1.4) amplifiedRegion = 1.4;
            }

            double rdepth = (dr[pp] / 8000.0);
            if (rdepth < 0) rdepth = -rdepth * 0.3;
            rdepth = rdepth * 3.0 - 2.0;

            if (rdepth < 0) {
                rdepth = rdepth / 2;
                if (rdepth < -1) rdepth = -1;
                rdepth = rdepth / 1.4;
                rdepth /= 2;
            } else {
                if (rdepth > 1) rdepth = 1;
                rdepth = rdepth / 8;
            }

            pp++;

            for (int yy = 0; yy < ySize; yy++) {
                double depth = ddd;
                double scale = sss;

                depth += rdepth * 0.2;
                depth = depth * ySize / 16.0;

                double yCenter = ySize / 2.0 + depth * 4;

                double val = 0;

                double yOffs =
                    (yy - (yCenter)) * 12 * 128 / Level::genDepth / scale;

                if (yOffs < 0) yOffs *= 4;

                double bb = ar[p] / 512;
                double cc = br[p] / 512;

                double v = (pnr[p] / 10 + 1) / 2;
                if (v < 0)
                    val = bb;
                else if (v > 1)
                    val = cc;
                else
                    val = bb + (cc - bb) * v;
                val -= yOffs;

                if (amplifiedLand) {
                    // Vertical profile that ramps up only above the mid
                    // height and fades again near the very top so we get
                    // sharp mountain peaks instead of a carpet of floating
                    // pockets across the whole vertical range.
                    double yFrac = (double)yy / (double)(ySize - 1);
                    double profile = 0.0;
                    if (yFrac > 0.38) {
                        double t = (yFrac - 0.38) / 0.50;
                        if (t > 1.0) t = 1.0;
                        profile = t * t * (3.0 - 2.0 * t);
                    }
                    if (yFrac > 0.94) {
                        double fade = (1.0 - yFrac) / 0.06;
                        if (fade < 0.0) fade = 0.0;
                        profile *= fade;
                    }

                    // Lift scales further inside macro mountain regions, so
                    // some bands of the world become truly colossal while
                    // others stay tame enough to keep the world traversable.
                    double regionLift = 1.0 + amplifiedRegion * 1.8;
                    double cliffLift =
                        (amplifiedRidge * 9.5 * regionLift -
                         amplifiedValley * 3.5) *
                        profile;
                    val += cliffLift;

                    if (amplifiedValley > 0.0) {
                        // Carve dramatic canyons across the lower half.
                        double valleyProfile = 0.0;
                        if (yFrac < 0.60) {
                            double t = (0.60 - yFrac) / 0.60;
                            if (t > 1.0) t = 1.0;
                            valleyProfile = t * t;
                        }
                        val -= amplifiedValley * 7.5 * valleyProfile;
                    }
                }

                // Push the slide cutoff higher inside amplified mountain
                // ridges so peaks can actually punch close to the world
                // ceiling instead of being clipped a few cells down.
                int slideStart = ySize - 4;
                if (amplifiedLand) {
                    slideStart = amplifiedRidge > 0.4 ? ySize - 2 : ySize - 3;
                }
                if (yy > slideStart) {
                    double slide = (yy - slideStart) /
                                   (double)(ySize - 1 - slideStart);
                    val = val * (1 - slide) + -10 * slide;
                }

                buffer[p] = val;
                p++;
            }
        }
    }

    return buffer;
}

bool RandomLevelSource::hasChunk(int x, int y) { return true; }

void RandomLevelSource::calcWaterDepths(ChunkSource* parent, int xt, int zt) {
    int xo = xt * 16;
    int zo = zt * 16;
    for (int x = 0; x < 16; x++) {
        int y = level->getSeaLevel();
        for (int z = 0; z < 16; z++) {
            int xp = xo + x + 7;
            int zp = zo + z + 7;
            int h = level->getHeightmap(xp, zp);
            if (h <= 0) {
                if (level->getHeightmap(xp - 1, zp) > 0 ||
                    level->getHeightmap(xp + 1, zp) > 0 ||
                    level->getHeightmap(xp, zp - 1) > 0 ||
                    level->getHeightmap(xp, zp + 1) > 0) {
                    bool hadWater = false;
                    if (hadWater ||
                        (level->getTile(xp - 1, y, zp) == Tile::calmWater_Id &&
                         level->getData(xp - 1, y, zp) < 7))
                        hadWater = true;
                    if (hadWater ||
                        (level->getTile(xp + 1, y, zp) == Tile::calmWater_Id &&
                         level->getData(xp + 1, y, zp) < 7))
                        hadWater = true;
                    if (hadWater ||
                        (level->getTile(xp, y, zp - 1) == Tile::calmWater_Id &&
                         level->getData(xp, y, zp - 1) < 7))
                        hadWater = true;
                    if (hadWater ||
                        (level->getTile(xp, y, zp + 1) == Tile::calmWater_Id &&
                         level->getData(xp, y, zp + 1) < 7))
                        hadWater = true;
                    if (hadWater) {
                        for (int x2 = -5; x2 <= 5; x2++) {
                            for (int z2 = -5; z2 <= 5; z2++) {
                                int d =
                                    (x2 > 0 ? x2 : -x2) + (z2 > 0 ? z2 : -z2);

                                if (d <= 5) {
                                    d = 6 - d;
                                    if (level->getTile(xp + x2, y, zp + z2) ==
                                        Tile::calmWater_Id) {
                                        int od =
                                            level->getData(xp + x2, y, zp + z2);
                                        if (od < 7 && od < d) {
                                            level->setData(xp + x2, y, zp + z2,
                                                           d, Tile::UPDATE_ALL);
                                        }
                                    }
                                }
                            }
                        }
                        if (hadWater) {
                            level->setTileAndData(xp, y, zp, Tile::calmWater_Id,
                                                  7, Tile::UPDATE_CLIENTS);
                            for (int y2 = 0; y2 < y; y2++) {
                                level->setTileAndData(xp, y2, zp,
                                                      Tile::calmWater_Id, 8,
                                                      Tile::UPDATE_CLIENTS);
                            }
                        }
                    }
                }
            }
        }
    }
}

// 4J - changed this to used pprandom rather than random, so that we can run it
// concurrently with getChunk
void RandomLevelSource::postProcess(ChunkSource* parent, int xt, int zt) {
    HeavyTile::instaFall = true;
    int xo = xt * 16;
    int zo = zt * 16;

    Biome* biome = level->getBiome(xo + 16, zo + 16);

    if (FLOATING_ISLANDS) {
        calcWaterDepths(parent, xt, zt);
    }

    pprandom->setSeed(level->getSeed());
    int64_t xScale = pprandom->nextLong() / 2 * 2 + 1;
    int64_t zScale = pprandom->nextLong() / 2 * 2 + 1;
    // 4jcraft added casts to a higher int and unsigned
    pprandom->setSeed((((uint64_t)xt * (uint64_t)xScale) +
                       ((uint64_t)zt * (uint64_t)zScale)) ^
                      level->getSeed());

    bool hasVillage = false;

    if (generateStructures) {
        mineShaftFeature->postProcess(level, pprandom, xt, zt);
        hasVillage = villageFeature->postProcess(level, pprandom, xt, zt);
        strongholdFeature->postProcess(level, pprandom, xt, zt);
        scatteredFeature->postProcess(level, random, xt, zt);
    }

    if (biome != Biome::desert && biome != Biome::desertHills) {
        if (!hasVillage && pprandom->nextInt(4) == 0) {
            int x = xo + pprandom->nextInt(16) + 8;
            int y = pprandom->nextInt(Level::genDepth);
            int z = zo + pprandom->nextInt(16) + 8;

            LakeFeature calmWater(Tile::calmWater_Id);
            calmWater.place(level, pprandom, x, y, z);
        }
    }

    if (!hasVillage && pprandom->nextInt(8) == 0) {
        int x = xo + pprandom->nextInt(16) + 8;
        int y = pprandom->nextInt(pprandom->nextInt(Level::genDepth - 8) + 8);
        int z = zo + pprandom->nextInt(16) + 8;
        if (y < level->seaLevel || pprandom->nextInt(10) == 0) {
            LakeFeature calmLava(Tile::calmLava_Id);
            calmLava.place(level, pprandom, x, y, z);
        }
    }

    for (int i = 0; i < 8; i++) {
        int x = xo + pprandom->nextInt(16) + 8;
        int y = pprandom->nextInt(Level::genDepth);
        int z = zo + pprandom->nextInt(16) + 8;
        MonsterRoomFeature mrf;
        mrf.place(level, pprandom, x, y, z);
    }

    biome->decorate(level, pprandom, xo, zo);
    addAmplifiedMountainResources(xt, zt);
    addAmplifiedHighWaterfalls(xt, zt);
    addAmplifiedCliffCaves(xt, zt);

    app.processSchematics(parent->getChunk(xt, zt));

    MobSpawner::postProcessSpawnMobs(level, biome, xo + 8, zo + 8, 16, 16,
                                     pprandom);

    // 4J - brought forward from 1.2.3 to get snow back in taiga biomes
    xo += 8;
    zo += 8;
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int y = level->getTopRainBlock(xo + x, zo + z);

            if (level->shouldFreezeIgnoreNeighbors(x + xo, y - 1, z + zo)) {
                level->setTileAndData(x + xo, y - 1, z + zo, Tile::ice_Id, 0,
                                      Tile::UPDATE_CLIENTS);
            }
            if (level->shouldSnow(x + xo, y, z + zo)) {
                level->setTileAndData(x + xo, y, z + zo, Tile::topSnow_Id, 0,
                                      Tile::UPDATE_CLIENTS);
            }
            // Amplified: only cover the very high stone peaks with snow.
            // No grass/dirt slopes - we want the snow to read as alpine cap
            // sitting on rock, and only on the truly extreme summits.
            if (amplified && y > level->seaLevel + 58 &&
                y < Level::genDepthMinusOne &&
                level->getTile(x + xo, y, z + zo) == 0 &&
                level->getTile(x + xo, y - 1, z + zo) == Tile::stone_Id) {
                level->setTileAndData(x + xo, y, z + zo, Tile::topSnow_Id, 0,
                                      Tile::UPDATE_CLIENTS);
            }
        }
    }

    HeavyTile::instaFall = false;
}

void RandomLevelSource::addAmplifiedMountainResources(int xt, int zt) {
    if (!amplified) return;

    int xo = xt * 16;
    int zo = zt * 16;
    int clusters = 1 + pprandom->nextInt(3);
    for (int i = 0; i < clusters; i++) {
        int x = xo + pprandom->nextInt(16) + 8;
        int z = zo + pprandom->nextInt(16) + 8;
        int top = level->getHeightmap(x, z);
        int minY = level->seaLevel + 8;
        int maxY = top - 4;
        if (top < level->seaLevel + 24 || maxY <= minY) continue;

        int roll = pprandom->nextInt(100);
        int tile = Tile::coalOre_Id;
        int count = pprandom->nextInt(5, 9);
        if (roll >= 62 && roll < 90) {
            tile = Tile::ironOre_Id;
            count = pprandom->nextInt(4, 7);
        } else if (roll >= 90 && roll < 97) {
            tile = Tile::goldOre_Id;
            count = pprandom->nextInt(3, 5);
        } else if (roll >= 97) {
            tile = Tile::emeraldOre_Id;
            count = pprandom->nextInt(1, 2);
        }

        int y = pprandom->nextInt(minY, maxY);
        for (int n = 0; n < count; n++) {
            int xx = x + pprandom->nextInt(-2, 2);
            int yy = y + pprandom->nextInt(-1, 1);
            int zz = z + pprandom->nextInt(-2, 2);
            if (yy <= 1 || yy >= Level::genDepthMinusOne) continue;
            if (level->getTile(xx, yy, zz) == Tile::stone_Id) {
                level->setTileAndData(xx, yy, zz, tile, 0,
                                      Tile::UPDATE_INVISIBLE_NO_LIGHT);
            }
        }
    }
}

void RandomLevelSource::addAmplifiedHighWaterfalls(int xt, int zt) {
    if (!amplified || pprandom->nextInt(2) != 0) return;

    int xo = xt * 16;
    int zo = zt * 16;
    static const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    int placed = 0;
    int targetWaterfalls = 1 + pprandom->nextInt(2);
    for (int tries = 0; tries < 12 && placed < targetWaterfalls; tries++) {
        int x = xo + pprandom->nextInt(16) + 8;
        int z = zo + pprandom->nextInt(16) + 8;
        int y = level->getHeightmap(x, z);
        if (y < level->seaLevel + 28 || y >= Level::genDepthMinusOne - 2) {
            continue;
        }

        for (int i = 0; i < 4; i++) {
            int dir = (i + pprandom->nextInt(4)) & 3;
            int wx = x + dirs[dir][0];
            int wz = z + dirs[dir][1];
            int wy = y - 1;

            if (level->getTile(wx, wy, wz) != 0 ||
                level->getTile(wx, wy - 1, wz) != 0) {
                continue;
            }

            int drop = 0;
            for (int yy = wy - 1; yy > level->seaLevel && drop < 40; yy--) {
                if (level->getTile(wx, yy, wz) != 0) break;
                drop++;
            }
            if (drop < 8) continue;

            level->setTileAndData(wx, wy, wz, Tile::calmWater_Id, 0,
                                  Tile::UPDATE_CLIENTS);
            placed++;
            break;
        }
    }
}

void RandomLevelSource::addAmplifiedCliffCaves(int xt, int zt) {
    if (!amplified || pprandom->nextInt(3) != 0) return;

    int xo = xt * 16;
    int zo = zt * 16;
    static const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    int attempts = 1 + pprandom->nextInt(2);
    int placed = 0;
    for (int attempt = 0; attempt < attempts * 4 && placed < attempts;
         attempt++) {
        int x = xo + pprandom->nextInt(16) + 8;
        int z = zo + pprandom->nextInt(16) + 8;
        int cliffTop = level->getHeightmap(x, z);

        if (cliffTop < level->seaLevel + 18 ||
            cliffTop >= Level::genDepthMinusOne - 4) {
            continue;
        }

        // Look for a steep face in one of the cardinal directions.
        int faceDir = -1;
        int faceDrop = 0;
        for (int i = 0; i < 4; i++) {
            int dir = (i + pprandom->nextInt(4)) & 3;
            int probeX = x + dirs[dir][0] * 4;
            int probeZ = z + dirs[dir][1] * 4;
            int neighborTop = level->getHeightmap(probeX, probeZ);
            int drop = cliffTop - neighborTop;
            if (drop > 14 && drop > faceDrop) {
                faceDrop = drop;
                faceDir = dir;
            }
        }
        if (faceDir < 0) continue;

        int outX = dirs[faceDir][0];
        int outZ = dirs[faceDir][1];
        int inX = -outX;
        int inZ = -outZ;

        // Entrance roughly half way up the cliff so the mouth is clearly
        // visible from the valley but still has plenty of stone above it.
        int entranceY = cliffTop - 4 - pprandom->nextInt(faceDrop - 8);
        if (entranceY < level->seaLevel + 4) continue;

        // Walk outwards until we hit air - that is the cliff face.
        int faceX = x;
        int faceZ = z;
        bool foundFace = false;
        for (int step = 0; step < 12; step++) {
            int probeX = faceX + outX;
            int probeZ = faceZ + outZ;
            if (level->getTile(probeX, entranceY, probeZ) == 0) {
                foundFace = true;
                break;
            }
            faceX = probeX;
            faceZ = probeZ;
        }
        if (!foundFace) continue;

        int length = 7 + pprandom->nextInt(9);
        int baseRadius = 2 + pprandom->nextInt(2);

        double cx = faceX + 0.5;
        double cy = entranceY + 0.5;
        double cz = faceZ + 0.5;

        for (int l = 0; l < length; l++) {
            double t = (double)l / (double)length;
            // Gentle wander into the mountain so it doesn't look too
            // surgical.
            double wanderY = sin(l * 0.35) * 1.5 +
                             (pprandom->nextDouble() - 0.5) * 0.8;
            double wanderH = sin(l * 0.5) * 0.8;
            cx += inX + (inZ != 0 ? wanderH : 0.0);
            cz += inZ + (inX != 0 ? wanderH : 0.0);
            cy += wanderY * 0.4;

            // The entrance flares out wider than the tail of the tunnel.
            double tailFactor = 1.0 - t * 0.6;
            int rr = (int)(baseRadius * tailFactor + 0.5);
            if (rr < 1) rr = 1;

            for (int dx = -rr - 1; dx <= rr + 1; dx++) {
                for (int dy = -rr - 1; dy <= rr + 1; dy++) {
                    for (int dz = -rr - 1; dz <= rr + 1; dz++) {
                        double dd =
                            (dx * dx) +
                            (dy * dy) * (1.4 + tailFactor * 0.6) +
                            (dz * dz);
                        if (dd > (double)(rr * rr)) continue;
                        int wx = (int)cx + dx;
                        int wy = (int)cy + dy;
                        int wz = (int)cz + dz;
                        if (wy <= 2 || wy >= Level::genDepthMinusOne) continue;

                        int tile = level->getTile(wx, wy, wz);
                        if (tile == Tile::stone_Id || tile == Tile::dirt_Id ||
                            tile == Tile::grass_Id || tile == Tile::sand_Id ||
                            tile == Tile::sandStone_Id ||
                            tile == Tile::gravel_Id) {
                            level->setTileAndData(
                                wx, wy, wz, 0, 0,
                                Tile::UPDATE_INVISIBLE_NO_LIGHT);
                        }
                    }
                }
            }
        }
        placed++;
    }
}

bool RandomLevelSource::save(bool force, ProgressListener* progressListener) {
    return true;
}

bool RandomLevelSource::tick() { return false; }

bool RandomLevelSource::shouldSave() { return true; }

std::wstring RandomLevelSource::gatherStats() { return L"RandomLevelSource"; }

std::vector<Biome::MobSpawnerData*>* RandomLevelSource::getMobsAt(
    MobCategory* mobCategory, int x, int y, int z) {
    Biome* biome = level->getBiome(x, z);
    if (biome == nullptr) {
        return nullptr;
    }
    if (mobCategory == MobCategory::monster &&
        scatteredFeature->isSwamphut(x, y, z)) {
        return scatteredFeature->getSwamphutEnemies();
    }
    return biome->getMobs(mobCategory);
}

TilePos* RandomLevelSource::findNearestMapFeature(
    Level* level, const std::wstring& featureName, int x, int y, int z) {
    if (LargeFeature::STRONGHOLD == featureName &&
        strongholdFeature != nullptr) {
        return strongholdFeature->getNearestGeneratedFeature(level, x, y, z);
    }
    return nullptr;
}

void RandomLevelSource::recreateLogicStructuresForChunk(int chunkX,
                                                        int chunkZ) {
    if (generateStructures) {
        std::vector<uint8_t> emptyBlocks;
        mineShaftFeature->apply(this, level, chunkX, chunkZ, emptyBlocks);
        villageFeature->apply(this, level, chunkX, chunkZ, emptyBlocks);
        strongholdFeature->apply(this, level, chunkX, chunkZ, emptyBlocks);
        scatteredFeature->apply(this, level, chunkX, chunkZ, emptyBlocks);
    }
}
