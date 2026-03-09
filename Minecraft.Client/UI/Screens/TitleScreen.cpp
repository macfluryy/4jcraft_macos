#include "../../Platform/stdafx.h"
#include "../Button.h"
#include "OptionsScreen.h"
#include "SelectWorldScreen.h"
#include "JoinMultiplayerScreen.h"
#include "../../Rendering/Tesselator.h"
#include "../../Textures/Textures.h"
#include "../../../Minecraft.World/Util/StringHelpers.h"
#include "../../../Minecraft.World/IO/Streams/InputOutputStream.h"
#include "../../../Minecraft.World/Headers/net.minecraft.locale.h"
#include "../../../Minecraft.World/Platform/System.h"
#include "../../../Minecraft.World/Util/Random.h"
#include "TitleScreen.h"

Random *TitleScreen::random = new Random();

TitleScreen::TitleScreen()
{
	// 4J - added initialisers
	vo = 0;
	multiplayerButton = NULL;

    splash = L"missingno";
//    try {	// 4J - removed try/catch
    std::vector<std::wstring> splashes;

	/*
    BufferedReader *br = new BufferedReader(new InputStreamReader(InputStream::getResourceAsStream(L"res\\title\\splashes.txt"))); //, Charset.forName("UTF-8")
		
    std::wstring line = L"";
    while ( !(line = br->readLine()).empty() )
	{
        line = trimString( line );
        if (line.length() > 0)
		{
            splashes.push_back(line);
        }
    }
	
    br->close();
	delete br;
	*/

    splash = L""; //splashes.at(random->nextInt(splashes.size()));

//    } catch (Exception e) {
//    }
}

void TitleScreen::tick()
{
	//vo += 1.0f;
	//if( vo > 100.0f ) minecraft->setScreen(new SelectWorldScreen(this));		// 4J - temp testing
}

void TitleScreen::keyPressed(wchar_t eventCharacter, int eventKey)
{
}

void TitleScreen::init()
{
	/* 4J - removed
    Calendar c = Calendar.getInstance();
    c.setTime(new Date());

    if (c.get(Calendar.MONTH) + 1 == 11 && c.get(Calendar.DAY_OF_MONTH) == 9) {
        splash = "Happy birthday, ez!";
    } else if (c.get(Calendar.MONTH) + 1 == 6 && c.get(Calendar.DAY_OF_MONTH) == 1) {
        splash = "Happy birthday, Notch!";
    } else if (c.get(Calendar.MONTH) + 1 == 12 && c.get(Calendar.DAY_OF_MONTH) == 24) {
        splash = "Merry X-mas!";
    } else if (c.get(Calendar.MONTH) + 1 == 1 && c.get(Calendar.DAY_OF_MONTH) == 1) {
        splash = "Happy new year!";
    }
	*/

    Language *language = Language::getInstance();

    const int spacing = 24;
    const int topPos = height / 4 + spacing * 2;

    buttons.push_back(new Button(1, width / 2 - 100, topPos, language->getElement(L"menu.singleplayer")));
    buttons.push_back(multiplayerButton = new Button(2, width / 2 - 100, topPos + spacing * 1, language->getElement(L"menu.multiplayer")));
    buttons.push_back(new Button(3, width / 2 - 100, topPos + spacing * 2, language->getElement(L"menu.mods")));

    if (minecraft->appletMode)
	{
        buttons.push_back(new Button(0, width / 2 - 100, topPos + spacing * 3, language->getElement(L"menu.options")));
    }
	else
	{
        buttons.push_back(new Button(0, width / 2 - 100, topPos + spacing * 3 + 12, 98, 20, language->getElement(L"menu.options")));
        buttons.push_back(new Button(4, width / 2 + 2, topPos + spacing * 3 + 12, 98, 20, language->getElement(L"menu.quit")));
    }

    if (minecraft->user == NULL)
	{
        multiplayerButton->active = false;
    }

}

void TitleScreen::buttonClicked(Button *button)
{
    if (button->id == 0)
	{
        minecraft->setScreen(new OptionsScreen(this, minecraft->options));
    }
    if (button->id == 1)
	{
        minecraft->setScreen(new SelectWorldScreen(this));
    }
    if (button->id == 2)
	{
        minecraft->setScreen(new JoinMultiplayerScreen(this));
    }
    if (button->id == 3)
	{
 //       minecraft->setScreen(new TexturePackSelectScreen(this));		// 4J - TODO put back in
    }
    if (button->id == 4)
	{
        minecraft->stop();
    }
}

void TitleScreen::render(int xm, int ym, float a)
{
    // 4J Unused - Iggy Flash UI renders the title screen on consoles
}
