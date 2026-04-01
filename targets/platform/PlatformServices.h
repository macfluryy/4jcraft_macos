#pragma once

#include "IPlatformInput.h"
#include "IPlatformProfile.h"
#include "IPlatformRender.h"
#include "IPlatformStorage.h"

class IPlatformInput;
class IPlatformProfile;
class IPlatformRender;
class IPlatformStorage;

// Interface references to platform services. World code uses these
// instead of the concrete 4J globals directly. The bindings are
// established by the app layer at startup.

extern IPlatformInput& PlatformInput;
extern IPlatformProfile& PlatformProfile;
extern IPlatformRender& PlatformRender;
extern IPlatformStorage& PlatformStorage;
