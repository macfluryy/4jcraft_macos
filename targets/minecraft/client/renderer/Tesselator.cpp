#include "Tesselator.h"

#include <GL/gl.h>

#include <vector>

#include "platform/sdl2/Render.h"
#include "app/mac/MacGame.h"
#include "app/include/stubs.h"
#include "minecraft/client/MemoryTracker.h"

bool Tesselator::TRIANGLE_MODE = false;
bool Tesselator::USE_VBO = false;

















thread_local Tesselator* Tesselator::m_tlsInstance = nullptr;

Tesselator* Tesselator::getInstance() { return m_tlsInstance; }

void Tesselator::CreateNewThreadStorage(int bytes) {
    Tesselator::m_tlsInstance = new Tesselator(bytes / 4);
}


Tesselator::Tesselator(int size) {
    
    
    vertices = 0;
    hasColor = false;
    hasTexture = false;
    hasTexture2 = false;
    hasNormal = false;
    p = 0;
    count = 0;
    _noColor = false;
    tesselating = false;
    vboMode = false;
    vboId = 0;
    vboCounts = 10;

    
    
    u = v = 0;
    col = 0;
    mode = 0;
    xo = yo = zo = 0;
    xoo = yoo = zoo = 0;  
    _normal = 0;

    useCompactFormat360 = false;             
    mipmapEnable = true;                     
    useProjectedTexturePixelShader = false;  

    this->size = size;

    _array = new std::vector<int>(size);

    vboMode =
        USE_VBO;  
                  
    if (vboMode) {
        vboIds = MemoryTracker::createIntBuffer(vboCounts);
        ARBVertexBufferObject::glGenBuffersARB(vboIds);
    }
}

Tesselator* Tesselator::getUniqueInstance(int size) {
    return new Tesselator(size);
}

void Tesselator::end() {
    
    
    tesselating = false;
    if (vertices > 0) {
        
        
        if (!hasColor) {
            
            
            
            
            
            unsigned int* pColData = (unsigned int*)_array->data();
            pColData += 5;
            for (int i = 0; i < vertices; i++) {
                *pColData = 0x00000000;
                pColData += 8;
            }
        }
        if (mode == GL_QUADS && TRIANGLE_MODE) {
            
            RenderManager.DrawVertices(
                C4JRender::PRIMITIVE_TYPE_TRIANGLE_LIST, vertices,
                _array->data(),
                useCompactFormat360
                    ? C4JRender::VERTEX_TYPE_COMPRESSED
                    : C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,
                useProjectedTexturePixelShader
                    ? C4JRender::PIXEL_SHADER_TYPE_PROJECTION
                    : C4JRender::PIXEL_SHADER_TYPE_STANDARD);
        } else {
            
            
            
            
            
            
            int vertexCount = vertices;
            if (useCompactFormat360) {
                RenderManager.DrawVertices(
                    (C4JRender::ePrimitiveType)mode, vertexCount,
                    _array->data(), C4JRender::VERTEX_TYPE_COMPRESSED,
                    C4JRender::PIXEL_SHADER_TYPE_STANDARD);
            } else {
                if (useProjectedTexturePixelShader) {
                    RenderManager.DrawVertices(
                        (C4JRender::ePrimitiveType)mode, vertexCount,
                        _array->data(),
                        C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_TEXGEN,
                        C4JRender::PIXEL_SHADER_TYPE_PROJECTION);
                } else {
                    RenderManager.DrawVertices(
                        (C4JRender::ePrimitiveType)mode, vertexCount,
                        _array->data(),
                        C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,
                        C4JRender::PIXEL_SHADER_TYPE_STANDARD);
                }
            }
        }
        
        
        
        
        
        
        
        
        
    }

    clear();
}

void Tesselator::clear() {
    vertices = 0;

    p = 0;
    count = 0;
}

void Tesselator::begin() {
    begin(GL_QUADS);
    bounds.reset();  
}

void Tesselator::useProjectedTexture(bool enable) {
    useProjectedTexturePixelShader = enable;
}

void Tesselator::useCompactVertices(bool enable) {
    useCompactFormat360 = enable;
}

bool Tesselator::getCompactVertices() { return useCompactFormat360; }

bool Tesselator::setMipmapEnable(bool enable) {
    bool prev = mipmapEnable;
    mipmapEnable = enable;
    return prev;
}

void Tesselator::begin(int mode) {
    



    tesselating = true;

    clear();
    this->mode = mode;
    hasNormal = false;
    hasColor = false;
    hasTexture = false;
    hasTexture2 = false;
    _noColor = false;
}

void Tesselator::tex(float u, float v) {
    hasTexture = true;
    this->u = u;
    this->v = v;
}

void Tesselator::tex2(int tex2) {
    hasTexture2 = true;
    this->_tex2 = tex2;
}

void Tesselator::color(float r, float g, float b) {
    color((int)(r * 255), (int)(g * 255), (int)(b * 255));
}

