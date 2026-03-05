#include "4J_Render.h"
#include <cstring>

C4JRender RenderManager;

static float s_identityMatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

void C4JRender::Tick() {}
void C4JRender::UpdateGamma(unsigned short usGamma) {}
void C4JRender::MatrixMode(int type) {}
void C4JRender::MatrixSetIdentity() {}
void C4JRender::MatrixTranslate(float x, float y, float z) {}
void C4JRender::MatrixRotate(float angle, float x, float y, float z) {}
void C4JRender::MatrixScale(float x, float y, float z) {}
void C4JRender::MatrixPerspective(float fovy, float aspect, float zNear, float zFar) {}
void C4JRender::MatrixOrthogonal(float left, float right, float bottom, float top, float zNear, float zFar) {}
void C4JRender::MatrixPop() {}
void C4JRender::MatrixPush() {}
void C4JRender::MatrixMult(float *mat) {}
const float* C4JRender::MatrixGet(int type) { return s_identityMatrix; }
void C4JRender::Set_matrixDirty() {}
void C4JRender::Initialise() {}
void C4JRender::InitialiseContext() {}
void C4JRender::StartFrame() {}
void C4JRender::DoScreenGrabOnNextPresent() {}
void C4JRender::Present() {}
void C4JRender::Clear(int flags) {}
void C4JRender::SetClearColour(const float colourRGBA[4]) {}
bool C4JRender::IsWidescreen() { return true; }
bool C4JRender::IsHiDef() { return true; }
void C4JRender::CaptureThumbnail(ImageFileBuffer *pngOut) {}
void C4JRender::CaptureScreen(ImageFileBuffer *jpgOut, XSOCIAL_PREVIEWIMAGE *previewOut) {}
void C4JRender::BeginConditionalSurvey(int identifier) {}
void C4JRender::EndConditionalSurvey() {}
void C4JRender::BeginConditionalRendering(int identifier) {}
void C4JRender::EndConditionalRendering() {}
void C4JRender::DrawVertices(ePrimitiveType PrimitiveType, int count, void *dataIn, eVertexType vType, C4JRender::ePixelShaderType psType) {}
void C4JRender::CBuffLockStaticCreations() {}
int C4JRender::CBuffCreate(int count) { return 0; }
void C4JRender::CBuffDelete(int first, int count) {}
void C4JRender::CBuffStart(int index, bool full) {}
void C4JRender::CBuffClear(int index) {}
int C4JRender::CBuffSize(int index) { return 0; }
void C4JRender::CBuffEnd() {}
bool C4JRender::CBuffCall(int index, bool full) { return false; }
void C4JRender::CBuffTick() {}
void C4JRender::CBuffDeferredModeStart() {}
void C4JRender::CBuffDeferredModeEnd() {}
int C4JRender::TextureCreate() { return 0; }
void C4JRender::TextureFree(int idx) {}
void C4JRender::TextureBind(int idx) {}
void C4JRender::TextureBindVertex(int idx) {}
void C4JRender::TextureSetTextureLevels(int levels) {}
int C4JRender::TextureGetTextureLevels() { return 1; }
void C4JRender::TextureData(int width, int height, void *data, int level, eTextureFormat format) {}
void C4JRender::TextureDataUpdate(int xoffset, int yoffset, int width, int height, void *data, int level) {}
void C4JRender::TextureSetParam(int param, int value) {}
void C4JRender::TextureDynamicUpdateStart() {}
void C4JRender::TextureDynamicUpdateEnd() {}
// really don't know if this is nessesary but didn't find any other functions to load images properly as a png..
// im sorry.
#ifdef __linux__
#include <png.h>
#include <stdio.h>
#include <string.h>

static HRESULT LoadPNGFromRows(png_structp png, png_infop info, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16) png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    unsigned char *buf = new unsigned char[width * height * 4];
    png_bytep *rows = new png_bytep[height];
    for (int y = 0; y < height; y++)
        rows[y] = buf + y * width * 4;
    png_read_image(png, rows);
    delete[] rows;
    // considering i worked on previous projects with raw pngs,,,,, 
    int *pixels = new int[width * height];
    for (int i = 0; i < width * height; i++)
    {
        unsigned char r = buf[i * 4 + 0];
        unsigned char g = buf[i * 4 + 1];
        unsigned char b = buf[i * 4 + 2];
        unsigned char a = buf[i * 4 + 3];
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    delete[] buf;

    pSrcInfo->Width = width;
    pSrcInfo->Height = height;
    *ppDataOut = pixels;
    return S_OK;
}

