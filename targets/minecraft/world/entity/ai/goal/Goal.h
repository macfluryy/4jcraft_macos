#pragma once

class Level;

class Goal {
private:
    int _requiredControlFlags;

protected:
    Goal();

public:
    virtual ~Goal() {}
    virtual bool canUse() = 0;
    virtual bool canContinueToUse();
    virtual bool canInterrupt();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual void setRequiredControlFlags(int requiredControlFlags);
    virtual int getRequiredControlFlags();

    
    
    virtual void setLevel(Level* level) {};
};