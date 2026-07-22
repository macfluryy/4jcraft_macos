#include "RangedAttribute.h"

#include "minecraft/world/entity/ai/attributes/BaseAttribute.h"

RangedAttribute::RangedAttribute(eATTRIBUTE_ID id, double defaultValue,
                                 double minValue, double maxValue)
    : BaseAttribute(id, defaultValue) {
    this->minValue = minValue;
    this->maxValue = maxValue;

    
    
    
    
    
    
}

double RangedAttribute::getMinValue() { return minValue; }

double RangedAttribute::getMaxValue() { return maxValue; }

double RangedAttribute::sanitizeValue(double value) {
    if (value < minValue) value = minValue;
    if (value > maxValue) value = maxValue;

    return value;
}