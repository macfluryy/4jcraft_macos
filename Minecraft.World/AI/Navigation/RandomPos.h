#pragma once

class PathfinderMob;

class RandomPos {
private:
    static Vec3* tempDir;

public:
    static Vec3* getPos(std::shared_ptr<PathfinderMob> mob, int xzDist,
                        int yDist, int quadrant = -1);  // 4J added quadrant
    static Vec3* getPosTowards(std::shared_ptr<PathfinderMob> mob, int xzDist,
                               int yDist, Vec3* towardsPos);
    static Vec3* getPosAvoid(std::shared_ptr<PathfinderMob> mob, int xzDist,
                             int yDist, Vec3* avoidPos);

private:
    static Vec3* generateRandomPos(std::shared_ptr<PathfinderMob> mob,
                                   int xzDist, int yDist, Vec3* dir,
                                   int quadrant = -1);  // 4J added quadrant
};