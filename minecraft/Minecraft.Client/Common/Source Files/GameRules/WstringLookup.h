#pragma once

class WstringLookup {
private:
    unsigned int numIDs;
    std::unordered_map<std::wstring, unsigned int> str2int;
    std::vector<std::wstring> int2str;

public:
    WstringLookup();

    std::wstring lookup(unsigned int id);

    unsigned int lookup(std::wstring);

    void getTable(std::wstring** lookup, unsigned int* len);
};
