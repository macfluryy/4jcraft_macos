#pragma once
class Synth
{
public:
    virtual double getValue(double x, double y) = 0;
    virtual ~Synth(){}

    doubleArray create(int width, int height);
};

