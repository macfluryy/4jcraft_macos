#include <string.h>

#include "Path.h"
#include "minecraft/world/entity/Entity.h"

Path::~Path() {
    for (size_t i = 0; i < nodes.size(); i++) delete nodes[i];
}

Path::Path(std::vector<Node*>& nodes) {
    index = 0;

    length = nodes.size();
    // 4J - copying these nodes over from a std::vector<Node*> (which is an
    // array of Node
    // * references) to just a straight array of Nodes, so that this Path is no
    // longer dependent of Nodes allocated elsewhere and can handle its own
    // destruction Note: cameFrom pointer will be useless now but that isn't
    // used once this is just a path
    this->nodes = std::vector<Node*>(length);

    for (int i = 0; i < length; i++) {
        this->nodes[i] = new Node();
        memcpy(this->nodes[i], nodes[i], sizeof(Node));
    }
}

void Path::next() { index++; }

bool Path::isDone() { return index >= length; }

Node* Path::last() {
    if (length > 0) {
        return nodes[length - 1];
    }
    return nullptr;
}

Node* Path::get(int i) { return nodes[i]; }

int Path::getSize() { return length; }

void Path::setSize(int length) { this->length = length; }

int Path::getIndex() { return index; }

void Path::setIndex(int index) { this->index = index; }

Vec3 Path::getPos(std::shared_ptr<Entity> e, int index) {
    double x = nodes[index]->x + (int)(e->bbWidth + 1) * 0.5;
    double y = nodes[index]->y;
    double z = nodes[index]->z + (int)(e->bbWidth + 1) * 0.5;
    return Vec3(x, y, z);
}

Vec3 Path::currentPos(std::shared_ptr<Entity> e) { return getPos(e, index); }

Vec3 Path::currentPos() {
    return Vec3(nodes[index]->x, nodes[index]->y, nodes[index]->z);
}

bool Path::sameAs(Path* path) {
    if (path == nullptr) return false;
    if (path->nodes.size() != nodes.size()) return false;
    for (int i = 0; i < nodes.size(); ++i)
        if (nodes[i]->x != path->nodes[i]->x ||
            nodes[i]->y != path->nodes[i]->y ||
            nodes[i]->z != path->nodes[i]->z)
            return false;
    return true;
}

bool Path::endsIn(Vec3* pos) {
    Node* lastNode = last();
    if (lastNode == nullptr) return false;
    return lastNode->x == (int)pos->x && lastNode->y == (int)pos->y &&
           lastNode->z == (int)pos->z;
}

bool Path::endsInXZ(Vec3* pos) {
    Node* lastNode = last();
    if (lastNode == nullptr) return false;
    return lastNode->x == (int)pos->x && lastNode->z == (int)pos->z;
}
