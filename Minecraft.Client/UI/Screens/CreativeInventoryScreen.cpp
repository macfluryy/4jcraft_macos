#include "../../Platform/stdafx.h"
#include "CreativeInventoryScreen.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.entity.player.h"
#include "../../../Minecraft.World/Containers/Inventory.h"
#include "../../../Minecraft.World/Containers/SimpleContainer.h"
#include "../../../Minecraft.World/Containers/Slot.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.item.h"
#include "../../../Minecraft.World/Headers/net.minecraft.locale.h"
#include "../../Minecraft.h"
#include "../../Textures/Textures.h"
#include "../../Rendering/EntityRenderers/ItemRenderer.h"
#include "../../Rendering/Lighting.h"
#include "../../../Minecraft.World/Containers/InventoryMenu.h"
#include <algorithm>
#include <cmath>

// Static member initialization
int CreativeInventoryScreen::selectedTabIndex = IUIScene_CreativeMenu::eCreativeInventoryTab_BuildingBlocks;
std::shared_ptr<SimpleContainer> CreativeInventoryScreen::basicInventory = std::make_shared<SimpleContainer>(0, ITEMS_PER_PAGE);

// ContainerCreative implementation
CreativeInventoryScreen::ContainerCreative::ContainerCreative(std::shared_ptr<Player> player) : AbstractContainerMenu()
{
	std::shared_ptr<Inventory> inventoryplayer = player->inventory;
	
	// Add creative inventory slots (5 rows x 9 columns = 45 slots)
	for (int i = 0; i < ROWS; ++i)
	{
		for (int j = 0; j < COLUMNS; ++j)
		{
			addSlot(new Slot(basicInventory, i * COLUMNS + j, 9 + j * 18, 18 + i * 18));
		}
	}
	
	// Add hotbar slots (9 slots at bottom)
	for (int k = 0; k < 9; ++k)
	{
		addSlot(new Slot(inventoryplayer, k, 9 + k * 18, 112));
	}
	
	scrollTo(0.0f);
}

bool CreativeInventoryScreen::ContainerCreative::stillValid(std::shared_ptr<Player> player)
{
	return true;
}

std::shared_ptr<ItemInstance> CreativeInventoryScreen::ContainerCreative::clicked(int slotIndex, int buttonNum, int clickType, std::shared_ptr<Player> player)
{
	std::shared_ptr<Inventory> inventory = player->inventory;
	std::shared_ptr<ItemInstance> carried = inventory->getCarried();
	
	// Handle clicks outside the GUI
	if (slotIndex == SLOT_CLICKED_OUTSIDE)
	{
		// Drop the carried item
		if (carried != NULL)
		{
			if (buttonNum == 0)
			{
				player->drop(carried, true);
				inventory->setCarried(std::shared_ptr<ItemInstance>());
			}
			else if (buttonNum == 1)
			{
				std::shared_ptr<ItemInstance> single = carried->copy();
				single->count = 1;
				player->drop(single, true);
				carried->count--;
				if (carried->count <= 0)
				{
					inventory->setCarried(std::shared_ptr<ItemInstance>());
				}
			}
		}
		return std::shared_ptr<ItemInstance>();
	}
	
	// Validate slot index
	if (slotIndex < 0 || slotIndex >= (int)slots.size())
	{
		return std::shared_ptr<ItemInstance>();
	}
	
	Slot* slot = slots.at(slotIndex);
	
	// Handle creative inventory slots (0-44)
	if (slotIndex >= 0 && slotIndex < ITEMS_PER_PAGE)
	{
		std::shared_ptr<ItemInstance> slotItem = slot->getItem();
		
		// Handle SWAP (number key) - copy item to hotbar
		if (clickType == CLICK_SWAP)
		{
			if (slotItem != NULL && buttonNum >= 0 && buttonNum < 9)
			{
				std::shared_ptr<ItemInstance> copy = slotItem->copy();
				copy->count = copy->getMaxStackSize();
				inventory->setItem(buttonNum, copy);
			}
			return std::shared_ptr<ItemInstance>();
		}
		
		// Handle CLONE (middle click)
		if (clickType == CLICK_CLONE)
		{
			if (slotItem != NULL)
			{
				std::shared_ptr<ItemInstance> copy = slotItem->copy();
				copy->count = copy->getMaxStackSize();
				inventory->setCarried(copy);
			}
			return std::shared_ptr<ItemInstance>();
		}
		
		// Handle normal clicks
		if (slotItem != NULL)
		{
			if (buttonNum == 0)  // Left click
			{
				std::shared_ptr<ItemInstance> copy = slotItem->copy();
				copy->count = copy->getMaxStackSize();
				inventory->setCarried(copy);
			}
			else if (buttonNum == 1)  // Right click
			{
				std::shared_ptr<ItemInstance> copy = slotItem->copy();
				copy->count = 1;
				inventory->setCarried(copy);
			}
		}
		else if (carried != NULL)
		{
			// Clicking on empty creative slot with item - clear the carried item
			inventory->setCarried(std::shared_ptr<ItemInstance>());
		}
		
		return std::shared_ptr<ItemInstance>();
	}
	
	// For hotbar slots (45-53), use normal container behavior
	return AbstractContainerMenu::clicked(slotIndex, buttonNum, clickType, player);
}

