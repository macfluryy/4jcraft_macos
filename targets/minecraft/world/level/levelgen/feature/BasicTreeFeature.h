#pragma once
#include <stdint.h>

#include "Feature.h"

class Level;
class Random;

class BasicTree : public Feature {
private:
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    static uint8_t axisConversionArray[];

    
    Random* rnd;

    
    Level* thisLevel;

    
    int origin[3];
    
    int height;
    
    int trunkHeight;
    double trunkHeightScale;
    double branchDensity;
    double branchSlope;
    double widthScale;
    double foliageDensity;
    int trunkWidth;
    int heightVariance;
    int foliageHeight;
    
    
    int** foliageCoords;
    int foliageCoordsLength;
    void prepare();
    void crossection(int x, int y, int z, float radius, uint8_t direction,
                     int material);
    float treeShape(int y);
    float foliageShape(int y);
    void foliageCluster(int x, int y, int z);
    void limb(int* start, int* end, int material);
    void makeFoliage();
    bool trimBranches(int localY);
    void makeTrunk();
    void makeBranches();
    int checkLine(int* start, int* end);
    bool checkLocation();

public:
    BasicTree(bool doUpdate);
    virtual ~BasicTree();

    virtual void init(double heightInit, double widthInit,
                      double foliageDensityInit);
    virtual bool place(Level* level, Random* random, int x, int y, int z);
};
