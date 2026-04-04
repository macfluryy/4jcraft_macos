#pragma once

#include <string>

#include "ObjectiveCriteria.h"

class DummyCriteria : public ObjectiveCriteria {
private:
    std::wstring name;

public:
    DummyCriteria(const std::wstring& name);

    virtual std::wstring getName();
    virtual int getScoreModifier(
        std::vector<std::shared_ptr<Player> >* players);
    virtual bool isReadOnly();
};