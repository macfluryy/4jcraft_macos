#include "Render.h"

// Command Buffers
void C4JRender::CBuffLockStaticCreations() {}
int C4JRender::CBuffSize(int) { return 0; }
void C4JRender::CBuffTick() {}
void C4JRender::CBuffDeferredModeStart() {}
void C4JRender::CBuffDeferredModeEnd() {}

// Render States
void C4JRender::StateSetLightEnable(int, bool) {}
void C4JRender::StateSetEnableViewportClipPlanes(bool) {}
void C4JRender::StateSetForceLOD(int) {}
void C4JRender::StateSetTexGenCol(int, float, float, float, float, bool) {}

// Textures
void C4JRender::TextureDynamicUpdateStart() {}
void C4JRender::TextureDynamicUpdateEnd() {}
void C4JRender::TextureGetStats() {}
void* C4JRender::TextureGetTexture(int) { return nullptr; }

int C4JRender::SaveTextureData(const char*, D3DXIMAGE_INFO*, int*) { return 0; }
int C4JRender::SaveTextureDataToMemory(void*, int, int*, int, int, int*) {
    return 0;
}

// Screen/Image Capturing
void C4JRender::DoScreenGrabOnNextPresent() {}
void C4JRender::CaptureThumbnail(ImageFileBuffer*) {}
void C4JRender::CaptureScreen(ImageFileBuffer*, XSOCIAL_PREVIEWIMAGE*) {}

// Conditional Rendering & Events
void C4JRender::BeginConditionalSurvey(int) {}
void C4JRender::EndConditionalSurvey() {}
void C4JRender::BeginConditionalRendering(int) {}
void C4JRender::EndConditionalRendering() {}
void C4JRender::BeginEvent(const wchar_t*) {}
void C4JRender::EndEvent() {}
void C4JRender::Tick() {}

// Lifecycle
void C4JRender::Suspend() {}
bool C4JRender::Suspended() { return false; }
void C4JRender::Resume() {}