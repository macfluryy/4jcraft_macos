#include "app/common/src/JavaEdition/JavaServerListStore.h"

#include <cstdint>
#include <map>
#include <string>

#include "java/InputOutputStream/BufferedReader.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/InputOutputStream/FileInputStream.h"
#include "java/InputOutputStream/FileOutputStream.h"
#include "java/InputOutputStream/InputStreamReader.h"
#include "util/StringHelpers.h"

namespace {

constexpr int kMaxEntries = 10000;
constexpr size_t kMaxLines = 100000;
constexpr size_t kMaxLineLength = 64u * 1024;
const std::wstring kKeyPrefix = L"jsrv.";

struct PartialEntry {
    std::wstring name;
    std::string host;
    uint16_t port = 25565;
    bool hasHost = false;
};
bool isAllDigits(const std::wstring& s) {
    if (s.empty()) return false;
    for (wchar_t wc : s) {
        if (wc < L'0' || wc > L'9') return false;
    }
    return true;
}

int parseIndex(const std::wstring& s) {
    if (!isAllDigits(s)) return -1;
    long value = 0;
    for (wchar_t wc : s) {
        value = value * 10 + (wc - L'0');
        if (value >= kMaxEntries) return -1;
    }
    return static_cast<int>(value);
}

uint16_t parsePortOrDefault(const std::wstring& s) {
    if (!isAllDigits(s)) return 25565;
    long value = 0;
    for (wchar_t wc : s) {
        value = value * 10 + (wc - L'0');
        if (value > 65535) return 25565;
    }
    if (value < 1) return 25565;
    return static_cast<uint16_t>(value);
}

std::wstring widenAscii(const std::string& s) {
    std::wstring out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        out.push_back(static_cast<wchar_t>(c));
    }
    return out;
}

std::string narrowWide(const std::wstring& s) {
    std::string out;
    out.reserve(s.size());
    for (wchar_t wc : s) {
        out.push_back(static_cast<char>(static_cast<unsigned int>(wc) & 0xff));
    }
    return out;
}

}

JavaServerListStore::JavaServerListStore(const File& workingDirectory)
    : m_file(File(workingDirectory, L"javaservers.txt")) {}

JavaServerListStore::LoadResult JavaServerListStore::load() {
    LoadResult result;
    if (!m_file.exists()) {
        return result;
    }

    FileInputStream fileInput(m_file);
    InputStreamReader streamReader(&fileInput);
    BufferedReader reader(&streamReader);

    std::map<int, PartialEntry> byIndex;
    std::wstring line;
    size_t lineCount = 0;
    while ((line = reader.readLine()) != L"") {
        if (++lineCount > kMaxLines) break;
        if (line.size() > kMaxLineLength) {
            result.hadCorruptLines = true;
            continue;
        }

        if (line.find(L'\0') != std::wstring::npos) {
            std::wstring filtered;
            filtered.reserve(line.size());
            for (wchar_t wc : line) {
                if (wc != L'\0') filtered.push_back(wc);
            }
            line = filtered;
        }

        const size_t colon = line.find(L':');
        if (colon == std::wstring::npos) {
            result.hadCorruptLines = true;
            continue;
        }

        const std::wstring key = line.substr(0, colon);
        const std::wstring value = line.substr(colon + 1);
        if (key.compare(0, kKeyPrefix.size(), kKeyPrefix) != 0) {
            continue;
        }

        const std::wstring rest = key.substr(kKeyPrefix.size());
        const size_t dot = rest.find(L'.');
        if (dot == std::wstring::npos) {
            continue;
        }

        const int index = parseIndex(rest.substr(0, dot));
        if (index < 0) {
            continue;
        }
        const std::wstring field = rest.substr(dot + 1);

        if (field == L"name") {
            byIndex[index].name = value;
        } else if (field == L"host") {
            byIndex[index].host = narrowWide(value);
            byIndex[index].hasHost = true;
        } else if (field == L"port") {
            byIndex[index].port = parsePortOrDefault(value);
        }
    }

    reader.close();

    for (const auto& [index, partial] : byIndex) {
        (void)index;
        if (!partial.hasHost || partial.host.empty()) {
            result.hadCorruptLines = true;
            continue;
        }
        JavaServerEntry entry;
        entry.name = partial.name;
        entry.host = partial.host;
        entry.port = partial.port;
        result.list.add(entry);
    }

    return result;
}

bool JavaServerListStore::save(const JavaServerList& list) {
    FileOutputStream fileOutput(m_file);
    DataOutputStream dataOutput(&fileOutput);

    auto writeAscii = [&dataOutput](const std::wstring& s) {
        for (wchar_t wc : s) {
            unsigned int b = static_cast<unsigned int>(wc) & 0xff;
            dataOutput.write(b);
        }
    };

    writeAscii(L"jsrv.count:" + toWString<size_t>(list.size()) + L"\n");
    for (size_t i = 0; i < list.size(); ++i) {
        const JavaServerEntry& e = list.at(i);
        const std::wstring idx = toWString<size_t>(i);
        writeAscii(kKeyPrefix + idx + L".name:" + e.name + L"\n");
        writeAscii(kKeyPrefix + idx + L".host:" + widenAscii(e.host) + L"\n");
        writeAscii(kKeyPrefix + idx + L".port:" +
                   toWString<int>(static_cast<int>(e.port)) + L"\n");
    }

    dataOutput.close();
    return true;
}