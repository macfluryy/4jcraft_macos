#include "../../Platform/stdafx.h"
#include "../../Util/Arrays.h"
#include "FixedBiomeSource.h"

/**
 * @brief Constructs a FixedBiomeSource with a single biome, temperature, and
 * downfall.
 *
 * @param fixed Biome to use for the entire source
 * @param temperature Temperature value for all blocks
 * @param downfall Downfall value for all blocks
 */
FixedBiomeSource::FixedBiomeSource(Biome* fixed, float temperature,
                                   float downfall) {
    this->biome = fixed;
    this->temperature = temperature;
    this->downfall = downfall;
}

/**
 * @brief Returns the biome at a given ChunkPos.
 *
 * @param cp Target chunk position
 * @return Biome* Always returns the fixed biome
 */
Biome* FixedBiomeSource::getBiome(ChunkPos* cp) { return biome; }

/**
 * @brief Returns the biome at specific coordinates.
 *
 * @param x X coordinate
 * @param z Z coordinate
 * @return Biome* Always returns the fixed biome
 */
Biome* FixedBiomeSource::getBiome(int x, int z) { return biome; }

/**
 * @brief Returns the temperature at specific coordinates.
 *
 * @param x X coordinate
 * @param z Z coordinate
 * @return float The fixed temperature
 */
float FixedBiomeSource::getTemperature(int x, int z) { return temperature; }

/**
 * @brief Fills a float array with temperature values for a rectangular region.
 *
 * If the array is null or too small, it will be reallocated.
 *
 * @param temperatures Array to fill with temperature values
 * @param x Starting X coordinate
 * @param z Starting Z coordinate
 * @param w Width of the region
 * @param h Height of the region
 */
void FixedBiomeSource::getTemperatureBlock(floatArray& temperatures, int x,
                                           int z, int w, int h) const {
    if (!temperatures.data || temperatures.length < w * h) {
        if (temperatures.data) delete[] temperatures.data;
        temperatures = floatArray(w * h);
    }
    Arrays::fill(temperatures, 0, w * h, temperature);
}
/**
 * @brief Returns a floatArray filled with temperatures.
 *
 * @param x Starting X coordinate
 * @param z Starting Z coordinate
 * @param w Width of the region
 * @param h Height of the region
 * @note 4J - Caller is responsible for deleting returned array.
 * @note 4J - Temperatures array is for output only
 * @return floatArray Filled with temperature values
 */
floatArray FixedBiomeSource::getTemperatureBlock(int x, int z, int w,
                                                 int h) const {
    floatArray temps(w * h);
    getTemperatureBlock(temps, x, z, w, h);
    return temps;
}

/**
 * @brief Fills a double array with temperature values.
 *
 * @param temperatures Array to fill (memory allocated inside)
 * @param x Starting X coordinate
 * @param z Starting Z coordinate
 * @param w Width of the region
 * @param h Height of the region
 */
void FixedBiomeSource::getTemperatureBlock(doubleArray& temperatures, int x,
                                           int z, int w, int h) const {
    temperatures = doubleArray(w * h);
    Arrays::fill(temperatures, 0, w * h, static_cast<double>(temperature));
}

/**
 * @brief Fills a float array with downfall values for a rectangular region.
 */
void FixedBiomeSource::getDownfallBlock(floatArray& downfalls, int x, int z,
                                        int w, int h) const {
    if (downfalls.data == NULL || downfalls.length < w * h) {
        if (downfalls.data != NULL) delete[] downfalls.data;
        downfalls = floatArray(w * h);
    }
    Arrays::fill(downfalls, 0, w * h, downfall);
}
/**
 * @brief Returns a floatArray filled with downfall values.
 */
floatArray FixedBiomeSource::getDownfallBlock(int x, int z, int w,
                                              int h) const {
    floatArray downfalls(w * h);
    getDownfallBlock(downfalls, x, z, w, h);
    return downfalls;
}

