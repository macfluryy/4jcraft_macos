#pragma once

class GameRules {
private:
    class GameRule {
    private:
        std::wstring value;
        bool booleanValue;
        int intValue;
        double doubleValue;

    public:
        GameRule(const std::wstring& startValue);

        void set(const std::wstring& newValue);
        std::wstring get();
        bool getBoolean();
        int getInt();
        double getDouble();
    };

public:
    // 4J: Originally strings
    // default rules
    static const int RULE_DOFIRETICK;
    static const int RULE_MOBGRIEFING;
    static const int RULE_KEEPINVENTORY;
    static const int RULE_DOMOBSPAWNING;
    static const int RULE_DOMOBLOOT;
    static const int RULE_DOTILEDROPS;
    static const int RULE_COMMANDBLOCKOUTPUT;
    static const int RULE_NATURAL_REGENERATION;
    static const int RULE_DAYLIGHT;

private:
    std::unordered_map<std::wstring, GameRule*> rules;

public:
    GameRules();
    ~GameRules();

    bool getBoolean(const int rule);

    // 4J: Removed unused functions
    /*void set(const std::wstring &ruleName, const std::wstring &newValue);
    void registerRule(const std::wstring &name, const std::wstring &startValue);
    std::wstring get(const std::wstring &ruleName);
    int getInt(const std::wstring &ruleName);
    double getDouble(const std::wstring &ruleName);
    CompoundTag *createTag();
    void loadFromTag(CompoundTag *tag);
    vector<std::wstring> *getRuleNames();
    bool contains(const std::wstring &rule);*/
};