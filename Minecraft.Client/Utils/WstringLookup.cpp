
#include "../Build/stdafx.h"

#include "WstringLookup.h"

WstringLookup::WstringLookup()
{
	numIDs = 0;
}

std::wstring WstringLookup::lookup(UINT id)
{
	// TODO
	//if (id > currentMaxID)
	//	throw error

	return int2str.at(id);
}
	
UINT WstringLookup::lookup(std::wstring str)
{
	if (str2int.find(str) == str2int.end())
	{
		std::pair<std::wstring,UINT> p = 
			std::pair<std::wstring,UINT>(str, numIDs);

		str2int.insert( p );
		int2str.push_back( str );

		return numIDs++;
	}
	else
	{
		return str2int.at(str);
	}
}

VOID WstringLookup::getTable(std::wstring **lookup, UINT *len)
{
	// Outputs
	std::wstring *out_lookup; UINT out_len;

	// Fill lookup.
	out_lookup = new std::wstring[int2str.size()];
	for (UINT i = 0; i < numIDs; i++)
		out_lookup[i] = int2str.at(i);

	out_len = numIDs;

	// Return.
	*lookup = out_lookup;
	*len = out_len;
	return;
}