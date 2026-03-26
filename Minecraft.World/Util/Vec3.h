#pragma once

#include <optional>
#include <string>

class AABB;

class Vec3 {
    // 4J added so we can have separate pools for different threads
    class ThreadStorage {
    public:
        static const int POOL_SIZE = 1024;
        Vec3* pool;
        unsigned int poolPointer;
        ThreadStorage();
        ~ThreadStorage();
    };
    static unsigned int tlsIdx;
    static ThreadStorage* tlsDefault;

public:
    // Each new thread that needs to use Vec3 pools will need to call one of the
    // following 2 functions, to either create its own local storage, or share
    // the default storage already allocated by the main thread
    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

    static Vec3* newPermanent(double x, double y, double z);
    static void clearPool();
    static void resetPool();
    static Vec3* newTemp(double x, double y, double z);
    double x, y, z;

    Vec3() {}
    Vec3(double x, double y, double z);

private:
    Vec3* set(double x, double y, double z);

public:
    Vec3 vectorTo(const Vec3& p) const;
    Vec3 normalize() const;
    double dot(const Vec3& p) const;
    Vec3 cross(const Vec3& p) const;
    Vec3 add(double x, double y, double z) const;
    double distanceTo(const Vec3& p) const;
    double distanceToSqr(const Vec3& p);
    double distanceToSqr(double x2, double y2, double z2) const;
    Vec3 scale(double l) const;
    double length() const;
    std::optional<Vec3> clipX(const Vec3& b, double xt) const;
    std::optional<Vec3> clipY(const Vec3& b, double yt) const;
    std::optional<Vec3> clipZ(const Vec3& b, double zt) const;
    std::wstring toString() const;
    Vec3 lerp(const Vec3& v, double a) const;
    void xRot(float degs);
    void yRot(float degs);
    void zRot(float degs);

    // 4J Added
    double distanceTo(AABB* box);

    Vec3 closestPointOnLine(const Vec3& p1, const Vec3& p2) const;
    double distanceFromLine(const Vec3& p1, const Vec3& p2) const;

    constexpr bool operator==(const Vec3& rhs) const {
        return x == rhs.x and y == rhs.y and z == rhs.z;
    }
};
