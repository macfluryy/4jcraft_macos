#pragma once



class WstringLookup
{
private:
	UINT numIDs;
	std::unordered_map<std::wstring, UINT> str2int;
	std::vector<std::wstring> int2str;

public:
	WstringLookup();

	std::wstring lookup(UINT id);
	
	UINT lookup(std::wstring);

	VOID getTable(std::wstring **lookup, UINT *len);
};