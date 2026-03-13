#pragma once

class Random;

class PortalForcer {
private:
    Random* random;

public:
    // 4J Stu Added - Java has no ctor, but we need to initialise random
    PortalForcer();

    void force(Level* level, std::shared_ptr<Entity> e);

public:
    bool findPortal(Level* level, std::shared_ptr<Entity> e);

public:
    bool createPortal(Level* level, std::shared_ptr<Entity> e);
};