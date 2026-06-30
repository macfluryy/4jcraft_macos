#pragma once
#include <cstdint>

#include "app/common/src/JavaEdition/JavaBlockIdMap.h"

namespace JavaItemIdMap {

inline int javaItemSubstitute(int javaId) {
    switch (javaId) {
        case 386: case 387: return 340;
        case 423: case 411: return 363;
        case 424: case 412: return 364;
        case 413:           return 282;
        case 414:           return 371;
        case 415:           return 334;
        case 427: case 428: case 429:
        case 430: case 431: return 324;
        default:            return -1;
    }
}


inline int toLce(int javaItemId) {
    if (javaItemId >= 0 && javaItemId <= 255) {
        return JavaBlockIdMap::toLce(static_cast<uint8_t>(javaItemId));
    }
    const int sub = javaItemSubstitute(javaItemId);
    if (sub >= 0) return sub;
    if ((javaItemId >= 256 && javaItemId <= 421) ||
        (javaItemId >= 2256 && javaItemId <= 2267)) {
        return javaItemId;
    }
    return 1;
}

inline int toJava(int lceItemId) {
    if (lceItemId >= 0 && lceItemId <= 255) {
        for (int j = 0; j <= 255; ++j) {
            if (JavaBlockIdMap::toLce(static_cast<uint8_t>(j)) == lceItemId) {
                return j;
            }
        }
        return lceItemId;
    }
    if ((lceItemId >= 256 && lceItemId <= 421) ||
        (lceItemId >= 2256 && lceItemId <= 2267)) {
        for (int j = 256; j <= 431; ++j) {
            if (javaItemSubstitute(j) == lceItemId) return j;
        }
        return lceItemId;
    }
    return -1;
}

}