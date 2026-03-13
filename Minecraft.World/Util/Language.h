#pragma once

class Language
{
private:
	static Language *singleton;
public:
	Language();
    static Language *getInstance();
    std::wstring getElement(std::wstring elementId, ...);
	std::wstring getElement(const std::wstring& elementId, va_list args);
    std::wstring getElementName(const std::wstring& elementId);
    std::wstring getElementDescription(const std::wstring& elementId);
};
