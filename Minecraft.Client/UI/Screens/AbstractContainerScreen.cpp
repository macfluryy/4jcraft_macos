#include "../../Platform/stdafx.h"
#include "AbstractContainerScreen.h"
#include "../../Rendering/EntityRenderers/ItemRenderer.h"
#include "../../Player/MultiPlayerLocalPlayer.h"
#include "../../Rendering/Lighting.h"
#include "../../GameState/GameMode.h"
#include "../../Input/KeyMapping.h"
#include "../../GameState/Options.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/Headers/net.minecraft.locale.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.item.h"

ItemRenderer *AbstractContainerScreen::itemRenderer = new ItemRenderer();

AbstractContainerScreen::AbstractContainerScreen(AbstractContainerMenu *menu)
{
	// 4J - added initialisers
	imageWidth = 176;
	imageHeight = 166;

	this->menu = menu;
}

void AbstractContainerScreen::init()
{
    Screen::init();
    minecraft->player->containerMenu = menu;
// 	leftPos = (width - imageWidth) / 2;
// 	topPos = (height - imageHeight) / 2;

}

void AbstractContainerScreen::render(int xm, int ym, float a)
{
	// 4J Stu - Not used
#if ENABLE_JAVA_GUIS
    renderBackground();
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;

    renderBg(a);

    glPushMatrix();
    glRotatef(120, 1, 0, 0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef((float)xo, (float)yo, 0);

    glColor4f(1, 1, 1, 1);
    glEnable(GL_RESCALE_NORMAL);
    Lighting::turnOnGui();

    Slot *hoveredSlot = NULL;
	
	AUTO_VAR(itEnd, menu->slots->end());
	for (AUTO_VAR(it, menu->slots->begin()); it != itEnd; it++)
	{
        Slot *slot = *it; //menu->slots->at(i);

        renderSlot(slot);

        if (isHovering(slot, xm, ym))
		{
            hoveredSlot = slot;

            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);

            int x = slot->x;
            int y = slot->y;
            fillGradient(x, y, x + 16, y + 16, 0x80ffffff, 0x80ffffff);
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);
        }
    }

    std::shared_ptr<Inventory> inventory = minecraft->player->inventory;
    if (inventory->getCarried() != NULL)
	{
        glTranslatef(0, 0, 32);
        // Slot old = carriedSlot;
        // carriedSlot = null;
        itemRenderer->renderGuiItem(font, minecraft->textures, inventory->getCarried(), xm - xo - 8, ym - yo - 8);
        itemRenderer->renderGuiItemDecorations(font, minecraft->textures, inventory->getCarried(), xm - xo - 8, ym - yo - 8);
        // carriedSlot = old;
    }
    Lighting::turnOff();
    glDisable(GL_RESCALE_NORMAL);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    renderLabels();

    // 4jcraft: newer tooltips backported from java edition 1.3.x (MCP 7.x)
    if (inventory->getCarried() == NULL && hoveredSlot != NULL &&
        hoveredSlot->hasItem()) {
        std::shared_ptr<ItemInstance> item = hoveredSlot->getItem();

        // std::wstring elementName =
        // trimString(Language::getInstance()->getElementName(hoveredSlot->getItem()->getDescriptionId()));
        std::vector<std::wstring> elementName;
        std::vector<std::wstring>* tooltipLines =
            item->getHoverText(minecraft->player, false, elementName);

        if (tooltipLines != NULL && tooltipLines->size() > 0) {
            int tooltipWidth = 0;
            std::vector<std::wstring> cleanedLines;
            std::vector<int> lineColors;

            for (int lineIndex = 0; lineIndex < (int)tooltipLines->size();
                 ++lineIndex) {
                std::wstring rawLine = (*tooltipLines)[lineIndex];
                std::wstring clean = L"";
                int lineColor = 0xffffffff;

                // 4jcraft: LCE is using HTML font elements for its tooltip
                // colors, so make sure to parse them for parity w iggy UI
                //
                // examples would be enchantment books, potions and music
                // discs
                size_t fontPos = rawLine.find(L"<font");
                if (fontPos != std::wstring::npos) {
                    size_t colorPos = rawLine.find(L"color=\"", fontPos);
                    if (colorPos != std::wstring::npos) {
                        colorPos += 7;
                        size_t colorEnd = rawLine.find(L'"', colorPos);
                        if (colorEnd != std::wstring::npos) {
                            std::wstring colorStr =
                                rawLine.substr(colorPos, colorEnd - colorPos);
                            if (!colorStr.empty() && colorStr[0] == L'#') {
                                colorStr = colorStr.substr(1);
                            }
                            if (!colorStr.empty()) {
                                wchar_t* endPtr;
                                long hexColor =
                                    wcstol(colorStr.c_str(), &endPtr, 16);
                                if (*endPtr == L'\0') {
                                    lineColor = 0xff000000 | (int)hexColor;
                                }
                            }
                        }
                    }
                }

                bool inTag = false;
                for (wchar_t currentChar : rawLine) {
                    if (currentChar == L'<') {
                        inTag = true;
                    } else if (currentChar == L'>') {
                        inTag = false;
                    } else if (!inTag) {
                        clean += currentChar;
                    }
                }

                cleanedLines.push_back(clean);
                lineColors.push_back(lineColor);

                int lineWidth = font->width(clean);
                if (lineWidth > tooltipWidth) {
                    tooltipWidth = lineWidth;
                }
            }

            int tooltipX = xm - xo + 12;
            int tooltipY = ym - yo - 12;

            int tooltipHeight = 8;

            if (tooltipLines->size() > 1) {
                tooltipHeight += 2 + (tooltipLines->size() - 1) * 10;
            }

            int bgColor = 0xf0100010;
            fillGradient(tooltipX - 3, tooltipY - 4,
                         tooltipX + tooltipWidth + 3, tooltipY - 3, bgColor,
                         bgColor);
            fillGradient(tooltipX - 3, tooltipY + tooltipHeight + 3,
                         tooltipX + tooltipWidth + 3,
                         tooltipY + tooltipHeight + 4, bgColor, bgColor);
            fillGradient(tooltipX - 3, tooltipY - 3,
                         tooltipX + tooltipWidth + 3,
                         tooltipY + tooltipHeight + 3, bgColor, bgColor);
            fillGradient(tooltipX - 4, tooltipY - 3, tooltipX - 3,
                         tooltipY + tooltipHeight + 3, bgColor, bgColor);
            fillGradient(tooltipX + tooltipWidth + 3, tooltipY - 3,
                         tooltipX + tooltipWidth + 4,
                         tooltipY + tooltipHeight + 3, bgColor, bgColor);

            int borderStart = 0x505000ff;
            int borderFinish =
                (borderStart & 0xfefefe) >> 1 | borderStart & 0xff000000;
            fillGradient(tooltipX - 3, (tooltipY - 3) + 1, (tooltipX - 3) + 1,
                         (tooltipY + tooltipHeight + 3) - 1, borderStart,
                         borderFinish);
            fillGradient(tooltipX + tooltipWidth + 2, (tooltipY - 3) + 1,
                         tooltipX + tooltipWidth + 3,
                         (tooltipY + tooltipHeight + 3) - 1, borderStart,
                         borderFinish);
            fillGradient(tooltipX - 3, tooltipY - 3,
                         tooltipX + tooltipWidth + 3, (tooltipY - 3) + 1,
                         borderStart, borderStart);
            fillGradient(tooltipX - 3, tooltipY + tooltipHeight + 2,
                         tooltipX + tooltipWidth + 3,
                         tooltipY + tooltipHeight + 3, borderFinish,
                         borderFinish);

            int currentY = tooltipY;
            for (int lineIndex = 0; lineIndex < (int)tooltipLines->size();
                 ++lineIndex) {
                std::wstring& currentLine = cleanedLines[lineIndex];
                int textColor;

                if (lineIndex == 0) {
                    textColor = app.GetHTMLColour(item->getRarity()->color);
                } else {
                    textColor = (lineColors[lineIndex] != 0xffffffff)
                                    ? lineColors[lineIndex]
                                    : 0xffaaaaaa;
                }

                font->drawShadow(currentLine, tooltipX, currentY, textColor);

                if (lineIndex == 0) {
                    currentY += 2;
                }

                currentY += 10;
            }
        }
    }

    glPopMatrix();

    Screen::render(xm, ym, a);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
