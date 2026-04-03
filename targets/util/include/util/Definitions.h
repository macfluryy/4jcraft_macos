#pragma once

#define MAX_PATH_SIZE 256

enum ByteOrder {
    BIGENDIAN,
    LITTLEENDIAN,

    LOCALSYSTEM_ENDIAN = LITTLEENDIAN,
};
