#include "app/common/src/JavaEdition/JavaServerList.h"

size_t JavaServerList::size() const {
    return m_entries.size();
}

const JavaServerEntry& JavaServerList::at(size_t i) const {
    return m_entries[i];
}

const std::vector<JavaServerEntry>& JavaServerList::entries() const {
    return m_entries;
}

bool JavaServerList::add(const JavaServerEntry& e) {
    if (e.host.empty()) {
        return false;
    }
    m_entries.push_back(e);
    return true;
}

bool JavaServerList::edit(size_t i, const JavaServerEntry& e) {
    if (i >= m_entries.size() || e.host.empty()) {
        return false;
    }
    m_entries[i] = e;
    return true;
}

bool JavaServerList::remove(size_t i) {
    if (i >= m_entries.size()) {
        return false;
    }
    m_entries.erase(m_entries.begin() + static_cast<std::ptrdiff_t>(i));
    return true;
}