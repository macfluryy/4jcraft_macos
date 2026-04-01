#pragma once

class DescFormatter {
public:
    virtual ~DescFormatter() = default;
    virtual std::wstring format(const std::wstring& i18nValue) = 0;
};