#endif
}

void AbstractContainerScreen::renderLabels()
{
}

void AbstractContainerScreen::renderSlot(Slot *slot)
{
	// 4J Unused
#if ENABLE_JAVA_GUIS
    int x = slot->x;
    int y = slot->y;
    std::shared_ptr<ItemInstance> item = slot->getItem();

    // if (item == NULL)
	// {
    //     int icon = slot->getNoItemIcon();
    //     if (icon >= 0)
	// 	{
    //         glDisable(GL_LIGHTING);
    //         minecraft->textures->bind(minecraft->textures->loadTexture(TN_GUI_ITEMS));//L"/gui/items.png"));
    //         blit(x, y, icon % 16 * 16, icon / 16 * 16, 16, 16);
    //         glEnable(GL_LIGHTING);
    //         return;
    //     }
    // }

    itemRenderer->renderGuiItem(font, minecraft->textures, item, x, y);
    itemRenderer->renderGuiItemDecorations(font, minecraft->textures, item, x, y);
#endif
}

Slot *AbstractContainerScreen::findSlot(int x, int y)
{
	AUTO_VAR(itEnd, menu->slots->end());
	for (AUTO_VAR(it, menu->slots->begin()); it != itEnd; it++)
	{
        Slot *slot = *it; //menu->slots->at(i);
        if (isHovering(slot, x, y)) return slot;
    }
    return NULL;
}

