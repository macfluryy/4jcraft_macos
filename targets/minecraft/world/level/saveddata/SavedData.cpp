
#include <string>

#include "SavedData.h"
#include "minecraft/world/level/saveddata/SavedData.h"

SavedData::SavedData(const std::wstring& id) : id(id) { dirty = false; }

void SavedData::setDirty() { setDirty(true); }

void SavedData::setDirty(bool dirty) { this->dirty = dirty; }

bool SavedData::isDirty() { return dirty; }