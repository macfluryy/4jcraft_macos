#pragma once
using namespace std;

class User
{
public:
	static vector<Tile *> allowedTiles;
	static void staticCtor();
	std::wstring name;
	std::wstring sessionId;
	std::wstring mpPassword;

	User(const std::wstring& name, const std::wstring& sessionId);
};