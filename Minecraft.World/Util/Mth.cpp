#include "../Platform/stdafx.h"
#include "Mth.h"
#include "Random.h"

const int Mth::BIG_ENOUGH_INT = 1024;
const float Mth::BIG_ENOUGH_FLOAT = BIG_ENOUGH_INT;
const float Mth::RAD_TO_GRAD = PI / 180.0f;
const float Mth::DEGRAD = PI / 180.0f;
const float Mth::RADDEG = 180.0f / PI;

float* Mth::_sin = NULL;

// 4jcraft for clarity
constexpr size_t SIN_TAB_CNT = 65536;

const float Mth::sinScale = 65536.0f / (float)(PI * 2);

// 4J - added - was in static constructor
void Mth::init() {
    _sin = new float[SIN_TAB_CNT];
    for (int i = 0; i < SIN_TAB_CNT; i++) {
        _sin[i] = (float)::sin(i * PI * 2 / (float)SIN_TAB_CNT);
    }
}

float Mth::sin(float i) {
    if (_sin == NULL) init();  // 4J - added

    // 4jcraft changed, what ever this was, it was not safe
    // fmodf returns between -65536 and 65536 (casted)
    // last end is there to shift it into 0 to 65535

    return _sin[(int32_t)fmodf(i * sinScale, (float)SIN_TAB_CNT) &
                (SIN_TAB_CNT - 1)];
}

float Mth::cos(float i) {
    if (_sin == NULL) init();  // 4J - added
    // 4jcraft same thing as ::sin but shift by SIN_TAB_CNT / 4
    // which is aquivalent to shift by pi / 2
    // and again the same modulo logic to cramp and map it onto the computed
    // table

    return _sin[(int32_t)fmodf(i * sinScale + ((float)SIN_TAB_CNT / 4),
                               (float)SIN_TAB_CNT) &
                (SIN_TAB_CNT - 1)];
}

float Mth::sqrt(float x) { return (float)::sqrt(x); }

float Mth::sqrt(double x) { return (float)::sqrt(x); }

int Mth::floor(float v) {
    int i = (int)v;
    return v < i ? i - 1 : i;
}

__int64 Mth::lfloor(double v) {
    __int64 i = (__int64)v;
    return v < i ? i - 1 : i;
}

int Mth::fastFloor(double x) {
    return (int)(x + BIG_ENOUGH_FLOAT) - BIG_ENOUGH_INT;
}

int Mth::floor(double v) {
    int i = (int)v;
    return v < i ? i - 1 : i;
}

int Mth::absFloor(double v) { return (int)(v >= 0 ? v : -v + 1); }

float Mth::abs(float v) { return v >= 0 ? v : -v; }

int Mth::abs(int v) { return v >= 0 ? v : -v; }

int Mth::ceil(float v) {
    int i = (int)v;
    return v > i ? i + 1 : i;
}

int Mth::clamp(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

float Mth::clamp(float value, float min, float max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

double Mth::asbMax(double a, double b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    return a > b ? a : b;
}

int Mth::intFloorDiv(int a, int b) {
    if (a < 0) return -((-a - 1) / b) - 1;
    return a / b;
}

int Mth::nextInt(Random* random, int minInclusive, int maxInclusive) {
    if (minInclusive >= maxInclusive) {
        return minInclusive;
    }
    return random->nextInt(maxInclusive - minInclusive + 1) + minInclusive;
}

float Mth::wrapDegrees(float input) {
    // input %= 360;
    while (input >= 180) {
        input -= 360;
    }
    while (input < -180) {
        input += 360;
    }
    return input;
}

double Mth::wrapDegrees(double input) {
    // input %= 360;
    while (input >= 180) {
        input -= 360;
    }
    while (input < -180) {
        input += 360;
    }
    return input;
}

// 4J Added
bool Mth::almostEquals(double double1, double double2, double precision) {
    return (std::abs(double1 - double2) <= precision);
}
