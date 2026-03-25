#pragma once

#include "GameRuleDefinition.h"

class NamedAreaRuleDefinition : public GameRuleDefinition {
private:
    std::wstring m_name;
    AABB* m_area;

public:
    NamedAreaRuleDefinition();
    ~NamedAreaRuleDefinition();

    virtual void writeAttributes(DataOutputStream* dos,
                                 unsigned int numAttributes);

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_NamedArea;
    }

    virtual void addAttribute(const std::wstring& attributeName,
                              const std::wstring& attributeValue);

    AABB* getArea() { return m_area; }
    std::wstring getName() { return m_name; }
};
