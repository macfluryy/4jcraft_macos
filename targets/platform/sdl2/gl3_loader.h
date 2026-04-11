#pragma once

// macOS: use Apple's OpenGL 3.3 Core framework directly.
// Suppress deprecation warnings — we know OpenGL is deprecated on macOS 10.14+
// but this is intentional for legacy compatibility.
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#include <cstdio>

// ── Constants that OpenGL 3.3 Core dropped from the legacy API ──────────────
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88B4
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
// GL_QUADS is not available in Core Profile; quads are converted to triangles
// in DrawVertices(), so this constant is only used as a sentinel tag.
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif

// ── Runtime sanity check ─────────────────────────────────────────────────────
// Call once after the OpenGL context is made current.
static inline bool gl3_load() {
    const char* ver = (const char*)glGetString(GL_VERSION);
    if (!ver) {
        fprintf(stderr, "[gl3_loader] ERROR: No active GL context found.\n");
        return false;
    }
    int major = 0, minor = 0;
    if (sscanf(ver, "%d.%d", &major, &minor) >= 2) {
        if (major < 3 || (major == 3 && minor < 3)) {
            fprintf(stderr,
                    "[gl3_loader] ERROR: Need GL 3.3, but system provides %s\n",
                    ver);
            return false;
        }
    }
    fprintf(stderr, "[gl3_loader] GL Version: %s — macOS Core Profile OK.\n",
            ver);
    return true;
}