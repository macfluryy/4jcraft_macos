#pragma once

#include <string>
#include <unordered_map>

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

    
    








};