#include "JavaAddressParser.h"

#include <cwctype>


namespace {

constexpr uint16_t kDefaultPort = 25565;

std::wstring trim(const std::wstring& s) {
    const wchar_t* ws = L" \t\n\r\f\v";
    const auto start = s.find_first_not_of(ws);
    if (start == std::wstring::npos) return std::wstring();
    const auto end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

uint16_t parsePort(const std::wstring& s) {
    size_t i = 0;
    while (i < s.size() && std::iswspace(static_cast<wint_t>(s[i]))) ++i;

    bool negative = false;
    if (i < s.size() && (s[i] == L'+' || s[i] == L'-')) {
        negative = (s[i] == L'-');
        ++i;
    }

    bool sawDigit = false;
    long long value = 0;
    while (i < s.size() && s[i] >= L'0' && s[i] <= L'9') {
        sawDigit = true;
        value = value * 10 + (s[i] - L'0');
        if (value > 1000000) value = 1000000;
        ++i;
    }

    if (!sawDigit) return kDefaultPort;
    if (negative) value = -value;
    if (value <= 0 || value >= 65536) return kDefaultPort;
    return static_cast<uint16_t>(value);
}
std::string toAsciiHost(const std::wstring& whost) {
    std::string host;
    host.reserve(whost.size());
    for (wchar_t wc : whost) {
        if (wc < 128) host.push_back(static_cast<char>(wc));
    }
    return host;
}

}

ParsedAddress parseAddress(const std::wstring& input) {
    const std::wstring ip = trim(input);

    std::wstring whost;
    uint16_t port = kDefaultPort;

    if (!ip.empty() && ip[0] == L'[') {
        const auto close = ip.find(L']');
        if (close != std::wstring::npos) {
            whost = ip.substr(1, close - 1);
            const std::wstring rest = trim(ip.substr(close + 1));
            if (!rest.empty() && rest[0] == L':') {
                port = parsePort(rest.substr(1));
            }
        } else {
            whost = ip;
        }
    } else {
        const auto colon = ip.find(L':');
        if (colon != std::wstring::npos) {
            whost = ip.substr(0, colon);
            port = parsePort(ip.substr(colon + 1));
        } else {
            whost = ip;
        }
    }

    ParsedAddress result;
    result.host = toAsciiHost(whost);
    result.port = port;
    result.hostEmpty = result.host.empty();
    return result;
}