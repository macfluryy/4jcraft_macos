#pragma once

#include <cstdint>

#include "java/Class.h"








struct IntKeyHash {
    int operator()(const int& k) const {
        
        unsigned int h = k;
        h += ~(h << 9);
        h ^= (h >> 14);
        h += (h << 4);
        h ^= (h >> 10);
        return h;
    }
};

struct IntKeyEq {
    bool operator()(const int& x, const int& y) const { return x == y; }
};




struct IntKeyHash2 {
    int operator()(const int& k) const {
        unsigned int h = (unsigned int)k;
        h ^= (h >> 20) ^ (h >> 12);
        return (int)(h ^ (h >> 7) ^ (h >> 4));
    }
};




struct LongKeyHash {
    int hash(const int& k) const {
        unsigned int h = (unsigned int)k;
        h ^= (h >> 20) ^ (h >> 12);
        return (int)(h ^ (h >> 7) ^ (h >> 4));
    }

    int operator()(const int64_t& k) const {
        return hash((int)(k ^ (((uint64_t)k) >> 32)));
    }
};

struct LongKeyEq {
    bool operator()(const int64_t& x, const int64_t& y) const { return x == y; }
};

struct eINSTANCEOFKeyHash {
    int operator()(const eINSTANCEOF& k) const {
        unsigned int h = (unsigned int)k;
        h ^= (h >> 20) ^ (h >> 12);
        return (int)(h ^ (h >> 7) ^ (h >> 4));
    }
};

struct eINSTANCEOFKeyEq {
    bool operator()(const eINSTANCEOF& x, const eINSTANCEOF& y) const {
        return x == y;
    }
};
