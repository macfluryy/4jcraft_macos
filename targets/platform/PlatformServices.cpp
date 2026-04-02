#include "PlatformServices.h"

#include "../platform/sdl2/Input.h"
#include "../platform/sdl2/Profile.h"
#include "../platform/sdl2/Render.h"
#include "../platform/sdl2/Storage.h"

class IPlatformInput;
class IPlatformProfile;
class IPlatformRenderer;
class IPlatformStorage;

IPlatformInput& PlatformInput = InputManager;
IPlatformProfile& PlatformProfile = ProfileManager;
IPlatformRenderer& PlatformRender = RenderManager;
IPlatformStorage& PlatformStorage = StorageManager;
