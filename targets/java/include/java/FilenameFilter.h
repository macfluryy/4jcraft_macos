#pragma once

#include <string>

class File;


class FilenameFilter {
public:
    virtual bool accept(File* dir, const std::wstring& name) = 0;
};