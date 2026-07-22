#pragma once
#include "minecraft/client/renderer/texture/StitchedTexture.h"

class ClockTexture : public StitchedTexture {
private:
    double rot, rota;
    int m_iPad;
    ClockTexture* m_dataTexture;

public:
    ClockTexture();
    ClockTexture(int iPad, ClockTexture* dataTexture);
    void cycleFrames();

    virtual int getSourceWidth() const;
    virtual int getSourceHeight() const;
    virtual int getFrames();
    virtual void freeFrameTextures();  
    virtual bool hasOwnData();         
};