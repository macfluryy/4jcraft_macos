#pragma once

#include <functional>
#include <memory>
#include <set>
#include <string>

typedef std::function<std::wstring(void)> StringBuilder;

class UIString {
protected:
    static int s_currentLanguage;
    static int s_currentLocale;

public:
    static bool setCurrentLanguage();
    static int getCurrentLanguage();

protected:
    class UIStringCore : public std::enable_shared_from_this<UIStringCore> {
    private:
        int m_lastSetLanguage;
        int m_lastSetLocale;

        int m_lastUpdatedLanguage;
        int m_lastUpdatedLocale;

        std::wstring m_wstrCache;

        bool m_bIsConstant;

        StringBuilder m_fStringBuilder;

    public:
        UIStringCore(StringBuilder wstrBuilder);
        UIStringCore(const std::wstring& str);

        std::wstring& getString();

        bool hasNewString();
        bool update(bool force);

        bool needsUpdating();
        void setUpdated();
    };

    std::shared_ptr<UIStringCore> m_core;

public:
    UIString();

    UIString(int ids);  // Create a dynamic UI std::string from a std::string id
                        // value.

    UIString(StringBuilder wstrBuilder);  // Create a dynamic UI std::string
                                          // with a custom update function.

    // Create a UIString with a constant value.
    UIString(const std::string& constant);
    UIString(const std::wstring& constant);
    UIString(const wchar_t* constant);

    ~UIString();

    bool empty();
    bool compare(const UIString& uiString);

    bool needsUpdating();  // Language has been change since the last time
                           // setUpdated was called.
    void setUpdated();     // The new text has been used.

    std::wstring& getString();

    const wchar_t* c_str();
    unsigned int length();
};