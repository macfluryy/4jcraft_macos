#include "PlatformServices.h"
#include "StdFileIO.h"

#include "sdl2/Input.h"
#include "sdl2/Profile.h"
#include "sdl2/Render.h"
#include "sdl2/Storage.h"

static StdFileIO s_stdFileIO;

IPlatformFileIO& PlatformFileIO = s_stdFileIO;
IPlatformInput& PlatformInput = InputManager;
IPlatformProfile& PlatformProfile = ProfileManager;
IPlatformRenderer& PlatformRender = RenderManager;
IPlatformStorage& PlatformStorage = StorageManager;
