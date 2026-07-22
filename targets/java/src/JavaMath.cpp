#include "java/JavaMath.h"

#include <cmath>
#include <limits>

#include "java/Random.h"

Random Math::rand = Random();
















double Math::random() { return Math::rand.nextDouble(); }














int64_t Math::round(double d) {
    
    

    d = std::floor(d + 0.5);

    
    if (d >= (double)std::numeric_limits<int64_t>::max()) {
        return std::numeric_limits<int64_t>::max();

    } else if (d <= (double)std::numeric_limits<int64_t>::min()) {
        return std::numeric_limits<int64_t>::min();
    }

    return (int64_t)d;
}

int Math::_max(int a, int b) { return a > b ? a : b; }

int Math::_min(int a, int b) { return a < b ? a : b; }

float Math::_max(float a, float b) { return a > b ? a : b; }

float Math::_min(float a, float b) { return a < b ? a : b; }

float Math::wrapDegrees(float input) {
    while (input >= 360.0f) input -= 360.0f;
    if (input >= 180.0f) input -= 360.0f;
    if (input < -180.0f) input += 360.0f;
    return input;
}

double Math::wrapDegrees(double input) {
    while (input >= 360.0) input -= 360.0;
    if (input >= 180.0) input -= 360.0;
    if (input < -180.0) input += 360.0;
    return input;
}