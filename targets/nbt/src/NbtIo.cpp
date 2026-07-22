#include "nbt/NbtIo.h"

#include "java/InputOutputStream/BufferedOutputStream.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/System.h"
#include "nbt/CompoundTag.h"
#include "nbt/Tag.h"

class DataInput;
class DataOutput;
class OutputStream;

CompoundTag* NbtIo::readCompressed(InputStream* in) {
    
    DataInputStream dis =
        DataInputStream(in);  
    CompoundTag* ret = NbtIo::read((DataInput*)&dis);
    dis.close();
    return ret;
}

void NbtIo::writeCompressed(CompoundTag* tag, OutputStream* out) {
    
    
    
    BufferedOutputStream bos = BufferedOutputStream(out, 1024);
    DataOutputStream dos =
        DataOutputStream(&bos);  
    NbtIo::write(tag, &dos);
    dos.close();
}



CompoundTag* NbtIo::decompress(std::vector<uint8_t> buffer) {
    ByteArrayInputStream bais = ByteArrayInputStream(buffer);
    
    DataInputStream in =
        DataInputStream(&bais);  
    CompoundTag* ret = NbtIo::read((DataInput*)&in);
    bais.reset();  
                   
    in.close();
    return ret;
}

std::vector<uint8_t> NbtIo::compress(CompoundTag* tag) {
    
    ByteArrayOutputStream baos = ByteArrayOutputStream();
    DataOutputStream dos =
        DataOutputStream(&baos);  
    NbtIo::write(tag, &dos);

    std::vector<uint8_t> ret(baos.buf.size());
    System::arraycopy(baos.buf, 0, &ret, 0, baos.buf.size());
    dos.close();
    return ret;
}

CompoundTag* NbtIo::read(DataInput* dis) {
    Tag* tag = Tag::readNamedTag(dis);

    if (tag->getId() == Tag::TAG_Compound) return (CompoundTag*)tag;

    if (tag != nullptr) delete tag;
    
    return nullptr;
}

void NbtIo::write(CompoundTag* tag, DataOutput* dos) {
    Tag::writeNamedTag(tag, dos);
}