void CreativeInventoryScreen::ContainerCreative::scrollTo(float pos)
{
	int i = (itemList.size() + COLUMNS - 1) / COLUMNS - ROWS;
	int j = (int)((double)(pos * (float)i) + 0.5);
	
	if (j < 0)
	{
		j = 0;
	}
	
	for (int k = 0; k < ROWS; ++k)
	{
		for (int l = 0; l < COLUMNS; ++l)
		{
			int i1 = l + (k + j) * COLUMNS;
			
			if (i1 >= 0 && i1 < (int)itemList.size())
			{
				basicInventory->setItem(l + k * COLUMNS, itemList[i1]);
			}
			else
			{
				basicInventory->setItem(l + k * COLUMNS, std::shared_ptr<ItemInstance>());
			}
		}
	}
}

bool CreativeInventoryScreen::ContainerCreative::canScroll()
{
	return itemList.size() > ITEMS_PER_PAGE;
}

CreativeInventoryScreen::CreativeInventoryScreen(std::shared_ptr<Player> player) 
	: AbstractContainerScreen(new ContainerCreative(player))
{
	this->player = player;
	player->containerMenu = menu;
	
	currentScroll = 0.0f;
	isScrolling = false;
	wasClicking = false;
	isLeftMouseDown = false;
	
	imageHeight = 136;
	imageWidth = 195;
}

void CreativeInventoryScreen::removed()
{
	AbstractContainerScreen::removed();
}

void CreativeInventoryScreen::init()
{
	buttons.clear();
	
	int i = selectedTabIndex;
	selectedTabIndex = -1;
	setCurrentCreativeTab(i);
}

void CreativeInventoryScreen::updateEvents()
{
#ifdef ENABLE_JAVA_GUIS
	// Handle mouse wheel scrolling.
	// We use ButtonDown with the scroll actions rather than GetScrollDelta() because
	// both share s_scrollTicksForButtonPressed; whichever is called first in a tick
	// zeroes it, so GetScrollDelta() would return 0 if hotbar scroll ran first.
	// ButtonDown/ScrollSnap() snapshots once per tick so all callers see the same value.
	if (needsScrollBars())
	{
		ContainerCreative* container = (ContainerCreative*)menu;
		int totalRows = ((int)container->itemList.size() + COLUMNS - 1) / COLUMNS;
		int scrollableRows = totalRows - ROWS;
		if (scrollableRows > 0)
		{
			float step = 1.0f / (float)scrollableRows;
			if (InputManager.ButtonDown(0, MINECRAFT_ACTION_LEFT_SCROLL))
			{
				currentScroll -= step;
				currentScroll = std::max(0.0f, std::min(1.0f, currentScroll));
				container->scrollTo(currentScroll);
			}
			else if (InputManager.ButtonDown(0, MINECRAFT_ACTION_RIGHT_SCROLL))
			{
				currentScroll += step;
				currentScroll = std::max(0.0f, std::min(1.0f, currentScroll));
				container->scrollTo(currentScroll);
			}
		}
	}
#endif
	Screen::updateEvents();
}

void CreativeInventoryScreen::containerTick()
{
}

void CreativeInventoryScreen::tick()
{
	Screen::tick();
}

void CreativeInventoryScreen::keyPressed(wchar_t eventCharacter, int eventKey)
{
	AbstractContainerScreen::keyPressed(eventCharacter, eventKey);
}

