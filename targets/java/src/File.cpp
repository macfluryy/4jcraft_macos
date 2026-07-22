#include "java/File.h"

#include <stdio.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "util/StringHelpers.h"  
#include "platform/PlatformServices.h"
#include "java/FileFilter.h"

const wchar_t File::pathSeparator = L'/';

const std::wstring File::pathRoot =
    L"";  

namespace {
namespace fs = std::filesystem;

fs::path ToFilesystemPath(const std::wstring& path) {
    return fs::path(path);
}

std::wstring ToFilename(const fs::path& path) {
    const std::string filename = path.filename().string();
    return filenametowstring(filename.c_str());
}

int64_t ToEpochMilliseconds(const fs::file_time_type& fileTime) {
    using namespace std::chrono;

    const auto systemTime = time_point_cast<milliseconds>(
        fileTime - fs::file_time_type::clock::now() + system_clock::now());
    return static_cast<int64_t>(systemTime.time_since_epoch().count());
}
}  



File::File(const File& parent, const std::wstring& child) {
    m_abstractPathName = parent.getPath() + pathSeparator + child;
}




File::File(const std::wstring& pathname) {
    if (pathname.empty()) {
        m_abstractPathName = L"";
        return;
    }

    std::wstring fixedPath = pathname;
    for (size_t i = 0; i < fixedPath.length(); ++i) {
        if (fixedPath[i] == L'\\') fixedPath[i] = L'/';
    }
    size_t dpos;
    while ((dpos = fixedPath.find(L"//")) != std::wstring::npos)
        fixedPath.erase(dpos, 1);
    if (fixedPath.find(L"GAME:/") == 0) fixedPath = fixedPath.substr(6);
    m_abstractPathName = fixedPath;

#if defined(__linux__) || defined(__APPLE__)
    std::string request = std::filesystem::path(m_abstractPathName).string();
    while (!request.empty() && request[0] == '/') request.erase(0, 1);
    if (request.find("res/") == 0) request.erase(0, 4);

    std::string exeDir = PlatformFileIO.getBasePath().string();
    std::string fileName = request;
    size_t lastSlash = fileName.find_last_of('/');
    if (lastSlash != std::string::npos)
        fileName = fileName.substr(lastSlash + 1);

    const char* bases[] = {"/",
                           "/Common/res/TitleUpdate/res/",
                           "/Common/Media/",
                           "/Common/res/",
                           "/Common/",
                           "resources/"};

    for (const char* base : bases) {
        std::string tryFull = exeDir + base + request;
        std::string tryFile = exeDir + base + fileName;
        if (PlatformFileIO.exists(tryFull)) {
            m_abstractPathName = convStringToWstring(tryFull);
            return;
        }
        if (PlatformFileIO.exists(tryFile)) {
            m_abstractPathName = convStringToWstring(tryFile);
            return;
        }
    }
#endif

#ifdef _WINDOWS64
    std::string path = std::filesystem::path(m_abstractPathName).string();
    std::string finalPath = StorageManager.GetMountedPath(path.c_str());
    if (finalPath.size() == 0) finalPath = path;
    m_abstractPathName = convStringToWstring(finalPath);
#endif
    


















}

File::File(const std::wstring& parent,
           const std::wstring& child)  
{
    m_abstractPathName =
        pathRoot + pathSeparator + parent + pathSeparator + child;
    
}
























bool File::_delete() {
    std::error_code error;
    const bool result = fs::remove(ToFilesystemPath(getPath()), error);
    if (!result || error) {
#ifndef _CONTENT_PACKAGE
        printf("File::_delete - Error code %d (%#0.8X)\n", error.value(),
               error.value());
#endif
        return false;
    }
    return true;
}




bool File::mkdir() const {
    std::error_code error;
    return fs::create_directory(ToFilesystemPath(getPath()), error);
}




















bool File::mkdirs() const {
    std::error_code error;
    const fs::path path = ToFilesystemPath(getPath());

    if (fs::exists(path, error)) {
        return fs::is_directory(path, error);
    }

    if (error) {
        return false;
    }

    return fs::create_directories(path, error);
}












bool File::exists() const {
    
    
    std::error_code error;
    return fs::exists(ToFilesystemPath(getPath()), error);
}







bool File::isFile() const { return exists() && !isDirectory(); }










bool File::renameTo(File dest) {
    std::error_code error;
    fs::rename(ToFilesystemPath(getPath()), ToFilesystemPath(dest.getPath()),
               error);
    if (error) {
        perror("File::renameTo - Error renaming file");
        return false;
    }
    return true;
}





















std::vector<File*>* File::listFiles() const {
    std::vector<File*>* vOutput = new std::vector<File*>();

    
    if (!isDirectory()) return vOutput;

    std::error_code error;
    for (fs::directory_iterator it(ToFilesystemPath(getPath()), error);
         !error && it != fs::directory_iterator(); it.increment(error)) {
        vOutput->push_back(new File(*this, ToFilename(it->path())));
    }
    return vOutput;
}













std::vector<File*>* File::listFiles(FileFilter* filter) const {
    
    if (!isDirectory()) return nullptr;

    std::vector<File*>* vOutput = new std::vector<File*>();

    std::error_code error;
    for (fs::directory_iterator it(ToFilesystemPath(getPath()), error);
         !error && it != fs::directory_iterator(); it.increment(error)) {
        File thisFile = File(*this, ToFilename(it->path()));
        if (filter->accept(&thisFile)) {
            vOutput->push_back(new File(thisFile));
        }
    }
    return vOutput;
}





bool File::isDirectory() const {
    std::error_code error;
    return fs::is_directory(ToFilesystemPath(getPath()), error);
}





int64_t File::length() {
    std::error_code error;
    const fs::path path = ToFilesystemPath(getPath());

    if (fs::is_regular_file(path, error)) {
        const auto size = fs::file_size(path, error);
        if (!error) {
            return static_cast<int64_t>(size);
        }
    }

    return 0;
}





int64_t File::lastModified() {
    std::error_code error;
    const fs::path path = ToFilesystemPath(getPath());

    if (fs::is_regular_file(path, error)) {
        const fs::file_time_type lastWriteTime =
            fs::last_write_time(path, error);
        if (!error) {
            return ToEpochMilliseconds(lastWriteTime);
        }
    }

    return 0l;
}

const std::wstring File::getPath() const {
    









    return m_abstractPathName;
}

std::wstring File::getName() const {
    unsigned int sep =
        (unsigned int)(m_abstractPathName.find_last_of(this->pathSeparator));
    return m_abstractPathName.substr(sep + 1, m_abstractPathName.length());
}

bool File::eq_test(const File& x, const File& y) {
    return x.getPath().compare(y.getPath()) == 0;
}


int File::hash_fnct(const File& k) {
    int hashCode = 0;

    
    

    wchar_t* ref = (wchar_t*)k.m_abstractPathName.c_str();

    for (unsigned int i = 0; i < k.m_abstractPathName.length(); i++) {
        hashCode += ((hashCode * 33) + ref[i]) % 149;
    }

    return (int)hashCode;
}