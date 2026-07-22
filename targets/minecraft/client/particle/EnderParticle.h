#pragma once
#include "Particle.h"
#include "java/Class.h"

class Level;





class EnderParticle : public Particle {
public:
    virtual eINSTANCEOF GetType() { return eType_ENDERPARTICLE; }

private:
    float oSize;
    double xStart, yStart, zStart;

public:
    EnderParticle(Level* level, double x, double y, double z, double xd,
                  double yd, double zd);
    virtual void render(Tesselator* t, float a, float xa, float ya, float za,
                        float xa2, float za2);
    virtual int getLightColor(float a);  
    virtual float getBrightness(float a);
    virtual void tick();
};