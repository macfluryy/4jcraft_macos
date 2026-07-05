#pragma once
#include <stdint.h>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "platform/PlatformTypes.h"
#include "GuiComponent.h"
#include "minecraft/client/GuiMessage.h"
#include "minecraft/client/renderer/entity/ItemRenderer.h"

class Random;
class Minecraft;
class ItemInstance;
class ItemRenderer;
class ResourceLocation;

class Gui : public GuiComponent {
private:
    static ResourceLocation PUMPKIN_BLUR_LOCATION;
    static ResourceLocation GUI_GUI_LOCATION;
    static ResourceLocation GUI_ICONS_LOCATION;
    // 4J-PB - this doesn't account for the safe zone, and the indent applied to
    // messages
    // static const int MAX_MESSAGE_WIDTH = 320;
    static const int m_iMaxMessageWidth = 280;
    static ItemRenderer* itemRenderer;
    std::vector<GuiMessage> guiMessages[XUSER_MAX_COUNT];
    Random* random;

    Minecraft* minecraft;

public:
    std::wstring selectedName;

private:
    int tickCount;
    std::wstring overlayMessageString;
    int overlayMessageTime;
    bool animateOverlayMessageColor;

    // 4J Added
    float lastTickA;
    float fAlphaIncrementPerCent;

    // 4jcraft: backported item switch tooltip display from 1.6.4
    int remainingHighlightTicks;
    std::shared_ptr<ItemInstance> highlightingItemStack;

    // 4jcraft: sidebar scoreboard render cache - rebuilt only when the
    // scoreboard revision changes. Strings, ordering, widths and offsets are
    // all precomputed there; the per-frame path only draws.
    struct SidebarLine {
        std::wstring text;
        std::wstring value;
        int valueWidth = 0;  // font width of `value`, cached at rebuild
    };
    int m_sidebarRevision = -1;
    std::wstring m_sidebarTitle;
    std::vector<SidebarLine> m_sidebarLines;
    int m_sidebarWidth = 0;
    int m_sidebarTitleOffset = 0;  // centered title x offset, cached

public:
    static float currentGuiBlendFactor;  // 4J added
    static float currentGuiScaleFactor;  // 4J added

    float progress;

    //    private DecimalFormat df = new DecimalFormat("##.00");

public:
    Gui(Minecraft* minecraft);

    void render(float a, bool mouseFree, int xMouse, int yMouse);
    float tbr;

private:
    void renderBossHealth(void);
    void renderPumpkin(int w, int h);
    void renderVignette(float br, int w, int h);
    void renderTp(float br, int w, int h);
    void renderSlot(int slot, int x, int y, float a);
    void renderPlayerList(int screenWidth, int screenHeight);
    void renderSidebar(int screenWidth, int screenHeight);

public:
    void tick();
    void clearMessages(int iPad = -1);
    void addMessage(const std::wstring& string, int iPad,
                    bool bIsDeathMessage = false);
    void setNowPlaying(const std::wstring& string);
    void displayClientMessage(int messageId, int iPad);

    // 4J Added
    std::size_t getMessagesCount(int iPad) { return guiMessages[iPad].size(); }
    std::wstring getMessage(int iPad, std::size_t index) {
        return guiMessages[iPad].at(index).string;
    }
    float getOpacity(int iPad, std::size_t index);

    std::wstring getJukeboxMessage(int iPad) { return overlayMessageString; }
    float getJukeboxOpacity(int iPad);

    // 4J Added
    void renderGraph(int dataLength, int dataPos, int64_t* dataA,
                     float dataAScale, int dataAWarning, int64_t* dataB,
                     float dataBScale, int dataBWarning);
    void renderStackedGraph(int dataPos, int dataLength, int dataSources,
                            int64_t (*func)(unsigned int dataPos,
                                            unsigned int dataSource));
};
