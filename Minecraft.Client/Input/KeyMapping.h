#pragma once

// 4J Stu - Not updated to 1.8.2 as we don't use this
class KeyMapping
{
public:
	std::wstring name;
	int key;
	KeyMapping(const std::wstring& name, int key);
};