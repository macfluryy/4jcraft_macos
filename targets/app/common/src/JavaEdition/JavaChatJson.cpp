#include "app/common/src/JavaEdition/JavaChatJson.h"
#include "app/common/src/JavaEdition/JavaProxyDebug.h"


#include <cstdint>
#include <vector>

#include "app/common/src/JavaEdition/JavaChatTranslations.h"

namespace {

bool cj_isJsonWs(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
bool cj_isDigit(int c)  { return c >= '0' && c <= '9'; }

constexpr wchar_t kSectionSign = L'§';

struct CjRgb { unsigned char r, g, b; };
constexpr CjRgb cj_kLegacyPalette[16] = {
    {0x00, 0x00, 0x00}, {0x00, 0x00, 0xAA}, {0x00, 0xAA, 0x00},
    {0x00, 0xAA, 0xAA}, {0xAA, 0x00, 0x00}, {0xAA, 0x00, 0xAA},
    {0xFF, 0xAA, 0x00}, {0xAA, 0xAA, 0xAA}, {0x55, 0x55, 0x55},
    {0x55, 0x55, 0xFF}, {0x55, 0xFF, 0x55}, {0x55, 0xFF, 0xFF},
    {0xFF, 0x55, 0x55}, {0xFF, 0x55, 0xFF}, {0xFF, 0xFF, 0x55},
    {0xFF, 0xFF, 0xFF},
};

int cj_hexVal(wchar_t c) {
    if (c >= L'0' && c <= L'9') return c - L'0';
    if (c >= L'a' && c <= L'f') return (c - L'a') + 10;
    if (c >= L'A' && c <= L'F') return (c - L'A') + 10;
    return -1;
}

wchar_t cj_nearestLegacyColor(int r, int g, int b) {
    int best = 0;
    long bestDist = -1;
    for (int i = 0; i < 16; ++i) {
        const long dr = r - cj_kLegacyPalette[i].r;
        const long dg = g - cj_kLegacyPalette[i].g;
        const long db = b - cj_kLegacyPalette[i].b;
        const long dist = dr * dr + dg * dg + db * db;
        if (bestDist < 0 || dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    return best < 10 ? static_cast<wchar_t>(L'0' + best)
                     : static_cast<wchar_t>(L'a' + (best - 10));
}

std::wstring cj_normalizeToLegacy(const std::wstring& in) {
    std::wstring out;
    out.reserve(in.size());
    wchar_t pending = 0;
    wchar_t active = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        const wchar_t c = in[i];
        if (c != kSectionSign) {
            if (pending && pending != active) {
                out.push_back(kSectionSign);
                out.push_back(pending);
                active = pending;
            }
            pending = 0;
            out.push_back(c);
            continue;
        }
        if (i + 1 >= in.size()) break;
        const wchar_t code = in[i + 1];
        if (cj_hexVal(code) >= 0) {
            pending = static_cast<wchar_t>(
                code >= L'A' && code <= L'F' ? code - L'A' + L'a' : code);
            ++i;
        } else if (code == L'x' || code == L'X') {
            int digits[6];
            int got = 0;
            size_t j = i + 2;
            while (got < 6 && j + 1 < in.size() && in[j] == kSectionSign &&
                   cj_hexVal(in[j + 1]) >= 0) {
                digits[got++] = cj_hexVal(in[j + 1]);
                j += 2;
            }
            if (got == 6) {
                pending = cj_nearestLegacyColor(digits[0] * 16 + digits[1],
                                                digits[2] * 16 + digits[3],
                                                digits[4] * 16 + digits[5]);
                i = j - 1;
            } else {
                ++i;
            }
        } else if (code == L'r' || code == L'R') {
            pending = L'f';
            ++i;
        } else {
            ++i;
        }
    }
    return out;
}


void cj_logFormattingCodes(const std::wstring& out) {
    const wchar_t kSection = L'§';
    for (size_t i = 0; i + 1 < out.size(); ++i) {
        if (out[i] != kSection) continue;
        const wchar_t code = out[i + 1];
        const bool isHexDigit = (code >= L'0' && code <= L'9') ||
                                (code >= L'a' && code <= L'f') ||
                                (code >= L'A' && code <= L'F');
        if (code == L'x' || code == L'X') {
            std::string hex;
            size_t j = i + 2;
            while (hex.size() < 6 && j + 1 < out.size() &&
                   out[j] == kSection) {
                const wchar_t h = out[j + 1];
                const bool hOk = (h >= L'0' && h <= L'9') ||
                                 (h >= L'a' && h <= L'f') ||
                                 (h >= L'A' && h <= L'F');
                if (!hOk) break;
                hex.push_back(static_cast<char>(h));
                j += 2;
            }
            JPROXY_LOGF(
                    "[JHEX] raw='\xc2\xa7x' +%zu hex pairs (#%s) parsed=RGB-HEX "
                    "lce=nearest-legacy-color\n",
                    hex.size(), hex.c_str());
            i = j - 1;
            continue;
        }
        const char codeA =
            (code >= 0x20 && code <= 0x7E) ? static_cast<char>(code) : '?';
        if (isHexDigit) {
            JPROXY_LOGF(
                    "[JHEX] raw='\xc2\xa7%c' parsed=legacy-color "
                    "lce='\xc2\xa7%c' fontEffect=color-set\n",
                    codeA, codeA);
        } else {
            JPROXY_LOGF(
                    "[JHEX] raw='\xc2\xa7%c' (U+%04X) parsed=format/reset "
                    "lce=%s\n",
                    codeA, static_cast<unsigned>(code) & 0xFFFFu,
                    (code == L'r' || code == L'R') ? "\xc2\xa7""f (reset->default)"
                                                   : "removed");
        }
        ++i;
    }
}

wchar_t cj_colorCode(const std::wstring& name) {
    struct M { const wchar_t* n; wchar_t c; };
    static const M kMap[] = {
        {L"black", L'0'},        {L"dark_blue", L'1'},   {L"dark_green", L'2'},
        {L"dark_aqua", L'3'},    {L"dark_red", L'4'},    {L"dark_purple", L'5'},
        {L"gold", L'6'},         {L"gray", L'7'},        {L"grey", L'7'},
        {L"dark_gray", L'8'},    {L"dark_grey", L'8'},   {L"blue", L'9'},
        {L"green", L'a'},        {L"aqua", L'b'},        {L"red", L'c'},
        {L"light_purple", L'd'}, {L"yellow", L'e'},      {L"white", L'f'},
        {L"reset", L'r'},
    };
    for (const M& e : kMap) {
        if (name == e.n) return e.c;
    }
    return 0;
}

wchar_t cj_formatCode(const std::wstring& key) {
    if (key == L"bold")          return L'l';
    if (key == L"italic")        return L'o';
    if (key == L"underlined")    return L'n';
    if (key == L"strikethrough") return L'm';
    if (key == L"obfuscated")    return L'k';
    return 0;
}

void cj_appendCodePoint(std::wstring& out, unsigned int cp) {
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

class ChatScanner {
public:
    explicit ChatScanner(const std::string& s)
        : m_p(reinterpret_cast<const unsigned char*>(s.data())),
          m_len(s.size()),
          m_pos(0) {}

    bool flatten(std::wstring& out) {
        skipWs();
        if (!parseValue(0, &out, std::wstring())) return false;
        skipWs();
        return eof();
    }

private:
    const unsigned char* m_p;
    size_t m_len;
    size_t m_pos;

    bool eof() const { return m_pos >= m_len; }
    int peek() const { return eof() ? -1 : m_p[m_pos]; }
    int get()  { return eof() ? -1 : m_p[m_pos++]; }
    void skipWs() { while (!eof() && cj_isJsonWs(m_p[m_pos])) ++m_pos; }

    bool parseValue(int depth, std::wstring* sink, const std::wstring& inherited) {
        if (depth > kMaxChatJsonDepth) return false;
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
            case '{': return parseObject(depth, sink, inherited);
            case '[': return parseArray(depth, sink, inherited);
            case 't': return matchLiteral("true");
            case 'f': return matchLiteral("false");
            case 'n': return matchLiteral("null");
            default:
                if (c == '-' || cj_isDigit(c)) return parseNumber();
                return false;
        }
    }

    bool parseObject(int depth, std::wstring* sink, const std::wstring& inherited) {
        get();
        skipWs();
        std::wstring own;
        std::wstring extras;
        bool sawText = false;
        bool sawTranslate = false;
        std::wstring translateRaw;
        std::vector<std::wstring> withArgs;
        std::wstring ownColor;
        std::wstring ownFmts;
        if (peek() == '}') { get(); return true; }
        while (true) {
            skipWs();
            if (peek() != '"') return false;
            std::wstring key;
            if (!parseString(key)) return false;
            skipWs();
            if (peek() != ':') return false;
            get();
            if (key == L"color") {
                std::wstring cname;
                if (!parseValue(depth + 1, &cname, inherited)) return false;
                wchar_t cc = cj_colorCode(cname);
                if (!cc && cname.size() == 7 && cname[0] == L'#') {
                    int v[6];
                    bool ok = true;
                    for (int k = 0; k < 6; ++k) {
                        v[k] = cj_hexVal(cname[1 + k]);
                        if (v[k] < 0) { ok = false; break; }
                    }
                    if (ok) {
                        cc = cj_nearestLegacyColor(v[0] * 16 + v[1],
                                                   v[2] * 16 + v[3],
                                                   v[4] * 16 + v[5]);
                    }
                }
                {
                    std::string nameA;
                    for (wchar_t wc : cname)
                        nameA.push_back(
                            (wc >= 0x20 && wc <= 0x7E) ? (char)wc : '?');
                    JPROXY_LOGF(
                            "[JHEX] color-key raw='%s' mapped=%s%c\n",
                            nameA.c_str(),
                            cc ? "\xc2\xa7" : "(none - dropped",
                            cc ? (char)cc : ')');
                }
                if (cc) {
                    ownColor.clear();
                    ownColor.push_back(kSectionSign);
                    ownColor.push_back(cc);
                }
            } else if (const wchar_t fc = cj_formatCode(key)) {
                skipWs();
                if (peek() == 't') {
                    if (!matchLiteral("true")) return false;
                    ownFmts.push_back(kSectionSign);
                    ownFmts.push_back(fc);
                } else {
                    if (!parseValue(depth + 1, nullptr, inherited)) return false;
                }
            } else if (key == L"text" && sink) {
                if (!parseValue(depth + 1, &own, inherited)) return false;
                sawText = true;
            } else if (key == L"translate" && sink) {
                std::wstring tmp;
                if (!parseValue(depth + 1, &tmp, inherited)) return false;
                translateRaw = tmp;
                sawTranslate = true;
            } else if (key == L"with" && sink) {
                if (!parseWithArray(depth + 1, withArgs,
                                    inherited + ownColor + ownFmts)) {
                    return false;
                }
            } else if (key == L"extra" && sink) {
                if (!parseValue(depth + 1, &extras,
                                inherited + ownColor + ownFmts)) {
                    return false;
                }
            } else {
                if (!parseValue(depth + 1, nullptr, inherited)) return false;
            }
            skipWs();
            const int c = get();
            if (c == ',') continue;
            if (c == '}') {
                if (sink) {
                    const std::wstring effective = inherited + ownColor + ownFmts;
                    std::wstring body;
                    if (sawText) {
                        body = own;
                    } else if (sawTranslate) {
                        body = applyTranslation(translateRaw, withArgs);
                    }
                    if (!body.empty()) {
                        if (!effective.empty()) {
                            sink->push_back(kSectionSign);
                            sink->push_back(L'r');
                            sink->append(effective);
                        }
                        sink->append(body);
                    }
                    sink->append(extras);
                }
                return true;
            }
            return false;
        }
    }

    bool parseWithArray(int depth, std::vector<std::wstring>& args,
                        const std::wstring& inherited) {
        skipWs();
        if (peek() != '[') {
            std::wstring one;
            if (!parseValue(depth, &one, inherited)) return false;
            args.push_back(std::move(one));
            return true;
        }
        get();
        skipWs();
        if (peek() == ']') { get(); return true; }
        while (true) {
            std::wstring elem;
            if (!parseValue(depth + 1, &elem, inherited)) return false;
            args.push_back(std::move(elem));
            skipWs();
            const int c = get();
            if (c == ',') { skipWs(); continue; }
            if (c == ']') return true;
            return false;
        }
    }

    static std::wstring applyTranslation(
        const std::wstring& keyW, const std::vector<std::wstring>& args) {
        std::string keyA;
        keyA.reserve(keyW.size());
        for (wchar_t c : keyW) keyA.push_back(static_cast<char>(c & 0x7F));
        const wchar_t* tmpl = JavaChatTranslations::lookup(keyA);
        if (tmpl == nullptr) {
            if (args.empty()) return keyW;
            std::wstring out = keyW;
            out.append(L": ");
            for (size_t i = 0; i < args.size(); ++i) {
                if (i) out.push_back(L' ');
                out.append(args[i]);
            }
            return out;
        }
        std::wstring out;
        for (const wchar_t* p = tmpl; *p; ++p) {
            if (*p == L'%' && p[1] >= L'0' && p[1] <= L'9') {
                size_t idx = static_cast<size_t>(p[1] - L'0');
                if (idx < args.size()) out.append(args[idx]);
                ++p;
            } else {
                out.push_back(*p);
            }
        }
        return out;
    }

    bool parseArray(int depth, std::wstring* sink, const std::wstring& inherited) {
        get();
        skipWs();
        if (peek() == ']') { get(); return true; }
        while (true) {
            if (!parseValue(depth + 1, sink, inherited)) return false;
            skipWs();
            const int c = get();
            if (c == ',') { skipWs(); continue; }
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
            case '"': out.push_back(L'"');  return true;
            case '\\': out.push_back(L'\\'); return true;
            case '/':  out.push_back(L'/');  return true;
            case 'n':  out.push_back(L'\n'); return true;
            case 'r':  out.push_back(L'\r'); return true;
            case 't':  out.push_back(L'\t'); return true;
            case 'b':  out.push_back(L'\b'); return true;
            case 'f':  out.push_back(L'\f'); return true;
            case 'u':  return parseUnicodeEscape(out);
            default:   return false;
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
                        cj_appendCodePoint(out, combined);
                        return true;
                    }
                }
            }
            m_pos = save;
        }
        cj_appendCodePoint(out, cp);
        return true;
    }

