#pragma once
#include <cstdint>
#include <format>
#include <vector>





















































class TileCompressData_SPU;
class DataInputStream;
class DataOutputStream;
template <typename T>
class XLockFreeStack;

class SparseDataStorage {
    friend class TileCompressData_SPU;

private:
    
    int64_t dataAndCount;  
                           

    
    

    static const int ALL_0_INDEX = 128;

public:
    SparseDataStorage();
    SparseDataStorage(bool isUpper);
    SparseDataStorage(SparseDataStorage* copyFrom);  
    ~SparseDataStorage();

    void setData(
        std::vector<uint8_t>& dataIn,
        unsigned int inOffset);  
                                 
    void getData(std::vector<uint8_t>& retArray,
                 unsigned int retOffset);  
                                           
    int get(int x, int y, int z);          
    void set(int x, int y, int z, int val);  
    typedef void (*tileUpdatedCallback)(int x, int y, int z, void* param,
                                        int yparam);
    int setDataRegion(
        std::vector<uint8_t>& dataIn, int x0, int y0, int z0, int x1, int y1,
        int z1, int offset, tileUpdatedCallback callback, void* param,
        int yparam);  
                      
                      
    int getDataRegion(
        std::vector<uint8_t>& dataInOut, int x0, int y0, int z0, int x1, int y1,
        int z1,
        int offset);  
                      
                      

    static void staticCtor();

    void addNewPlane(int y);
    void getPlaneIndicesAndData(unsigned char** planeIndices,
                                unsigned char** data);
    void updateDataAndCount(int64_t newDataAndCount);
    int compress();

    bool isCompressed();
    void queueForDelete(unsigned char* data);

    static void tick();
    static int deleteQueueIndex;
    static XLockFreeStack<unsigned char> deleteQueue[3];

#if defined(DATA_COMPRESSION_STATS)
    int count;
#endif

    void write(DataOutputStream* dos);
    void read(DataInputStream* dis);
};
