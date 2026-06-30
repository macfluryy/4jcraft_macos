#include "StatusJsonExtractor.h"

#include <climits>
#include <cstdint>

namespace {

bool isJsonWs(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
bool isDigit(int c) { return c >= '0' && c <= '9'; }
void appendCodePoint(std::wstring& out, unsigned int cp) {
    if constexpr (sizeof(wchar_t) >= 4) {
        out.push_back(static_cast<wchar_t>(cp));
    } else {
        if (cp <= 0xFFFF) {
            out.push_back(static_cast<wchar_t>(cp));
        } else {
            cp -= 0x10000;
            out.push_back(static_cast<wchar_t>(0xD800 + (cp >> 10)));
            out.push_back(static_cast<wchar_t>(0xDC00 + (cp & 0x3FF)));
        }
    }
}

class StatusScanner {
public:
    StatusScanner(const std::string& s)
        : m_p(reinterpret_cast<const unsigned char*>(s.data())),
          m_len(s.size()),
          m_pos(0) {}
    bool parse(StatusInfo& info) {
        skipWs();
        if (peek() != '{') return false;

        bool foundDescription = false;
        bool foundPlayers = false;
        if (!parseTopObject(0, info, foundDescription, foundPlayers)) return false;

        skipWs();
        if (!eof()) return false;

        info.valid = foundDescription || foundPlayers;
        return true;
    }

private:
    const unsigned char* m_p;
    size_t m_len;
    size_t m_pos;

    bool eof() const { return m_pos >= m_len; }
    int peek() const { return eof() ? -1 : m_p[m_pos]; }
    int get() { return eof() ? -1 : m_p[m_pos++]; }
    void skipWs() {
        while (!eof() && isJsonWs(m_p[m_pos])) ++m_pos;
    }

    bool parseTopObject(int depth, StatusInfo& info, bool& foundDescription,
                        bool& foundPlayers) {
        get();
        skipWs();
        if (peek() == '}') {
            get();
            return true;
        }
        while (true) {
            skipWs();
            if (peek() != '"') return false;
            std::wstring key;
            if (!parseString(key)) return false;
            skipWs();
            if (peek() != ':') return false;
            get();

            if (key == L"description") {
                if (!parseValue(depth + 1, &info.motd)) return false;
                foundDescription = true;
            } else if (key == L"players") {
                if (!parsePlayersValue(depth + 1, info)) return false;
                foundPlayers = true;
            } else if (key == L"version") {
                if (!parseVersionValue(depth + 1, info)) return false;
            } else {
                if (!parseValue(depth + 1, nullptr)) return false;
            }

            skipWs();
            const int c = get();
            if (c == ',') continue;
            if (c == '}') return true;
            return false;
        }
    }
    bool parsePlayersValue(int depth, StatusInfo& info) {
        if (depth > kMaxStatusJsonDepth) return false;
        skipWs();
        if (peek() != '{') {
            return parseValue(depth, nullptr);
        }
        get();
        skipWs();
        if (peek() == '}') {
            get();
            return true;
        }
        while (true) {
            skipWs();
            if (peek() != '"') return false;
            std::wstring key;
            if (!parseString(key)) return false;
            skipWs();
            if (peek() != ':') return false;
            get();

            if (key == L"online") {
                if (!parseValueCapturingInt(depth + 1, info.online)) return false;
            } else if (key == L"max") {
                if (!parseValueCapturingInt(depth + 1, info.max)) return false;
            } else {
                if (!parseValue(depth + 1, nullptr)) return false;
            }

            skipWs();
            const int c = get();
            if (c == ',') continue;
            if (c == '}') return true;
            return false;
        }
    }
    bool parseVersionValue(int depth, StatusInfo& info) {
        if (depth > kMaxStatusJsonDepth) return false;
        skipWs();
        if (peek() != '{') {
            return parseValue(depth, nullptr);
        }
        get();
        skipWs();
        if (peek() == '}') {
            get();
            return true;
        }
        while (true) {
            skipWs();
            if (peek() != '"') return false;
            std::wstring key;
            if (!parseString(key)) return false;
            skipWs();
            if (peek() != ':') return false;
            get();

            if (key == L"name") {
                if (!parseValue(depth + 1, &info.version)) return false;
            } else {
                if (!parseValue(depth + 1, nullptr)) return false;
            }

            skipWs();
            const int c = get();
            if (c == ',') continue;
            if (c == '}') return true;
            return false;
        }
    }
    bool parseValue(int depth, std::wstring* sink) {
        if (depth > kMaxStatusJsonDepth) return false;
        skipWs();
        const int c = peek();
        if (c < 0) return false;
        switch (c) {
            case '"': {
                std::wstring tmp;
                if (!parseString(tmp)) return false;
                if (sink) sink->append(tmp);
                return true;
            }
            case '{':
                return parseComponentObject(depth, sink);
            case '[':
                return parseArray(depth, sink);
            case 't':
                return matchLiteral("true");
            case 'f':
                return matchLiteral("false");
            case 'n':
                return matchLiteral("null");
            default:
                if (c == '-' || isDigit(c)) return parseNumber(nullptr, nullptr);
                return false;
        }
    }
    bool parseValueCapturingInt(int depth, int& out) {
        if (depth > kMaxStatusJsonDepth) return false;
        skipWs();
        const int c = peek();
        if (c == '-' || isDigit(c)) {
            long long value = 0;
            bool isInteger = false;
            if (!parseNumber(&value, &isInteger)) return false;
            if (isInteger && value >= INT_MIN && value <= INT_MAX) {
                out = static_cast<int>(value);
            }
            return true;
        }
        return parseValue(depth, nullptr);
    }
    bool parseComponentObject(int depth, std::wstring* sink) {
        get();
        skipWs();
        if (peek() == '}') {
            get();
            return true;
        }
        while (true) {
            skipWs();
            if (peek() != '"') return false;
            std::wstring key;
            if (!parseString(key)) return false;
            skipWs();
            if (peek() != ':') return false;
            get();

            std::wstring* valueSink = nullptr;
            if (sink && (key == L"text" || key == L"extra")) valueSink = sink;
            if (!parseValue(depth + 1, valueSink)) return false;

            skipWs();
            const int c = get();
            if (c == ',') continue;
            if (c == '}') return true;
            return false;
        }
    }
    bool parseArray(int depth, std::wstring* sink) {
        get();
        skipWs();
        if (peek() == ']') {
            get();
            return true;
        }
        while (true) {
            if (!parseValue(depth + 1, sink)) return false;
            skipWs();
            const int c = get();
            if (c == ',') {
                skipWs();
                continue;
            }
            if (c == ']') return true;
            return false;
        }
    }
    bool parseString(std::wstring& out) {
        if (get() != '"') return false;
        while (true) {
            const int c = get();
            if (c < 0) return false;
            if (c == '"') return true;
            if (c == '\\') {
                if (!parseEscape(out)) return false;
            } else {
                decodeUtf8(static_cast<unsigned char>(c), out);
            }
        }
    }
    bool parseEscape(std::wstring& out) {
        const int e = get();
        if (e < 0) return false;
        switch (e) {
            case '"': out.push_back(L'"'); return true;
            case '\\': out.push_back(L'\\'); return true;
            case '/': out.push_back(L'/'); return true;
            case 'n': out.push_back(L'\n'); return true;
            case 'r': out.push_back(L'\r'); return true;
            case 't': out.push_back(L'\t'); return true;
            case 'b': out.push_back(L'\b'); return true;
            case 'f': out.push_back(L'\f'); return true;
            case 'u': return parseUnicodeEscape(out);
            default: return false;
        }
    }
    bool parseUnicodeEscape(std::wstring& out) {
        unsigned int cp = 0;
        if (!readHex4(cp)) return false;

        if (cp >= 0xD800 && cp <= 0xDBFF) {
            const size_t save = m_pos;
            if (peek() == '\\') {
                get();
                if (get() == 'u') {
                    unsigned int lo = 0;
                    if (readHex4(lo) && lo >= 0xDC00 && lo <= 0xDFFF) {
                        const unsigned int combined =
                            0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                        appendCodePoint(out, combined);
                        return true;
                    }
                }
            }
            m_pos = save;
        }
        appendCodePoint(out, cp);
        return true;
    }
    bool readHex4(unsigned int& cp) {
        cp = 0;
        for (int i = 0; i < 4; ++i) {
            const int c = get();
            unsigned int digit;
            if (c >= '0' && c <= '9') {
                digit = static_cast<unsigned int>(c - '0');
            } else if (c >= 'a' && c <= 'f') {
                digit = static_cast<unsigned int>(c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                digit = static_cast<unsigned int>(c - 'A' + 10);
            } else {
                return false;
            }
            cp = (cp << 4) | digit;
        }
        return true;
    }
    void decodeUtf8(unsigned char b0, std::wstring& out) {
        if (b0 < 0x80) {
            appendCodePoint(out, b0);
            return;
        }
        int extra;
        unsigned int cp;
        if ((b0 & 0xE0) == 0xC0) {
            extra = 1;
            cp = b0 & 0x1F;
        } else if ((b0 & 0xF0) == 0xE0) {
            extra = 2;
            cp = b0 & 0x0F;
        } else if ((b0 & 0xF8) == 0xF0) {
            extra = 3;
            cp = b0 & 0x07;
        } else {
            appendCodePoint(out, b0);
            return;
        }
        for (int k = 0; k < extra; ++k) {
            const int n = peek();
            if (n < 0 || (n & 0xC0) != 0x80) {
                appendCodePoint(out, b0);
                return;
            }
            get();
            cp = (cp << 6) | static_cast<unsigned int>(n & 0x3F);
        }
        appendCodePoint(out, cp);
    }
    bool parseNumber(long long* outValue, bool* outIsInt) {
        if (outIsInt) *outIsInt = false;
        const size_t start = m_pos;
        bool isInteger = true;

        if (peek() == '-') get();

        if (peek() == '0') {
            get();
        } else if (isDigit(peek())) {
            while (isDigit(peek())) get();
        } else {
            return false;
        }

        if (peek() == '.') {
            isInteger = false;
            get();
            if (!isDigit(peek())) return false;
            while (isDigit(peek())) get();
        }
        if (peek() == 'e' || peek() == 'E') {
            isInteger = false;
            get();
            if (peek() == '+' || peek() == '-') get();
            if (!isDigit(peek())) return false;
            while (isDigit(peek())) get();
        }

        if (isInteger && outValue) {
            size_t i = start;
            bool negative = false;
            if (m_p[i] == '-') {
                negative = true;
                ++i;
            }
            long long value = 0;
            bool overflow = false;
            for (; i < m_pos; ++i) {
                const int d = m_p[i] - '0';
                if (value > (LLONG_MAX - 9) / 10) {
                    overflow = true;
                    break;
                }
                value = value * 10 + d;
            }
            if (!overflow) {
                if (outValue) *outValue = negative ? -value : value;
                if (outIsInt) *outIsInt = true;
            }
        }
        return true;
    }

    bool matchLiteral(const char* lit) {
        for (const char* p = lit; *p != '\0'; ++p) {
            if (get() != static_cast<unsigned char>(*p)) return false;
        }
        return true;
    }
};

}

StatusInfo extractStatus(const std::string& json) {
    StatusInfo info;
    if (json.empty() || json.size() > kMaxStatusJsonBytes) return info;

    StatusScanner scanner(json);
    StatusInfo parsed;
    if (scanner.parse(parsed)) return parsed;
    return info;
}