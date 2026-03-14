#include "../../Platform/stdafx.h"
#include "../../IO/Streams/ByteBuffer.h"
#include "ZoneIO.h"

namespace {
bool SeekFile(std::FILE* file, __int64 offset) {
#if defined(_WIN32)
    return _fseeki64(file, offset, SEEK_SET) == 0;
#else
    return fseeko(file, static_cast<off_t>(offset), SEEK_SET) == 0;
#endif
}
}  // namespace

ZoneIo::ZoneIo(std::FILE* channel, __int64 pos) {
    this->channel = channel;
    this->pos = pos;
}

void ZoneIo::write(byteArray bb, int size) {
    ByteBuffer* buff = ByteBuffer::wrap(bb);
    //    if (bb.length != size) throw new IllegalArgumentException("Expected "
    //    + size + " bytes, got " + bb.length);	// 4J - TODO
    buff->order(ZonedChunkStorage::BYTEORDER);
    buff->position(bb.length);
    buff->flip();
    write(buff, size);
    delete buff;
}

void ZoneIo::write(ByteBuffer* bb, int size) {
    SeekFile(channel, pos);
    std::fwrite(bb->getBuffer(), 1, bb->getSize(), channel);
    pos += size;
}

ByteBuffer* ZoneIo::read(int size) {
    byteArray bb = byteArray(size);
    SeekFile(channel, pos);
    ByteBuffer* buff = ByteBuffer::wrap(bb);
    // 4J - to investigate - why is this buffer flipped before anything goes in
    // it?
    buff->order(ZonedChunkStorage::BYTEORDER);
    buff->position(size);
    buff->flip();
    std::fread(buff->getBuffer(), 1, buff->getSize(), channel);
    pos += size;
    return buff;
}

void ZoneIo::flush() {
    // 4J - was channel.force(false);
}
