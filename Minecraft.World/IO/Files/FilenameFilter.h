#pragma once

#include "../../Platform/stdafx.h"

class File;

// 4J Jev, java lirary interface.
class FilenameFilter {
public:
    virtual bool accept(File* dir, const std::wstring& name) = 0;
};