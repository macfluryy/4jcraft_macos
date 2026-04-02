#include "PlatformServices.h"

#include "../4J.Input/4J_Input.h"
#include "../4J.Profile/4J_Profile.h"
#include "../4J.Render/4J_Render.h"
#include "../4J.Storage/4J_Storage.h"

class IPlatformInput;
class IPlatformProfile;
class IPlatformRenderer;
class IPlatformStorage;

IPlatformInput& PlatformInput = InputManager;
IPlatformProfile& PlatformProfile = ProfileManager;
IPlatformRenderer& PlatformRender = RenderManager;
IPlatformStorage& PlatformStorage = StorageManager;
