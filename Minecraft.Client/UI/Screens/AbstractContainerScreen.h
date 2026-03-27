#pragma once
#include "../Screen.h"
class ItemRenderer;
class AbstractContainerMenu;
class Slot;
class Container;

class AbstractContainerScreen : public Screen {
private:
    static ItemRenderer* itemRenderer;

protected:
    int imageWidth;
    int imageHeight;
    // int leftPos, topPos;
public:
    AbstractContainerMenu* menu;

    AbstractContainerScreen(AbstractContainerMenu* menu);
    virtual void init();
    virtual void render(int xm, int ym, float a);

protected:
    virtual void renderLabels();
    virtual void renderBg(float a) = 0;
    // 4jcraft: promoted from private to protected so CreativeInventoryScreen
    // can call findSlot() and isHovering() directly for its custom click
    // handling.
    virtual Slot* findSlot(int x, int y);
    virtual bool isHovering(Slot* slot, int xm, int ym);

private:
    virtual void renderSlot(Slot* slot);

protected:
    virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);
    virtual void keyPressed(wchar_t eventCharacter, int eventKey);

public:
    virtual void removed();
    virtual void slotsChanged(std::shared_ptr<Container> container);
    virtual bool isPauseScreen();
    virtual void tick();
};
