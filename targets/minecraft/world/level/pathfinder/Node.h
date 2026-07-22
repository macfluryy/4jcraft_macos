#pragma once

#include <string>

#include "java/Class.h"

class Node {
    
    friend class BinaryHeap;
    friend class PathFinder;
    friend class EnderDragon;

public:
    const int x, y, z;

private:
    const int hash;

protected:
    int heapIdx;
    float g, h, f;
    Node* cameFrom;

public:
    bool closed;

    void _init();
    eINSTANCEOF GetType() { return eType_NODE; }

    Node()
        : hash(0),
          x(0),
          y(0),
          z(0) {}  
                   
    Node(const int x, const int y, const int z);

    static int createHash(const int x, const int y, const int z);
    float distanceTo(Node* to);
    float distanceToSqr(Node* to);
    bool equals(Node* o);
    int hashCode();
    bool inOpenSet();
    std::wstring toString();
};
