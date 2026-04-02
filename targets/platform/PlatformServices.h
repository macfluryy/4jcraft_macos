#pragma once

#include "IPlatformInput.h"
#include "IPlatformProfile.h"
#include "IPlatformRenderer.h"
#include "IPlatformStorage.h"

class IPlatformInput;
class IPlatformProfile;
class IPlatformRenderer;
class IPlatformStorage;

// Interface references to platform services. World code uses these
// instead of the concrete 4J globals directly. The bindings are
// established by the app layer at startup.

extern IPlatformInput& PlatformInput;
extern IPlatformProfile& PlatformProfile;
extern IPlatformRenderer& PlatformRender;
extern IPlatformStorage& PlatformStorage;