bool AbstractContainerScreen::isHovering(Slot *slot, int xm, int ym)
{
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;
    xm -= xo;
    ym -= yo;

    return xm >= slot->x - 1 && xm < slot->x + 16 + 1 && ym >= slot->y - 1 && ym < slot->y + 16 + 1;

}

void AbstractContainerScreen::mouseClicked(int x, int y, int buttonNum)
{
    Screen::mouseClicked(x, y, buttonNum);
    if (buttonNum == 0 || buttonNum == 1)
	{
        Slot *slot = findSlot(x, y);

        int xo = (width - imageWidth) / 2;
        int yo = (height - imageHeight) / 2;
        bool clickedOutside = (x < xo || y < yo || x >= xo + imageWidth || y >= yo + imageHeight);

        int slotId = -1;
        if (slot != NULL) slotId = slot->index;

        if (clickedOutside)
		{
            slotId = AbstractContainerMenu::CLICKED_OUTSIDE;
        }

        if (slotId != -1)
		{
            bool quickKey = slotId != AbstractContainerMenu::CLICKED_OUTSIDE && (Keyboard::isKeyDown(Keyboard::KEY_LSHIFT) || Keyboard::isKeyDown(Keyboard::KEY_RSHIFT));
            minecraft->gameMode->handleInventoryMouseClick(menu->containerId, slotId, buttonNum, quickKey, minecraft->player);
        }
    }

}

void AbstractContainerScreen::mouseReleased(int x, int y, int buttonNum)
{
    if (buttonNum == 0)
	{
    }
}

void AbstractContainerScreen::keyPressed(wchar_t eventCharacter, int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE || eventKey == minecraft->options->keyBuild->key)
	{
        minecraft->player->closeContainer();
    }
}

void AbstractContainerScreen::removed()
{
    if (minecraft->player == NULL) return;
}

void AbstractContainerScreen::slotsChanged(std::shared_ptr<Container> container)
{
}

bool AbstractContainerScreen::isPauseScreen()
{
	return false;
}

void AbstractContainerScreen::tick()
{
    Screen::tick();
    if (!minecraft->player->isAlive() || minecraft->player->removed) minecraft->player->closeContainer();

}