void Tesselator::color(float r, float g, float b, float a) {
    color((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(a * 255));
}

void Tesselator::color(int r, int g, int b) { color(r, g, b, 255); }

void Tesselator::color(int r, int g, int b, int a) {
    if (_noColor) return;

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (a > 255) a = 255;
    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;
    if (a < 0) a = 0;

    hasColor = true;
    
    col = (r << 24) | (g << 16) | (b << 8) | (a);
}

void Tesselator::color(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    color(r & 0xff, g & 0xff, b & 0xff);
}

void Tesselator::vertexUV(float x, float y, float z, float u, float v) {
    tex(u, v);
    vertex(x, y, z);
}











































void Tesselator::packCompactQuad() {
    
    for (int i = 0; i < 4; i++) {
        m_ix[i] += 16 * 128;
        m_iy[i] += 16 * 128;
        m_iz[i] += 16 * 128;
    }
    
    unsigned int minx = m_ix[0];
    unsigned int miny = m_iy[0];
    unsigned int minz = m_iz[0];
    for (int i = 1; i < 4; i++) {
        if (m_ix[i] < minx) minx = m_ix[i];
        if (m_iy[i] < miny) miny = m_iy[i];
        if (m_iz[i] < minz) minz = m_iz[i];
    }
    
    
    
    
    unsigned int basex = (minx >> 7);
    unsigned int basey = (miny >> 7);
    unsigned int basez = (minz >> 7);
    
    
    
    if (basex == 32) basex = 31;
    if (basey == 32) basey = 31;
    if (basez == 32) basez = 31;
    
    
    for (int i = 0; i < 4; i++) {
        m_ix[i] -= basex << 7;
        m_iy[i] -= basey << 7;
        m_iz[i] -= basez << 7;
    }
    
    unsigned int* data = (unsigned int*)&_array->data()[p];

    for (int i = 0; i < 4; i++) {
        data[i * 2 + 0] = (m_ix[i] << 8) | (m_iy[i]);
        data[i * 2 + 1] = (m_iz[i] << 24) | (m_clr[i]);
    }
    data[0] |= (basex << 26) | (basey << 21) | (basez << 16);

    
    unsigned int minu = m_u[0];
    unsigned int minv = m_v[0];
    unsigned int maxu = m_u[0];
    unsigned int maxv = m_v[0];

    for (int i = 1; i < 4; i++) {
        if (m_u[i] < minu) minu = m_u[i];
        if (m_v[i] < minv) minv = m_v[i];
        if (m_u[i] > maxu) maxu = m_u[i];
        if (m_v[i] > maxv) maxv = m_v[i];
    }
    
    
    
    
    
    
    
    
    unsigned int du = maxu - minu;
    unsigned int dv = maxv - minv;
    if (du > 255) du = 255;
    if (dv > 255) dv = 255;
    
    
    
    bool axisAligned = true;
    for (int i = 0; i < 4; i++) {
        if (!(((m_u[i] == minu) || (m_u[i] == maxu)) &&
              ((m_v[i] == minv) || (m_v[i] == maxv)))) {
            axisAligned = false;
        }
    }

    if (axisAligned) {
        
        
        for (int i = 0; i < 4; i++) {
            unsigned int code = 0;
            if (m_u[i] == maxu) code |= 2;
            if (m_v[i] == maxv) code |= 1;
            data[i * 2 + 1] |= code;
            data[i * 2 + 1] |= m_t2[i] << 16;
        }
        
        data[1 * 2 + 0] |= minu << 16;
        data[2 * 2 + 0] |= minv << 16;
        data[3 * 2 + 0] |= (du << 24 | dv << 16);

        p += 4 * 2;
    } else {
        
        
        
        
        
        

        for (int i = 0; i < 4; i++) {
            data[i * 2 + 1] |= (4);  
                                     
            data[i * 2 + 1] |= m_t2[i] << 16;
            data[8 + i * 2] =
                0;  
                    
            data[9 + i * 2] = m_u[i] << 16 | m_v[i];
        }

        
        p += 8 * 2;
    }
}

typedef unsigned short hfloat;
extern hfloat convertFloatToHFloat(float f);
extern float convertHFloatToFloat(hfloat hf);

#if defined(__linux__) && defined(__APPLE__)
namespace {
void packLinuxLightmapCoords(int tex2, std::int16_t& u, std::int16_t& v) {
    u = static_cast<std::int16_t>(tex2 & 0xffff);
    v = static_cast<std::int16_t>((tex2 >> 16) & 0xffff);

    
    
    
    u += 8;
    v += 8;
}

void logLinuxPackedLightmapCoords(const char* path, int tex2, std::int16_t u,
                                  std::int16_t v) {
    static int logCount = 0;
    if (logCount >= 16) return;

    ++logCount;
    app.DebugPrintf(
        "[linux-lightmap] %s raw=0x%08x packed=(%d,%d) sampled=(%.4f,%.4f)\n",
        path, tex2, (int)u, (int)v, u / 256.0f, v / 256.0f);
}
}  
#endif

void Tesselator::vertex(float x, float y, float z) {
    bounds.addVert(x + xo, y + yo, z + zo);  
    count++;

    
    
    float uu = mipmapEnable ? u : (u + 1.0f);

    
    
    if (useCompactFormat360) {
        unsigned int ucol = (unsigned int)col;

        unsigned short packedcol = ((col & 0xf8000000) >> 16) |
                                   ((col & 0x00fc0000) >> 13) |
                                   ((col & 0x0000f800) >> 11);
        int ipackedcol = ((int)packedcol) & 0xffff;  

        ipackedcol -= 32768;  
        ipackedcol &= 0xffff;

        std::int16_t* pShortData = (std::int16_t*)&_array->data()[p];

        pShortData[0] = (((int)((x + xo) * 1024.0f)) & 0xffff);
        pShortData[1] = (((int)((y + yo) * 1024.0f)) & 0xffff);
        pShortData[2] = (((int)((z + zo) * 1024.0f)) & 0xffff);
        pShortData[3] = ipackedcol;
        pShortData[4] = (((int)(uu * 8192.0f)) & 0xffff);
        pShortData[5] = (((int)(v * 8192.0f)) & 0xffff);
        std::int16_t u2 = static_cast<std::int16_t>(_tex2 & 0xffff);
        std::int16_t v2 = static_cast<std::int16_t>((_tex2 >> 16) & 0xffff);
#if defined(__linux__) && defined(__APPLE__)
        packLinuxLightmapCoords(_tex2, u2, v2);
        logLinuxPackedLightmapCoords("compact", _tex2, u2, v2);
#endif
        pShortData[6] = u2;
        pShortData[7] = v2;

        p += 4;

        vertices++;

        if (vertices % 4 == 0 &&
            ((p >= size - 4 * 4) ||
             ((p / 4) >=
              65532)))  
                        

        {
            end();
            tesselating = true;
        }
    } else {
        if (mode == GL_QUADS && TRIANGLE_MODE && count % 4 == 0) {
            for (int i = 0; i < 2; i++) {
                int offs = 8 * (3 - i);
                if (hasTexture) {
                    _array->data()[p + 3] = _array->data()[p - offs + 3];
                    _array->data()[p + 4] = _array->data()[p - offs + 4];
                }
                if (hasColor) {
                    _array->data()[p + 5] = _array->data()[p - offs + 5];
                }

                _array->data()[p + 0] = _array->data()[p - offs + 0];
                _array->data()[p + 1] = _array->data()[p - offs + 1];
                _array->data()[p + 2] = _array->data()[p - offs + 2];

                vertices++;
                p += 8;
            }
        }

        if (hasTexture) {
            float* fdata = (float*)(_array->data() + p + 3);
            *fdata++ = uu;
            *fdata++ = v;
        }
        if (hasColor) {
            _array->data()[p + 5] = col;
        }
        if (hasNormal) {
            _array->data()[p + 6] = _normal;
        }
        if (hasTexture2) {

#if defined(__linux__) && defined(__APPLE__)
            std::int16_t tex2U;
            std::int16_t tex2V;
            packLinuxLightmapCoords(_tex2, tex2U, tex2V);
            logLinuxPackedLightmapCoords("standard", _tex2, tex2U, tex2V);
            std::int16_t* pShortArray = (std::int16_t*)&_array->data()[p + 7];
            pShortArray[0] = tex2U;
            pShortArray[1] = tex2V;
#else
            _array->data()[p + 7] = _tex2;
#endif
        } else {
            
            
            
            *(unsigned int*)(&_array->data()[p + 7]) = 0xfe00fe00;
        }

        float* fdata = (float*)(_array->data() + p);
        *fdata++ = (x + xo);
        *fdata++ = (y + yo);
        *fdata++ = (z + zo);
        p += 8;

        vertices++;
        if (vertices % 4 == 0 && p >= size - 8 * 4) {
            end();
            tesselating = true;
        }
    }
}

void Tesselator::color(int c) {
    int r = ((c >> 16) & 255);
    int g = ((c >> 8) & 255);
    int b = ((c) & 255);
    color(r, g, b);
}

void Tesselator::color(int c, int alpha) {
    int r = ((c >> 16) & 255);
    int g = ((c >> 8) & 255);
    int b = ((c) & 255);
    color(r, g, b, alpha);
}

void Tesselator::noColor() { _noColor = true; }

void Tesselator::normal(float x, float y, float z) {
    hasNormal = true;

    
    std::int8_t xx = (std::int8_t)(x * 127);
    std::int8_t yy = (std::int8_t)(y * 127);
    std::int8_t zz = (std::int8_t)(z * 127);
    _normal = (xx & 0xff) | ((yy & 0xff) << 8) | ((zz & 0xff) << 16);
}

void Tesselator::offset(float xo, float yo, float zo) {
    this->xo = xo;
    this->yo = yo;
    this->zo = zo;

    
    this->xoo = xo;
    this->yoo = yo;
    this->zoo = zo;
}

void Tesselator::addOffset(float x, float y, float z) {
    xo += x;
    yo += y;
    zo += z;
}

bool Tesselator::hasMaxVertices() { return false; }