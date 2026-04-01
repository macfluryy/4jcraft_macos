#pragma once

#include <format>
#include <memory>
#include <vector>

#include "minecraft/world/level/pathfinder/Node.h"
#include "minecraft/world/phys/Vec3.h"

class Entity;

class Path {
    friend class PathFinder;

private:
    std::vector<Node*> nodes;
    int index;
    int length;

public:
    Path(std::vector<Node*>& nodes);
    ~Path();

    void next();
    bool isDone();
    Node* last();
    Node* get(int i);
    int getSize();
    void setSize(int length);
    int getIndex();
    void setIndex(int index);
    Vec3 getPos(std::shared_ptr<Entity> e, int index);
    std::vector<Node*> Getarray();
    Vec3 currentPos(std::shared_ptr<Entity> e);
    Vec3 currentPos();
    bool sameAs(Path* path);
    bool endsIn(Vec3* pos);
    bool endsInXZ(Vec3* pos);
};
