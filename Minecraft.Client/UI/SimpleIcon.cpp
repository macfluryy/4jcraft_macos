#include "../Platform/stdafx.h"
#include "SimpleIcon.h"

SimpleIcon::SimpleIcon(const std::wstring& name, const std::wstring& filename,
                       float U0, float V0, float U1, float V1)
    : StitchedTexture(name, filename) {
    u0 = U0;
    u1 = U1;
    v0 = V0;
    v1 = V1;
}
