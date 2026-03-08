#pragma once
//using namespace stdtd;

std::wstring toLower(const std::wstring& a);
std::wstring trimString(const std::wstring& a);
std::wstring replaceAll(const std::wstring& in, const std::wstring& replace, const std::wstring& with);

bool equalsIgnoreCase(const std::wstring& a, const std::wstring& b);
// 4J-PB - for use in the ::toString 
template <class T> std::wstring _toString(T t)
{
	std::wostringstream oss;
	oss << std::dec << t;
	return oss.str();
}
template <class T> T _fromString(const std::wstring& s)
{
    std::wistringstream stream (s);
    T t;
    stream >> t;
    return t;
}
template <class T> T _fromHEXString(const std::wstring& s)
{
    std::wistringstream stream (s);
    T t;
    stream >> std::hex >> t;
    return t;
}

std::wstring convStringToWstring(const std::string& converting);
const char *wstringtofilename(const std::wstring& name);
std::wstring filenametowstring(const char *name);

std::vector<std::wstring> &stringSplit(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems);
std::vector<std::wstring> stringSplit(const std::wstring &s, wchar_t delim);

void stripWhitespaceForHtml(std::wstring &string, bool bRemoveNewline=true);
std::wstring escapeXML(const std::wstring &in);
std::wstring parseXMLSpecials(const std::wstring &in);
