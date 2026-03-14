#pragma once

#include "../../Util/Class.h"

class CompoundTag;

class SavedData : public std::enable_shared_from_this<SavedData> {
public:
    const std::wstring id;

private:
    bool dirty;

public:
    SavedData(const std::wstring& id);
    virtual ~SavedData() {}

    virtual void load(CompoundTag* tag) = 0;
    virtual void save(CompoundTag* tag) = 0;

    void setDirty();
    void setDirty(bool dirty);
    bool isDirty();
};
