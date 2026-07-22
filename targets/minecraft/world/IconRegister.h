#pragma once

class Icon;

class IconRegister {
public:
    
    virtual Icon* registerIcon(const std::wstring& name) = 0;
    virtual int getIconType() = 0;
};