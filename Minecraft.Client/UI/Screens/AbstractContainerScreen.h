#pragma once
#include "../Screen.h"
class ItemRenderer;
class AbstractContainerMenu;
class Slot;
class Container;

class AbstractContainerScreen : public Screen {
protected:
    // 4jcraft: made protected to match JE 1.6.4
    static ItemRenderer* itemRenderer;

protected:
    int imageWidth;
    int imageHeight;
    // int leftPos, topPos;
public:
    AbstractContainerMenu* menu;

    AbstractContainerScreen(AbstractContainerMenu* menu);
    virtual void init() override;
    virtual void render(int xm, int ym, float a) override;

protected:
    virtual void renderLabels();
    virtual void renderBg(float a) = 0;
    // 4jcraft: promoted from private to protected so CreativeInventoryScreen
    // can call findSlot() and isHovering() directly for its custom click
    // handling.
    virtual Slot* findSlot(int x, int y);
    virtual bool isHovering(Slot* slot, int xm, int ym);
    // 4jcraft: extracted from render() into a standalone method so this can be
    // used in other classes
    virtual void renderTooltip(std::shared_ptr<ItemInstance> item, int x,
                               int y);

private:
    virtual void renderSlot(Slot* slot);

protected:
    virtual void mouseClicked(int x, int y, int buttonNum) override;
    virtual void mouseReleased(int x, int y, int buttonNum) override;
    virtual void keyPressed(wchar_t eventCharacter, int eventKey) override;

public:
    virtual void removed() override;
    virtual void slotsChanged(std::shared_ptr<Container> container);
    virtual bool isPauseScreen() override;
    virtual void tick() override;
};