#pragma once

class SmoothFloat {
private:
    float targetValue;
    float remainingValue;
    float lastAmount;

public:
    SmoothFloat();  
    float getNewDeltaValue(float deltaValue, float accelerationAmount);
    float getTargetValue();
};