#pragma once

class OwnableEntity {
public:
    virtual std::wstring getOwnerUUID() = 0;
    virtual std::shared_ptr<Entity> getOwner() = 0;
};