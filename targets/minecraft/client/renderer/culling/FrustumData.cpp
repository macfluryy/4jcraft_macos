#include "FrustumData.h"

#include "minecraft/world/phys/AABB.h"

// float** m_Frustum;

FrustumData::FrustumData() {
    this->m_Frustum = new float*[6];
    for (int i = 0; i < 6; i++) {
        this->m_Frustum[i] = new float[4];
    }
    proj = std::vector<float>(16);
    modl = std::vector<float>(16);
    clip = std::vector<float>(16);
}

FrustumData::~FrustumData() {
    for (int i = 0; i < 6; i++) delete[] m_Frustum[i];
    delete[] m_Frustum;
}

bool FrustumData::pointInFrustum(float x, float y, float z) {
    for (int i = 0; i < 6; i++) {
        const float* __restrict__ pl = m_Frustum[i];
        if (pl[A] * x + pl[B] * y + pl[C] * z + pl[D] <= 0)
            return false;
    }
    return true;
}

bool FrustumData::sphereInFrustum(float x, float y, float z, float radius) {
    for (int i = 0; i < 6; i++) {
        const float* __restrict__ pl = m_Frustum[i];
        if (pl[A] * x + pl[B] * y + pl[C] * z + pl[D] <= -radius)
            return false;
    }
    return true;
}

bool FrustumData::cubeFullyInFrustum(double x1, double y1, double z1, double x2,
                                     double y2, double z2) {
    const float fx1 = static_cast<float>(x1), fy1 = static_cast<float>(y1),
                fz1 = static_cast<float>(z1);
    const float fx2 = static_cast<float>(x2), fy2 = static_cast<float>(y2),
                fz2 = static_cast<float>(z2);

    for (int i = 0; i < 6; i++) {
        const float* __restrict__ pl = m_Frustum[i];
        const float pa = pl[A], pb = pl[B], pc = pl[C], pd = pl[D];

        if (pa * fx1 + pb * fy1 + pc * fz1 + pd <= 0) return false;
        if (pa * fx2 + pb * fy1 + pc * fz1 + pd <= 0) return false;
        if (pa * fx1 + pb * fy2 + pc * fz1 + pd <= 0) return false;
        if (pa * fx2 + pb * fy2 + pc * fz1 + pd <= 0) return false;
        if (pa * fx1 + pb * fy1 + pc * fz2 + pd <= 0) return false;
        if (pa * fx2 + pb * fy1 + pc * fz2 + pd <= 0) return false;
        if (pa * fx1 + pb * fy2 + pc * fz2 + pd <= 0) return false;
        if (pa * fx2 + pb * fy2 + pc * fz2 + pd <= 0) return false;
    }

    return true;
}

bool FrustumData::cubeInFrustum(double x1, double y1, double z1, double x2,
                                double y2, double z2) {
    const float fx1 = static_cast<float>(x1), fy1 = static_cast<float>(y1),
                fz1 = static_cast<float>(z1);
    const float fx2 = static_cast<float>(x2), fy2 = static_cast<float>(y2),
                fz2 = static_cast<float>(z2);

    for (int i = 0; i < 6; i++) {
        const float* __restrict__ pl = m_Frustum[i];
        const float pa = pl[A], pb = pl[B], pc = pl[C], pd = pl[D];

        if (pa * fx1 + pb * fy1 + pc * fz1 + pd > 0) continue;
        if (pa * fx2 + pb * fy1 + pc * fz1 + pd > 0) continue;
        if (pa * fx1 + pb * fy2 + pc * fz1 + pd > 0) continue;
        if (pa * fx2 + pb * fy2 + pc * fz1 + pd > 0) continue;
        if (pa * fx1 + pb * fy1 + pc * fz2 + pd > 0) continue;
        if (pa * fx2 + pb * fy1 + pc * fz2 + pd > 0) continue;
        if (pa * fx1 + pb * fy2 + pc * fz2 + pd > 0) continue;
        if (pa * fx2 + pb * fy2 + pc * fz2 + pd > 0) continue;

        return false;
    }

    return true;
}

bool FrustumData::isVisible(AABB* aabb) {
    return cubeInFrustum(aabb->x0, aabb->y0, aabb->z0, aabb->x1, aabb->y1,
                         aabb->z1);
}
