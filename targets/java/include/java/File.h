#pragma once

#include <cstdint>
#include <string>
#include <vector>

// 4J Stu - Represents java standard library class

class FileFilter;

class File {
public:
    // The system-dependent path-separator character
    static const wchar_t pathSeparator;

    // 4J Jev, the start of the file root
    static const std::wstring pathRoot;

    File() { m_abstractPathName = L""; }

    File(const File& parent, const std::wstring& child);
    File(const std::wstring& pathname);
    File(const std::wstring& parent, const std::wstring& child);
    bool _delete();
    bool mkdir() const;
    bool mkdirs() const;
    bool exists() const;
    bool isFile() const;
    bool renameTo(File dest);
    std::vector<File*>* listFiles() const;  // Array
    std::vector<File*>* listFiles(FileFilter* filter) const;
    bool isDirectory() const;
    int64_t length();
    int64_t lastModified();
    const std::wstring getPath() const;  // 4J Jev: TODO
    std::wstring getName() const;

    static int hash_fnct(const File& k);
    static bool eq_test(const File& x, const File& y);

private:
    void _init();
    std::wstring m_abstractPathName;

    // 4J Jev, just helper functions, change between paths and
    // std::vector<string>
    // File(std::vector<std::wstring> *path);
};

struct FileKeyHash {
    int operator()(const File& k) const { return File::hash_fnct(k); }
};

struct FileKeyEq {
    bool operator()(const File& x, const File& y) const {
        return File::eq_test(x, y);
    }
};