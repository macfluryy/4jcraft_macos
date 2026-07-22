#pragma once
#include <format>
#include <unordered_map>
#include <vector>

class ByteBuffer;
class IntBuffer;
class FloatBuffer;






class MemoryTracker {
private:
    static std::unordered_map<int, int> GL_LIST_IDS;
    static std::vector<int> TEXTURE_IDS;

public:
    static int genLists(int count);
    static int genTextures();
    static void releaseLists(int id);
    static void releaseTextures();
    static void release();
    
    static ByteBuffer* createByteBuffer(int size);
    static IntBuffer* createIntBuffer(int size);
    static FloatBuffer* createFloatBuffer(int size);
};
