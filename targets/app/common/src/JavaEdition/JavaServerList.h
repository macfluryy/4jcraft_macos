#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "app/common/src/JavaEdition/JavaServerEntry.h"

class JavaServerList {
public:
    size_t size() const;

    const JavaServerEntry& at(size_t i) const;
    const std::vector<JavaServerEntry>& entries() const;

    bool add(const JavaServerEntry& e);
    bool edit(size_t i, const JavaServerEntry& e);
    bool remove(size_t i);

private:
    std::vector<JavaServerEntry> m_entries;
};