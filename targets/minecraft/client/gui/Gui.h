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

    
    float lastTickA;
    float fAlphaIncrementPerCent;

    
    int remainingHighlightTicks;
    std::shared_ptr<ItemInstance> highlightingItemStack;

    
    
    std::wstring m_titleText;
    std::wstring m_subtitleText;
    int m_titleFadeIn = 10;    
    int m_titleStay = 70;
    int m_titleFadeOut = 20;
    int m_titleTicksLeft = 0;  

    
    
    
    struct SidebarLine {
        std::wstring text;
        std::wstring value;
        int valueWidth = 0;  
    };
    int m_sidebarRevision = -1;
    std::wstring m_sidebarTitle;
    std::vector<SidebarLine> m_sidebarLines;
    int m_sidebarWidth = 0;
    int m_sidebarTitleOffset = 0;  

public:
    static float currentGuiBlendFactor;  
    static float currentGuiScaleFactor;  

    float progress;

    

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
    void renderTitles(int screenWidth, int screenHeight, float a);

public:
    
    void setActionBar(const std::wstring& text);
    void setTitle(const std::wstring& text);
    void setSubtitle(const std::wstring& text);
    void setTitleTimes(int fadeIn, int stay, int fadeOut);
    void clearTitles(bool resetTimes);

private:

public:
    void tick();
    void clearMessages(int iPad = -1);
    void addMessage(const std::wstring& string, int iPad,
                    bool bIsDeathMessage = false);
    void setNowPlaying(const std::wstring& string);
    void displayClientMessage(int messageId, int iPad);

    
    std::size_t getMessagesCount(int iPad) { return guiMessages[iPad].size(); }
    std::wstring getMessage(int iPad, std::size_t index) {
        return guiMessages[iPad].at(index).string;
    }
    float getOpacity(int iPad, std::size_t index);

    std::wstring getJukeboxMessage(int iPad) { return overlayMessageString; }
    float getJukeboxOpacity(int iPad);

    
    void renderGraph(int dataLength, int dataPos, int64_t* dataA,
                     float dataAScale, int dataAWarning, int64_t* dataB,
                     float dataBScale, int dataBWarning);
    void renderStackedGraph(int dataPos, int dataLength, int dataSources,
                            int64_t (*func)(unsigned int dataPos,
                                            unsigned int dataSource));
};