/**
 * @brief Returns the fixed downfall value at given coordinates.
 */
float FixedBiomeSource::getDownfall(int x, int z) const { return downfall; }

/**
 * @brief Fills a double array with downfall values.
 */
void FixedBiomeSource::getDownfallBlock(doubleArray& downfalls, int x, int z,
                                        int w, int h) {
    if (!downfalls.data || downfalls.length < w * h) {
        if (downfalls.data) delete[] downfalls.data;
        downfalls = doubleArray(w * h);
    }
    Arrays::fill(downfalls, 0, w * h, static_cast<double>(downfall));
}

/**
 * @brief Fills a BiomeArray with the fixed biome for a region.
 */
void FixedBiomeSource::getBiomeBlock(BiomeArray& biomes, int x, int z, int w,
                                     int h, bool useCache) const {
    MemSect(36);
    biomes = BiomeArray(w * h);
    MemSect(0);
    Arrays::fill(biomes, 0, w * h, biome);
}

/**
 * @brief Fills a byteArray with the biome index for a region.
 */
void FixedBiomeSource::getBiomeIndexBlock(byteArray& biomeIndices, int x, int z,
                                          int w, int h, bool useCache) const {
    MemSect(36);
    biomeIndices = byteArray(w * h);
    MemSect(0);
    int biomeIndex = biome->id;
    Arrays::fill(biomeIndices, 0, w * h, biomeIndex);
}

/**
 * @brief Fills a BiomeArray with the fixed biome for a region (raw version).
 * @note 4J-PB Added in beyond 1.8.2
 * @note 4J - Caller is responsible for deleting biomes array, plus any optional arrays output if pointers are passed in (_temperatures, _downfalls)
 */
void FixedBiomeSource::getRawBiomeBlock(BiomeArray& biomes, int x, int z, int w,
                                        int h) const {
    MemSect(36);
    biomes = BiomeArray(w * h);
    MemSect(0);
    Arrays::fill(biomes, 0, w * h, biome);
}

/**
 * @brief Returns a raw BiomeArray. 
 * @note 4J-PB Added in beyond 1.8.2
 * @note 4J - caller is responsible for deleting biomes array, plus any optionalarrays
 * arrays output if pointers are passed in (_temperatures, _downfalls)
 */
BiomeArray FixedBiomeSource::getRawBiomeBlock(int x, int z, int w,
                                              int h) const {
    BiomeArray biomes;
    getRawBiomeBlock(biomes, x, z, w, h);
    return biomes;
}

/**
 * @brief Finds a biome within a radius randomly.
 */
TilePos* FixedBiomeSource::findBiome(int x, int z, int r, Biome* toFind,
                                     Random* random) {
    if (toFind == biome) {
        return new TilePos(x - r + random->nextInt(r * 2 + 1), 0,
                           z - r + random->nextInt(r * 2 + 1));
    }
    return nullptr;
}

/**
 * @brief Finds a biome from a list of allowed biomes randomly.
 */
TilePos* FixedBiomeSource::findBiome(int x, int z, int r,
                                     const std::vector<Biome*>& allowed,
                                     Random* random) {
    if (find(allowed.begin(), allowed.end(), biome) != allowed.end()) {
        return new TilePos(x - r + random->nextInt(r * 2 + 1), 0,
                           z - r + random->nextInt(r * 2 + 1));
    }
    return nullptr;
}

/**
 * @brief Checks if the allowed biome matches the fixed biome.
 */
bool FixedBiomeSource::containsOnly(int x, int z, int r, Biome* allowed) {
    return allowed == biome;
}

/**
 * @brief Checks if the fixed biome is in the allowed list.
 */
bool FixedBiomeSource::containsOnly(int x, int z, int r,
                                    const std::vector<Biome*>& allowed) {
    return find(allowed.begin(), allowed.end(), biome) != allowed.end();
}
