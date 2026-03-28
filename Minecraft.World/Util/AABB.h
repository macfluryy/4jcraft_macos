#pragma once

#include "Vec3.h"
#include "Definitions.h"

class HitResult;

class AABB {
    // 4J added so we can have separate pools for different threads
    class ThreadStorage {
    public:
        static const int POOL_SIZE = 1024;
        AABB* pool;
        unsigned int poolPointer;
        ThreadStorage();
        ~ThreadStorage();
    };
    static thread_local ThreadStorage* m_tlsPool;
    static ThreadStorage* m_tlsPoolDefault;

public:
    // Each new thread that needs to use Vec3 pools will need to call one of the
    // following 2 functions, to either create its own local storage, or share
    // the default storage already allocated by the main thread
    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

    static void clearPool();
    static void resetPool();
    static AABB* newTemp(double x0, double y0, double z0, double x1, double y1,
                         double z1);

    double x0, y0, z0;
    double x1, y1, z1;

    AABB() {}
    AABB(double x0, double y0, double z0, double x1, double y1, double z1);

public:
    AABB expand(double xa, double ya, double za) const;
    AABB grow(double xa, double ya, double za) const;
    AABB minmax(const AABB& other) const;
    double clipXCollide(const AABB& c, double xa) const;
    double clipYCollide(const AABB& c, double ya) const;
    double clipZCollide(const AABB& c, double za) const;
    bool intersects(const AABB& c) const;
    AABB move(double xa, double ya, double za) const;
    bool intersects(double x02, double y02, double z02, double x12, double y12,
                    double z12) const;
    bool contains(const Vec3& p) const;
    bool containsIncludingLowerBound(const Vec3& p) const;  // 4J Added
    double getSize() const;
    AABB shrink(double xa, double ya, double za) const;
    HitResult* clip(const Vec3& a, const Vec3& b) const;
    bool containsX(const Vec3& v) const;
    bool containsY(const Vec3& v) const;
    bool containsZ(const Vec3& v) const;
    std::wstring toString() const;
};
