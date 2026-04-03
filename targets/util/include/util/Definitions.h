#pragma once

#define MAX_PATH_SIZE 256

enum ByteOrder {
    BIGENDIAN,
    LITTLEENDIAN,

    LOCALSYSTEM_ENDIAN = LITTLEENDIAN,
};
enum EDefaultSkins {
    eDefaultSkins_ServerSelected,
    eDefaultSkins_Skin0,
    eDefaultSkins_Skin1,
    eDefaultSkins_Skin2,
    eDefaultSkins_Skin3,
    eDefaultSkins_Skin4,
    eDefaultSkins_Skin5,
    eDefaultSkins_Skin6,
    eDefaultSkins_Skin7,

    eDefaultSkins_Count,
};
