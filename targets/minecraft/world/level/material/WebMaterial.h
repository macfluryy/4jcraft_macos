#pragma once
#include "Material.h"



class WebMaterial : public Material {
public:
    WebMaterial(MaterialColor* color) : Material(color) {}
    virtual bool blocksMotion() { return false; }
};