void CreativeInventoryScreen::mouseClicked(int x, int y, int buttonNum)
{
	if (buttonNum == 0) isLeftMouseDown = true;

	Screen::mouseClicked(x, y, buttonNum);

	if (buttonNum == 0 || buttonNum == 1)
	{
		int i = x - (width - imageWidth) / 2;
		int j = y - (height - imageHeight) / 2;

		// Check for tab clicks first; let mouseReleased handle the actual tab switch
		for (int tab = 0; tab < IUIScene_CreativeMenu::eCreativeInventoryTab_COUNT; ++tab)
		{
			if (isMouseOverTab(tab, i, j))
			{
				return;
			}
		}

		// Determine which slot (if any) was clicked
		Slot *slot = findSlot(x, y);

		int xo = (width - imageWidth) / 2;
		int yo = (height - imageHeight) / 2;
		bool clickedOutside = (x < xo || y < yo || x >= xo + imageWidth || y >= yo + imageHeight);

		int slotId = -1;
		if (slot != NULL) slotId = slot->index;
		if (clickedOutside) slotId = AbstractContainerMenu::SLOT_CLICKED_OUTSIDE;

		if (slotId == -1) return;

		bool quickKey = slotId != AbstractContainerMenu::SLOT_CLICKED_OUTSIDE &&
		                (Keyboard::isKeyDown(Keyboard::KEY_LSHIFT) || Keyboard::isKeyDown(Keyboard::KEY_RSHIFT));
		int clickType = quickKey ? AbstractContainerMenu::CLICK_QUICK_MOVE : AbstractContainerMenu::CLICK_PICKUP;

		// 4jcraft: bypass AbstractContainerScreen::mouseClicked / handleInventoryMouseClick
		// here intentionally. The normal path sends a ContainerClickPacket to the server,
		// where player->containerMenu is still the InventoryMenu (45 slots). Creative slot
		// indices 0-44 are valid in ContainerCreative but not in InventoryMenu, and hotbar
		// indices 45-53 exceed InventoryMenu's slot count entirely, causing an out-of-range
		// crash in AbstractContainerMenu::clicked on the server side.
		// Instead we apply the click locally and sync hotbar changes via SetCreativeModeSlotPacket.
		menu->clicked(slotId, buttonNum, clickType, minecraft->player);

		// 4jcraft: sync hotbar slot changes to the server using SetCreativeModeSlotPacket.
		// The packet handler (PlayerConnection::handleSetCreativeModeSlot) validates slots
		// against InventoryMenu coordinates where the hotbar starts at USE_ROW_SLOT_START (36),
		// so we must offset the local hotbar index (0-8) accordingly.
		if (slotId >= ITEMS_PER_PAGE && slotId < ITEMS_PER_PAGE + 9)
		{
			int hotbarSlot = slotId - ITEMS_PER_PAGE;
			std::shared_ptr<ItemInstance> hotbarItem = minecraft->player->inventory->getItem(hotbarSlot);
			minecraft->gameMode->handleCreativeModeItemAdd(hotbarItem, hotbarSlot + InventoryMenu::USE_ROW_SLOT_START);
		}
	}
}

void CreativeInventoryScreen::mouseReleased(int x, int y, int buttonNum)
{
	if (buttonNum == 0) isLeftMouseDown = false;

	if (buttonNum == 0)
	{
		int i = x - (width - imageWidth) / 2;
		int j = y - (height - imageHeight) / 2;
		
		// Check for tab clicks
		for (int tab = 0; tab < IUIScene_CreativeMenu::eCreativeInventoryTab_COUNT; ++tab)
		{
			if (isMouseOverTab(tab, i, j))
			{
				setCurrentCreativeTab(tab);
				return;
			}
		}
	}
	
	AbstractContainerScreen::mouseReleased(x, y, buttonNum);
}

void CreativeInventoryScreen::render(int xm, int ym, float a)
{
	// Java: drawDefaultBackground()
	renderBackground();
	
	// Handle scrollbar dragging
	bool mouseDown = isLeftMouseDown;
	int i = (width - imageWidth) / 2;
	int j = (height - imageHeight) / 2;
	int k = i + 175;
	int l = j + 18;
	int i1 = k + 14;
	int j1 = l + 112;
	
	if (!wasClicking && mouseDown && xm >= k && ym >= l && xm < i1 && ym < j1)
	{
		isScrolling = needsScrollBars();
	}
	
	if (!mouseDown)
	{
		isScrolling = false;
	}
	
	wasClicking = mouseDown;
	
	if (isScrolling)
	{
		currentScroll = ((float)(ym - l) - 7.5f) / ((float)(j1 - l) - 15.0f);
		currentScroll = std::max(0.0f, std::min(1.0f, currentScroll));
		((ContainerCreative*)menu)->scrollTo(currentScroll);
	}
	
	AbstractContainerScreen::render(xm, ym, a);
}

