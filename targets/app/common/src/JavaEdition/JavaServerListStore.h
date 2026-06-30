#pragma once
#include "app/common/src/JavaEdition/JavaServerList.h"
#include "java/File.h"

class JavaServerListStore {
public:
    explicit JavaServerListStore(const File& workingDirectory);
    struct LoadResult {
        JavaServerList list;
        bool hadCorruptLines = false;
    };

    LoadResult load();
    bool save(const JavaServerList& list);

private:
    File m_file;
};