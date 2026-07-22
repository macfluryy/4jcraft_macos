#pragma once
#include "FrustumData.h"

class FloatBuffer;

class Frustum : public FrustumData {
private:
    static Frustum* frustum;

public:
    static FrustumData* getFrustum();

    
    
    
    
    
    
    

private:
    void normalizePlane(float** frustum, int side);

    FloatBuffer* _proj;
    FloatBuffer* _modl;
    FloatBuffer* _clip;

    void calculateFrustum();

    Frustum();
    ~Frustum();
};