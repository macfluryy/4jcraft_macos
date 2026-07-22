#pragma once

class SimpleContainer;







namespace net_minecraft_world {
class ContainerListener {
    friend class ::SimpleContainer;

private:
    virtual void containerChanged() = 0;
};
}  