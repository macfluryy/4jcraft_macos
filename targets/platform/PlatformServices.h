#pragma once

#include "IPlatformFileIO.h"
#include "IPlatformInput.h"
#include "IPlatformLeaderboard.h"
#include "IPlatformNetwork.h"
#include "IPlatformProfile.h"
#include "IPlatformRenderer.h"
#include "IPlatformStorage.h"

// Interface references to platform services. Game code uses these
// instead of concrete globals directly. Bindings are established
// by the app layer at startup.

extern IPlatformFileIO& PlatformFileIO;
extern IPlatformInput& PlatformInput;
extern IPlatformProfile& PlatformProfile;
extern IPlatformRenderer& PlatformRender;
extern IPlatformStorage& PlatformStorage;