    bool readHex4(unsigned int& cp) {
        cp = 0;
        for (int i = 0; i < 4; ++i) {
            const int c = get();
            unsigned int digit;
            if      (c >= '0' && c <= '9') digit = static_cast<unsigned int>(c - '0');
            else if (c >= 'a' && c <= 'f') digit = static_cast<unsigned int>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') digit = static_cast<unsigned int>(c - 'A' + 10);
            else return false;
            cp = (cp << 4) | digit;
        }
        return true;
    }

    void decodeUtf8(unsigned char b0, std::wstring& out) {
        if (b0 < 0x80) { cj_appendCodePoint(out, b0); return; }
        int extra; unsigned int cp;
        if      ((b0 & 0xE0) == 0xC0) { extra = 1; cp = b0 & 0x1F; }
        else if ((b0 & 0xF0) == 0xE0) { extra = 2; cp = b0 & 0x0F; }
        else if ((b0 & 0xF8) == 0xF0) { extra = 3; cp = b0 & 0x07; }
        else { cj_appendCodePoint(out, b0); return; }
        for (int k = 0; k < extra; ++k) {
            const int n = peek();
            if (n < 0 || (n & 0xC0) != 0x80) { cj_appendCodePoint(out, b0); return; }
            get();
            cp = (cp << 6) | static_cast<unsigned int>(n & 0x3F);
        }
        cj_appendCodePoint(out, cp);
    }

    bool parseNumber() {
        if (peek() == '-') get();
        if      (peek() == '0')          get();
        else if (cj_isDigit(peek()))     { while (cj_isDigit(peek())) get(); }
        else                             return false;
        if (peek() == '.') {
            get();
            if (!cj_isDigit(peek())) return false;
            while (cj_isDigit(peek())) get();
        }
        if (peek() == 'e' || peek() == 'E') {
            get();
            if (peek() == '+' || peek() == '-') get();
            if (!cj_isDigit(peek())) return false;
            while (cj_isDigit(peek())) get();
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

std::wstring flattenChatComponent(const std::string& json) {
    if (json.empty() || json.size() > kMaxChatJsonBytes) return {};
    ChatScanner scanner(json);
    std::wstring out;
    if (!scanner.flatten(out)) return {};
    cj_logFormattingCodes(out);
    std::wstring legacy = cj_normalizeToLegacy(out);
    if (legacy.size() != out.size()) {
        JPROXY_LOGF( "[JHEX] normalized len %zu -> %zu\n", out.size(),
                legacy.size());
    }
    return legacy;
}

std::wstring normalizeLegacyFormatting(const std::wstring& text) {
    return cj_normalizeToLegacy(text);
}