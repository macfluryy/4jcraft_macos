#include "BinaryHeap.h"

#include <algorithm>
#include <limits>

#include "Node.h"


void BinaryHeap::_init() {
    heap = std::vector<Node*>(1024);
    sizeVar = 0;
}

BinaryHeap::BinaryHeap() { _init(); }

BinaryHeap::~BinaryHeap() {}

Node* BinaryHeap::insert(Node* node) {
    


    
    if (sizeVar == heap.size()) {
        std::vector<Node*> newHeap = std::vector<Node*>(sizeVar << 1);

        std::copy(heap.begin(), heap.begin() + sizeVar, newHeap.begin());

        heap = newHeap;
    }

    
    heap[sizeVar] = node;
    node->heapIdx = sizeVar;
    upHeap(sizeVar++);

    return node;
}

void BinaryHeap::clear() { sizeVar = 0; }

Node* BinaryHeap::peek() { return heap[0]; }

Node* BinaryHeap::pop() {
    Node* popped = heap[0];
    heap[0] = heap[--sizeVar];
    heap[sizeVar] = nullptr;
    if (sizeVar > 0) downHeap(0);
    popped->heapIdx = -1;
    return popped;
}

void BinaryHeap::remove(Node* node) {
    
    heap[node->heapIdx] = heap[--sizeVar];
    heap[sizeVar] = nullptr;
    if (sizeVar > node->heapIdx) {
        if (heap[node->heapIdx]->f < node->f) {
            upHeap(node->heapIdx);
        } else {
            downHeap(node->heapIdx);
        }
    }
    
    node->heapIdx = -1;
}

void BinaryHeap::changeCost(Node* node, float newCost) {
    float oldCost = node->f;
    node->f = newCost;
    if (newCost < oldCost) {
        upHeap(node->heapIdx);
    } else {
        downHeap(node->heapIdx);
    }
}

int BinaryHeap::size() { return sizeVar; }

void BinaryHeap::upHeap(int idx) {
    Node* node = heap[idx];
    float cost = node->f;
    while (idx > 0) {
        int parentIdx = (idx - 1) >> 1;
        Node* parent = heap[parentIdx];
        if (cost < parent->f) {
            heap[idx] = parent;
            parent->heapIdx = idx;
            idx = parentIdx;
        } else
            break;
    }
    heap[idx] = node;
    node->heapIdx = idx;
}

void BinaryHeap::downHeap(int idx) {
    Node* node = heap[idx];
    float cost = node->f;

    while (true) {
        int leftIdx = 1 + (idx << 1);
        int rightIdx = leftIdx + 1;

        if (leftIdx >= sizeVar) break;

        
        Node* leftNode = heap[leftIdx];
        float leftCost = leftNode->f;
        
        Node* rightNode;
        float rightCost;

        if (rightIdx >= sizeVar) {
            
            rightNode = nullptr;
            rightCost = std::numeric_limits<float>::infinity();
        } else {
            rightNode = heap[rightIdx];
            rightCost = rightNode->f;
        }

        
        
        if (leftCost < rightCost) {
            if (leftCost < cost) {
                heap[idx] = leftNode;
                leftNode->heapIdx = idx;
                idx = leftIdx;
            } else
                break;
        } else {
            if (rightCost < cost) {
                heap[idx] = rightNode;
                rightNode->heapIdx = idx;
                idx = rightIdx;
            } else
                break;
        }
    }

    heap[idx] = node;
    node->heapIdx = idx;
}

bool BinaryHeap::isEmpty() { return sizeVar == 0; }
