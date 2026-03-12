#include "../Platform/stdafx.h"

std::wstring toLower(const std::wstring& a)
{
	std::wstring out = std::wstring(a);
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
}

std::wstring trimString(const std::wstring& a)
{
	std::wstring b;
	int start = (int)a.find_first_not_of(L" \t\n\r");
	int end = (int)a.find_last_not_of(L" \t\n\r");
	if( start == std::wstring::npos ) start = 0;
	if( end == std::wstring::npos ) end = (int)a.size()-1;
	b = a.substr(start,(end-start)+1);
	return b;
}

std::wstring replaceAll(const std::wstring& in, const std::wstring& replace, const std::wstring& with)
{
	std::wstring out = in;
	size_t pos = 0;
	while( ( pos = out.find(replace, pos) ) != std::wstring::npos )
	{
		out.replace( pos, replace.length(), with );
		pos++;
	}
	return out;
}

bool equalsIgnoreCase(const std::wstring& a, const std::wstring& b)
{
	bool out;
	std::wstring c = toLower(a);
	std::wstring d = toLower(b);
	out = c.compare(d) == 0;
	return out;
}

std::wstring convStringToWstring(const std::string& converting)
{
	std::wstring converted(converting.length(), L' ');
	copy(converting.begin(), converting.end(), converted.begin());
	return converted;
}

std::u16string convWstringToU16string(const std::wstring& converting) {
    std::u16string out;
    out.reserve(converting.size());

    if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
        // wchar_t is UTF-16: direct copy
        out.assign(converting.begin(), converting.end());
    } else {
        // wchar_t is UTF-32: encode to UTF-16
        for (wchar_t wc : converting) {
            uint32_t cp = static_cast<uint32_t>(wc);

            if (cp <= 0xFFFF) {
                // Avoid producing UTF-16 surrogate code points directly
                if (cp >= 0xD800 && cp <= 0xDFFF) {
                    out.push_back(u'\uFFFD'); // replacement char
                } else {
                    out.push_back(static_cast<char16_t>(cp));
                }
            } else if (cp <= 0x10FFFF) {
                cp -= 0x10000;
                out.push_back(static_cast<char16_t>(0xD800 + (cp >> 10)));
                out.push_back(static_cast<char16_t>(0xDC00 + (cp & 0x3FF)));
            } else {
                out.push_back(u'\uFFFD'); // invalid code point
            }
        }
    }

    return out;
}

// Convert for filename std::wstrings to a straight character pointer for Xbox APIs. The returned string is only valid until
// this function is called again, and it isn't thread-safe etc. as I'm just storing the returned name in a local static
// to save having to clear it up everywhere this is used.
const char *wstringtofilename(const std::wstring& name)
{
	static char buf[256];
	assert(name.length()<256);
	for(unsigned int i = 0; i < name.length(); i++ )
	{
		wchar_t c = name[i];
#if defined __PS3__ || defined __ORBIS__ || defined __linux__
		if(c=='\\') c='/';
#else
		if(c=='/') c='\\';
#endif
		assert(c<128);	// Will we have to do any conversion of non-ASCII characters in filenames?
		buf[i] = (char)c;
	}
	buf[name.length()] = 0;
	return buf;
}

std::wstring filenametowstring(const char *name)
{
	return convStringToWstring(name);
}

std::vector<std::wstring> &stringSplit(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems)
{
    std::wstringstream ss(s);
    std::wstring item;
    while(std::getline(ss, item, delim))
	{
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::wstring> stringSplit(const std::wstring &s, wchar_t delim)
{
    std::vector<std::wstring> elems;
    return stringSplit(s, delim, elems);
}

bool BothAreSpaces(wchar_t lhs, wchar_t rhs) { return (lhs == rhs) && (lhs == L' '); }

void stripWhitespaceForHtml(std::wstring &string, bool bRemoveNewline)
{
	// Strip newline chars
	if(bRemoveNewline)
	{	
		string.erase(std::remove(string.begin(), string.end(), '\n'), string.end());
		string.erase(std::remove(string.begin(), string.end(), '\r'), string.end());
	}

	string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());

	// Strip duplicate spaces
	string.erase(std::unique(string.begin(), string.end(), BothAreSpaces), string.end()); 

	string = trimString(string);
}

std::wstring escapeXML(const std::wstring &in)
{
	std::wstring out = in;
	out = replaceAll(out, L"&", L"&amp;");
	//out = replaceAll(out, L"\"", L"&quot;");
	//out = replaceAll(out, L"'", L"&apos;");
	out = replaceAll(out, L"<", L"&lt;");
	out = replaceAll(out, L">", L"&gt;");
	return out;
}

std::wstring parseXMLSpecials(const std::wstring &in)
{
	std::wstring out = in;
	out = replaceAll(out, L"&amp;", L"&");
	//out = replaceAll(out, L"\"", L"&quot;");
	//out = replaceAll(out, L"'", L"&apos;");
	out = replaceAll(out, L"&lt;", L"<");
	out = replaceAll(out, L"&gt;", L">");
	return out;
}
