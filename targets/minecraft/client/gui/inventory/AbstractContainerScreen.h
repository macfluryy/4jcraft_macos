#pragma once
#include <format>
#include <memory>
#include <string>
#include <vector>

#include "minecraft/client/gui/Screen.h"

class ItemRenderer;
class AbstractContainerMenu;
class Slot;
class Container;
class ItemInstance;

class AbstractContainerScreen : public Screen {
protected:
    
    static ItemRenderer* itemRenderer;

protected:
    int imageWidth;
    int imageHeight;
    
public:
    AbstractContainerMenu* menu;

    AbstractContainerScreen(AbstractContainerMenu* menu);
    virtual void init() override;
    virtual void render(int xm, int ym, float a) override;

protected:
    virtual void renderLabels();
    virtual void renderBg(float a) = 0;
    
    
    
    virtual Slot* findSlot(int x, int y);
    
    
    virtual bool isHoveringOver(int x, int y, int w, int h, int xm, int ym);
    virtual bool isHovering(Slot* slot, int xm, int ym);
    
    
    virtual void renderTooltipInternal(
        const std::vector<std::wstring>& cleanedLines,
        const std::vector<int>& lineColors, int xm, int ym);
    virtual void renderTooltip(std::shared_ptr<ItemInstance> item, int xm,
                               int ym);

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

    
    virtual void renderTooltip(const std::vector<std::wstring>& lines, int xm,
                               int ym);
    virtual void renderTooltip(const std::wstring& line, int xm, int ym);
};
