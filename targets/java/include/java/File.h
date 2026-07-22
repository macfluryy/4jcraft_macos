#pragma once

#include <cstdint>
#include <string>
#include <vector>



class FileFilter;

class File {
public:
    
    static const wchar_t pathSeparator;

    
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
    std::vector<File*>* listFiles() const;  
    std::vector<File*>* listFiles(FileFilter* filter) const;
    bool isDirectory() const;
    int64_t length();
    int64_t lastModified();
    const std::wstring getPath() const;  
    std::wstring getName() const;

    static int hash_fnct(const File& k);
    static bool eq_test(const File& x, const File& y);

private:
    void _init();
    std::wstring m_abstractPathName;

    
    
    
};

struct FileKeyHash {
    int operator()(const File& k) const { return File::hash_fnct(k); }
};

struct FileKeyEq {
    bool operator()(const File& x, const File& y) const {
        return File::eq_test(x, y);
    }
};