HRESULT C4JRender::LoadTextureData(const char *szFilename, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
    FILE *fp = fopen(szFilename, "rb");
    if (!fp) return E_FAIL;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) { fclose(fp); return E_FAIL; }
    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_read_struct(&png, NULL, NULL); fclose(fp); return E_FAIL; }
    if (setjmp(png_jmpbuf(png))) { png_destroy_read_struct(&png, &info, NULL); fclose(fp); return E_FAIL; }

    png_init_io(png, fp);
    png_read_info(png, info);

    HRESULT hr = LoadPNGFromRows(png, info, pSrcInfo, ppDataOut);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    return hr;
}

struct PNGMemReader { const unsigned char *data; png_size_t pos; png_size_t size; };

static void png_mem_read(png_structp png, png_bytep out, png_size_t len)
{
    PNGMemReader *r = (PNGMemReader *)png_get_io_ptr(png);
    if (r->pos + len > r->size) len = r->size - r->pos;
    memcpy(out, r->data + r->pos, len);
    r->pos += len;
}

HRESULT C4JRender::LoadTextureData(BYTE *pbData, DWORD dwBytes, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) return E_FAIL;
    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_read_struct(&png, NULL, NULL); return E_FAIL; }
    if (setjmp(png_jmpbuf(png))) { png_destroy_read_struct(&png, &info, NULL); return E_FAIL; }

    PNGMemReader reader = { pbData, 0, dwBytes };
    png_set_read_fn(png, &reader, png_mem_read);
    png_read_info(png, info);

    HRESULT hr = LoadPNGFromRows(png, info, pSrcInfo, ppDataOut);
    png_destroy_read_struct(&png, &info, NULL);
    return hr;
}

#else
HRESULT C4JRender::LoadTextureData(const char *szFilename, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut) { return S_OK; }
HRESULT C4JRender::LoadTextureData(BYTE *pbData, DWORD dwBytes, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut) { return S_OK; }
#endif
HRESULT C4JRender::SaveTextureData(const char *szFilename, D3DXIMAGE_INFO *pSrcInfo, int *ppDataOut) { return S_OK; }
HRESULT C4JRender::SaveTextureDataToMemory(void *pOutput, int outputCapacity, int *outputLength, int width, int height, int *ppDataIn) { return S_OK; }
void C4JRender::TextureGetStats() {}
void* C4JRender::TextureGetTexture(int idx) { return nullptr; }
void C4JRender::StateSetColour(float r, float g, float b, float a) {}
void C4JRender::StateSetDepthMask(bool enable) {}
void C4JRender::StateSetBlendEnable(bool enable) {}
void C4JRender::StateSetBlendFunc(int src, int dst) {}
void C4JRender::StateSetBlendFactor(unsigned int colour) {}
void C4JRender::StateSetAlphaFunc(int func, float param) {}
void C4JRender::StateSetDepthFunc(int func) {}
void C4JRender::StateSetFaceCull(bool enable) {}
void C4JRender::StateSetFaceCullCW(bool enable) {}
void C4JRender::StateSetLineWidth(float width) {}
void C4JRender::StateSetWriteEnable(bool red, bool green, bool blue, bool alpha) {}
void C4JRender::StateSetDepthTestEnable(bool enable) {}
void C4JRender::StateSetAlphaTestEnable(bool enable) {}
void C4JRender::StateSetDepthSlopeAndBias(float slope, float bias) {}
void C4JRender::StateSetFogEnable(bool enable) {}
void C4JRender::StateSetFogMode(int mode) {}
void C4JRender::StateSetFogNearDistance(float dist) {}
void C4JRender::StateSetFogFarDistance(float dist) {}
void C4JRender::StateSetFogDensity(float density) {}
void C4JRender::StateSetFogColour(float red, float green, float blue) {}
void C4JRender::StateSetLightingEnable(bool enable) {}
void C4JRender::StateSetVertexTextureUV(float u, float v) {}
void C4JRender::StateSetLightColour(int light, float red, float green, float blue) {}
void C4JRender::StateSetLightAmbientColour(float red, float green, float blue) {}
void C4JRender::StateSetLightDirection(int light, float x, float y, float z) {}
void C4JRender::StateSetLightEnable(int light, bool enable) {}
void C4JRender::StateSetViewport(eViewportType viewportType) {}
void C4JRender::StateSetEnableViewportClipPlanes(bool enable) {}
void C4JRender::StateSetTexGenCol(int col, float x, float y, float z, float w, bool eyeSpace) {}
void C4JRender::StateSetStencil(int Function, uint8_t stencil_ref, uint8_t stencil_func_mask, uint8_t stencil_write_mask) {}
void C4JRender::StateSetForceLOD(int LOD) {}
void C4JRender::BeginEvent(LPCWSTR eventName) {}
void C4JRender::EndEvent() {}
void C4JRender::Suspend() {}
bool C4JRender::Suspended() { return false; }
void C4JRender::Resume() {}
