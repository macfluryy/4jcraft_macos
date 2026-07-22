#pragma once

#include <string>



class NumberFormat {
public:
    static std::wstring format(int value) {
        
        wchar_t output[256];
        swprintf(output, 256, L"%d", value);
        std::wstring result = std::wstring(output);
        return result;
    }
};

class DecimalFormat {
private:
    const std::wstring formatString;

public:
    std::wstring format(double value) {
        
        wchar_t output[256];
        swprintf(output, 256, formatString.c_str(), value);
        std::wstring result = std::wstring(output);
        return result;
    }

    
    
    DecimalFormat(std::wstring x) : formatString(x) {};
};