void CreativeInventoryScreen::renderLabels()
{
#ifdef ENABLE_JAVA_GUIS
	if (IUIScene_CreativeMenu::specs && selectedTabIndex >= 0 && selectedTabIndex < IUIScene_CreativeMenu::eCreativeInventoryTab_COUNT)
	{
		IUIScene_CreativeMenu::TabSpec* spec = IUIScene_CreativeMenu::specs[selectedTabIndex];
		if (spec)
		{
			std::wstring tabName = app.GetString(spec->m_descriptionId);
			font->draw(tabName, 8, 6, 0x404040);
		}
	}
#endif
}

void CreativeInventoryScreen::renderBg(float a)
{
	int x = (width - imageWidth) / 2;
	int y = (height - imageHeight) / 2;

#ifdef ENABLE_JAVA_GUIS
	// Render all non-selected tabs first
	for (int tab = 0; tab < IUIScene_CreativeMenu::eCreativeInventoryTab_COUNT; ++tab)
	{
		if (tab != selectedTabIndex)
		{
			drawTab(tab);
		}
	}
	
	// Load and render main creative inventory background
	int tex = minecraft->textures->loadTexture(TN_GUI_CREATIVE_TAB_ITEMS);
	glColor4f(1, 1, 1, 1);
	minecraft->textures->bind(tex);
	blit(x, y, 0, 0, imageWidth, imageHeight);
	
	// Render scrollbar
	tex = minecraft->textures->loadTexture(TN_GUI_CREATIVE_TABS);
	minecraft->textures->bind(tex);
	
	int scrollX = x + 175;
	int scrollY = y + 18;
	int scrollHeight = 112;
	
	if (needsScrollBars())
	{
		int scrollPos = (int)((float)(scrollHeight - 17) * currentScroll);
		blit(scrollX, scrollY + scrollPos, 232, 0, 12, 15);
	}
	else
	{
		blit(scrollX, scrollY + (scrollHeight - 17) / 2, 244, 0, 12, 15);
	}
	
	// Render selected tab last (on top)
	drawTab(selectedTabIndex);
#endif
}

void CreativeInventoryScreen::setCurrentCreativeTab(int tab)
{
	if (tab < 0 || tab >= IUIScene_CreativeMenu::eCreativeInventoryTab_COUNT)
		return;
	
	int oldTab = selectedTabIndex;
	selectedTabIndex = tab;
	
	ContainerCreative* container = (ContainerCreative*)menu;
	container->itemList.clear();
	
	// Populate itemList from the tab's category groups
	if (IUIScene_CreativeMenu::specs && IUIScene_CreativeMenu::specs[tab])
	{
		IUIScene_CreativeMenu::TabSpec* spec = IUIScene_CreativeMenu::specs[tab];
		
		// Add items from static groups
		for (int i = 0; i < spec->m_staticGroupsCount; ++i)
		{
			int groupIdx = spec->m_staticGroupsA[i];
			if (groupIdx >= 0 && groupIdx < IUIScene_CreativeMenu::eCreativeInventoryGroupsCount)
			{
				auto& group = IUIScene_CreativeMenu::categoryGroups[groupIdx];
				for (auto& item : group)
				{
					container->itemList.push_back(item);
				}
			}
		}
	}
	
	currentScroll = 0.0f;
	container->scrollTo(0.0f);
}

void CreativeInventoryScreen::selectTab(int tab)
{
	setCurrentCreativeTab(tab);
}

bool CreativeInventoryScreen::needsScrollBars()
{
	return ((ContainerCreative*)menu)->canScroll();
}

bool CreativeInventoryScreen::isMouseOverTab(int tab, int mouseX, int mouseY)
{
	int i = tab;
	int j = 28 * i;
	int k = 0;
	
	if (i > 0)
	{
		j += i;
	}
	
	// Tabs are in the top row
	k = k - 32;
	
	return mouseX >= j && mouseX <= j + 28 && mouseY >= k && mouseY <= k + 32;
}

void CreativeInventoryScreen::drawTab(int tab)
{
#ifdef ENABLE_JAVA_GUIS
	bool isSelected = (tab == selectedTabIndex);
	int i = tab;
	int j = i * 28;
	int k = 0;
	int l = (width - imageWidth) / 2 + 28 * i;
	int i1 = (height - imageHeight) / 2;
	int j1 = 32;
	
	if (isSelected)
	{
		k += 32;
	}
	
	if (i > 0)
	{
		l += i;
	}
	
	// Tabs are in the top row
	i1 = i1 - 28;
	
	// Render tab background
	int tex = minecraft->textures->loadTexture(TN_GUI_CREATIVE_TABS);
	minecraft->textures->bind(tex);
	glColor4f(1, 1, 1, 1);
	blit(l, i1, j, k, 28, 32);
	
	// TODO: Render tab icon (would need item icons)
#endif
}
