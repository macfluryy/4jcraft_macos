#pragma once
#include "AbstractContainerScreen.h"
#include "../../../Minecraft.World/Containers/AbstractContainerMenu.h"
#include "../../Platform/Common/UI/IUIScene_CreativeMenu.h"
#include "../../Player/MultiPlayerLocalPlayer.h"

class Player;
class SimpleContainer;
class Inventory;
class Slot;

class CreativeInventoryScreen : public AbstractContainerScreen
{
private:
	static constexpr int ROWS = 5;
	static constexpr int COLUMNS = 9;
	static constexpr int ITEMS_PER_PAGE = ROWS * COLUMNS; // 45 items (9x5 grid)
	
	// Currently selected creative tab index
	static int selectedTabIndex;
	
	// Temporary inventory for creative mode items
	static std::shared_ptr<SimpleContainer> basicInventory;
	
	// Amount scrolled in Creative mode inventory (0 = top, 1 = bottom)
	float currentScroll;
	
	bool isScrolling;
	
	// True if the left mouse button is currently being held
	bool isLeftMouseDown;

	// True if the left mouse button was held down last time render was called
	bool wasClicking;

	std::shared_ptr<Player> player;
	
public:
	class ContainerCreative : public AbstractContainerMenu
	{
	public:
		std::vector<std::shared_ptr<ItemInstance>> itemList;
		
		ContainerCreative(std::shared_ptr<Player> player);
		virtual bool stillValid(std::shared_ptr<Player> player);
		virtual std::shared_ptr<ItemInstance> clicked(int slotIndex, int buttonNum, int clickType, std::shared_ptr<Player> player);
		void scrollTo(float pos);
		bool canScroll();
	};
	
public:
	CreativeInventoryScreen(std::shared_ptr<Player> player);
	virtual void removed();
	virtual void init();
	virtual void containerTick();
	virtual void tick();
	virtual void updateEvents();
	virtual void keyPressed(wchar_t eventCharacter, int eventKey);
	virtual void mouseClicked(int x, int y, int buttonNum);
	virtual void mouseReleased(int x, int y, int buttonNum);
	virtual void render(int xm, int ym, float a);
protected:
	virtual void renderLabels();
	virtual void renderBg(float a);
private:
	void setCurrentCreativeTab(int tab);
	void selectTab(int tab);
	bool needsScrollBars();
	bool isMouseOverTab(int tab, int mouseX, int mouseY);
	void drawTab(int tab);
};
