set(PROJECT_NAME Minecraft.Client)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "ClassDiagram.cd"
    "../Platform/Durango/manifest.xml"
#    "Orbis/GameConfig/Minecraft.spa"
    "../ReadMe.txt"
)
source_group("" FILES ${no_group_source_files})

set(Common
    "Common/App_Defines.h"
    "Common/App_enums.h"
    "Common/App_structs.h"
    "Common/Consoles_App.cpp"
    "Common/Consoles_App.h"
    "Common/Potion_Macros.h"
    "Extrax64Stubs.cpp"
)
source_group("Common" FILES ${Common})

set(Common__Source_Files
    "Common/Console_Awards_enum.h"
    "Common/Console_Debug_enum.h"
    "Common/Console_Utils.cpp"
    "Common/ConsoleGameMode.cpp"
    "Common/ConsoleGameMode.h"
)
source_group("Common\\Source Files" FILES ${Common__Source_Files})

set(Common__Source_Files__Audio
    "Common/Audio/Consoles_SoundEngine.cpp"
    "Common/Audio/Consoles_SoundEngine.h"
    "Common/Audio/SoundEngine.cpp"
    "Common/Audio/SoundEngine.h"
    "Common/Audio/SoundNames.cpp"
)
source_group("Common\\Source Files\\Audio" FILES ${Common__Source_Files__Audio})

set(Common__Source_Files__BuildVer
    "Common/BuildVer.h"
)
source_group("Common\\Source Files\\BuildVer" FILES ${Common__Source_Files__BuildVer})

set(Common__Source_Files__Colours
    "Common/Colours/ColourTable.cpp"
    "Common/Colours/ColourTable.h"
)
source_group("Common\\Source Files\\Colours" FILES ${Common__Source_Files__Colours})

set(Common__Source_Files__DLC
    "Common/DLC/DLCAudioFile.cpp"
    "Common/DLC/DLCAudioFile.h"
    "Common/DLC/DLCCapeFile.cpp"
    "Common/DLC/DLCCapeFile.h"
    "Common/DLC/DLCColourTableFile.cpp"
    "Common/DLC/DLCColourTableFile.h"
    "Common/DLC/DLCFile.cpp"
    "Common/DLC/DLCFile.h"
    "Common/DLC/DLCGameRules.h"
    "Common/DLC/DLCGameRulesFile.cpp"
    "Common/DLC/DLCGameRulesFile.h"
    "Common/DLC/DLCGameRulesHeader.cpp"
    "Common/DLC/DLCGameRulesHeader.h"
    "Common/DLC/DLCLocalisationFile.cpp"
    "Common/DLC/DLCLocalisationFile.h"
    "Common/DLC/DLCManager.cpp"
    "Common/DLC/DLCManager.h"
    "Common/DLC/DLCPack.cpp"
    "Common/DLC/DLCPack.h"
    "Common/DLC/DLCSkinFile.cpp"
    "Common/DLC/DLCSkinFile.h"
    "Common/DLC/DLCTextureFile.cpp"
    "Common/DLC/DLCTextureFile.h"
    "Common/DLC/DLCUIDataFile.cpp"
    "Common/DLC/DLCUIDataFile.h"
)
source_group("Common\\Source Files\\DLC" FILES ${Common__Source_Files__DLC})

set(Common__Source_Files__GameRules
    "Common/GameRules/ConsoleGameRules.h"
    "Common/GameRules/ConsoleGameRulesConstants.h"
    "Common/GameRules/GameRuleManager.cpp"
    "Common/GameRules/GameRuleManager.h"
    "../Utils/WstringLookup.cpp"
    "../Utils/WstringLookup.h"
)
source_group("Common\\Source Files\\GameRules" FILES ${Common__Source_Files__GameRules})

set(Common__Source_Files__GameRules__LevelGeneration
    "Common/GameRules/ApplySchematicRuleDefinition.cpp"
    "Common/GameRules/ApplySchematicRuleDefinition.h"
    "Common/GameRules/BiomeOverride.cpp"
    "Common/GameRules/BiomeOverride.h"
    "Common/GameRules/ConsoleGenerateStructure.cpp"
    "Common/GameRules/ConsoleGenerateStructure.h"
    "Common/GameRules/ConsoleGenerateStructureAction.h"
    "Common/GameRules/ConsoleSchematicFile.cpp"
    "Common/GameRules/ConsoleSchematicFile.h"
    "Common/GameRules/LevelGenerationOptions.cpp"
    "Common/GameRules/LevelGenerationOptions.h"
    "Common/GameRules/LevelGenerators.cpp"
    "Common/GameRules/LevelGenerators.h"
    "Common/GameRules/StartFeature.cpp"
    "Common/GameRules/StartFeature.h"
)
source_group("Common\\Source Files\\GameRules\\LevelGeneration" FILES ${Common__Source_Files__GameRules__LevelGeneration})

set(Common__Source_Files__GameRules__LevelGeneration__StructureActions
    "Common/GameRules/XboxStructureActionGenerateBox.cpp"
    "Common/GameRules/XboxStructureActionGenerateBox.h"
    "Common/GameRules/XboxStructureActionPlaceBlock.cpp"
    "Common/GameRules/XboxStructureActionPlaceBlock.h"
    "Common/GameRules/XboxStructureActionPlaceContainer.cpp"
    "Common/GameRules/XboxStructureActionPlaceContainer.h"
    "Common/GameRules/XboxStructureActionPlaceSpawner.cpp"
    "Common/GameRules/XboxStructureActionPlaceSpawner.h"
)
source_group("Common\\Source Files\\GameRules\\LevelGeneration\\StructureActions" FILES ${Common__Source_Files__GameRules__LevelGeneration__StructureActions})

set(Common__Source_Files__GameRules__LevelRules
    "Common/GameRules/LevelRules.cpp"
    "Common/GameRules/LevelRules.h"
)
source_group("Common\\Source Files\\GameRules\\LevelRules" FILES ${Common__Source_Files__GameRules__LevelRules})

set(Common__Source_Files__GameRules__LevelRules__RuleDefinitions
    "Common/GameRules/AddEnchantmentRuleDefinition.cpp"
    "Common/GameRules/AddEnchantmentRuleDefinition.h"
    "Common/GameRules/AddItemRuleDefinition.cpp"
    "Common/GameRules/AddItemRuleDefinition.h"
    "Common/GameRules/CollectItemRuleDefinition.cpp"
    "Common/GameRules/CollectItemRuleDefinition.h"
    "Common/GameRules/CompleteAllRuleDefinition.cpp"
    "Common/GameRules/CompleteAllRuleDefinition.h"
    "Common/GameRules/CompoundGameRuleDefinition.cpp"
    "Common/GameRules/CompoundGameRuleDefinition.h"
    "Common/GameRules/GameRuleDefinition.cpp"
    "Common/GameRules/GameRuleDefinition.h"
    "Common/GameRules/LevelRuleset.cpp"
    "Common/GameRules/LevelRuleset.h"
    "Common/GameRules/NamedAreaRuleDefinition.cpp"
    "Common/GameRules/NamedAreaRuleDefinition.h"
    "Common/GameRules/UpdatePlayerRuleDefinition.cpp"
    "Common/GameRules/UpdatePlayerRuleDefinition.h"
    "Common/GameRules/UseTileRuleDefinition.cpp"
    "Common/GameRules/UseTileRuleDefinition.h"
)
source_group("Common\\Source Files\\GameRules\\LevelRules\\RuleDefinitions" FILES ${Common__Source_Files__GameRules__LevelRules__RuleDefinitions})

set(Common__Source_Files__GameRules__LevelRules__Rules
    "Common/GameRules/GameRule.cpp"
    "Common/GameRules/GameRule.h"
    "Common/GameRules/GameRulesInstance.h"
)
source_group("Common\\Source Files\\GameRules\\LevelRules\\Rules" FILES ${Common__Source_Files__GameRules__LevelRules__Rules})

set(Common__Source_Files__Leaderboards
    "Common/Leaderboards/LeaderboardManager.cpp"
    "Common/Leaderboards/LeaderboardManager.h"
)
source_group("Common\\Source Files\\Leaderboards" FILES ${Common__Source_Files__Leaderboards})

set(Common__Source_Files__Localisation
    "../Utils/StringTable.cpp"
    "../Utils/StringTable.h"
)
source_group("Common\\Source Files\\Localisation" FILES ${Common__Source_Files__Localisation})

set(Common__Source_Files__Network
    "Common/Network/GameNetworkManager.cpp"
    "Common/Network/GameNetworkManager.h"
    "Common/Network/NetworkPlayerInterface.h"
    "Common/Network/PlatformNetworkManagerInterface.h"
    "Common/Network/PlatformNetworkManagerStub.cpp"
    "Common/Network/PlatformNetworkManagerStub.h"
    "Common/Network/SessionInfo.h"
    "../Network Implementation Notes.txt"
)
source_group("Common\\Source Files\\Network" FILES ${Common__Source_Files__Network})

set(Common__Source_Files__Network__Sony
#    "Common/Network/Sony/NetworkPlayerSony.cpp"
#    "Common/Network/Sony/NetworkPlayerSony.h"
#    "Common/Network/Sony/PlatformNetworkManagerSony.cpp"
#    "Common/Network/Sony/PlatformNetworkManagerSony.h"
#    "Common/Network/Sony/SonyCommerce.h"
#    "Common/Network/Sony/SonyHttp.cpp"
#    "Common/Network/Sony/SonyHttp.h"
#    "Common/Network/Sony/SonyRemoteStorage.cpp"
#    "Common/Network/Sony/SonyRemoteStorage.h"
#    "Common/Network/Sony/SQRNetworkManager.cpp"
#    "Common/Network/Sony/SQRNetworkManager.h"
#    "Common/Network/Sony/SQRNetworkPlayer.cpp"
#    "Common/Network/Sony/SQRNetworkPlayer.h"
)
source_group("Common\\Source Files\\Network\\Sony" FILES ${Common__Source_Files__Network__Sony})

set(Common__Source_Files__Telemetry
    "Common/Telemetry/TelemetryManager.cpp"
    "Common/Telemetry/TelemetryManager.h"
)
source_group("Common\\Source Files\\Telemetry" FILES ${Common__Source_Files__Telemetry})

set(Common__Source_Files__Trial
    "Common/Trial/TrialMode.cpp"
    "Common/Trial/TrialMode.h"
)
source_group("Common\\Source Files\\Trial" FILES ${Common__Source_Files__Trial})

set(Common__Source_Files__Tutorial
    "Common/Tutorial/FullTutorial.cpp"
    "Common/Tutorial/FullTutorial.h"
    "Common/Tutorial/FullTutorialMode.cpp"
    "Common/Tutorial/FullTutorialMode.h"
    "Common/Tutorial/Tutorial.cpp"
    "Common/Tutorial/Tutorial.h"
    "Common/Tutorial/TutorialEnum.h"
    "Common/Tutorial/TutorialMessage.cpp"
    "Common/Tutorial/TutorialMessage.h"
    "Common/Tutorial/TutorialMode.cpp"
    "Common/Tutorial/TutorialMode.h"
)
source_group("Common\\Source Files\\Tutorial" FILES ${Common__Source_Files__Tutorial})

set(Common__Source_Files__Tutorial__Constraints
    "Common/Tutorial/AreaConstraint.cpp"
    "Common/Tutorial/AreaConstraint.h"
    "Common/Tutorial/ChangeStateConstraint.cpp"
    "Common/Tutorial/ChangeStateConstraint.h"
    "Common/Tutorial/InputConstraint.cpp"
    "Common/Tutorial/InputConstraint.h"
    "Common/Tutorial/TutorialConstraint.h"
    "Common/Tutorial/TutorialConstraints.h"
)
source_group("Common\\Source Files\\Tutorial\\Constraints" FILES ${Common__Source_Files__Tutorial__Constraints})

set(Common__Source_Files__Tutorial__Hints
    "Common/Tutorial/AreaHint.cpp"
    "Common/Tutorial/AreaHint.h"
    "Common/Tutorial/DiggerItemHint.cpp"
    "Common/Tutorial/DiggerItemHint.h"
    "Common/Tutorial/LookAtEntityHint.cpp"
    "Common/Tutorial/LookAtEntityHint.h"
    "Common/Tutorial/LookAtTileHint.cpp"
    "Common/Tutorial/LookAtTileHint.h"
    "Common/Tutorial/TakeItemHint.cpp"
    "Common/Tutorial/TakeItemHint.h"
    "Common/Tutorial/TutorialHint.cpp"
    "Common/Tutorial/TutorialHint.h"
    "Common/Tutorial/TutorialHints.h"
)
source_group("Common\\Source Files\\Tutorial\\Hints" FILES ${Common__Source_Files__Tutorial__Hints})

set(Common__Source_Files__Tutorial__Tasks
    "Common/Tutorial/AreaTask.cpp"
    "Common/Tutorial/AreaTask.h"
    "Common/Tutorial/ChoiceTask.cpp"
    "Common/Tutorial/ChoiceTask.h"
    "Common/Tutorial/CompleteUsingItemTask.cpp"
    "Common/Tutorial/CompleteUsingItemTask.h"
    "Common/Tutorial/ControllerTask.cpp"
    "Common/Tutorial/ControllerTask.h"
    "Common/Tutorial/CraftTask.cpp"
    "Common/Tutorial/CraftTask.h"
    "Common/Tutorial/EffectChangedTask.cpp"
    "Common/Tutorial/EffectChangedTask.h"
    "Common/Tutorial/FullTutorialActiveTask.cpp"
    "Common/Tutorial/FullTutorialActiveTask.h"
    "Common/Tutorial/InfoTask.cpp"
    "Common/Tutorial/InfoTask.h"
    "Common/Tutorial/PickupTask.cpp"
    "Common/Tutorial/PickupTask.h"
    "Common/Tutorial/ProcedureCompoundTask.cpp"
    "Common/Tutorial/ProcedureCompoundTask.h"
    "Common/Tutorial/ProgressFlagTask.cpp"
    "Common/Tutorial/ProgressFlagTask.h"
    "Common/Tutorial/StateChangeTask.h"
    "Common/Tutorial/StatTask.cpp"
    "Common/Tutorial/StatTask.h"
    "Common/Tutorial/TutorialTask.cpp"
    "Common/Tutorial/TutorialTask.h"
    "Common/Tutorial/TutorialTasks.h"
    "Common/Tutorial/UseItemTask.cpp"
    "Common/Tutorial/UseItemTask.h"
    "Common/Tutorial/UseTileTask.cpp"
    "Common/Tutorial/UseTileTask.h"
    "Common/Tutorial/XuiCraftingTask.cpp"
    "Common/Tutorial/XuiCraftingTask.h"
)
source_group("Common\\Source Files\\Tutorial\\Tasks" FILES ${Common__Source_Files__Tutorial__Tasks})

set(Common__Source_Files__UI
    "Common/UI/UI.h"
    "Common/UI/UIBitmapFont.cpp"
    "Common/UI/UIBitmapFont.h"
    "Common/UI/UIController.cpp"
    "Common/UI/UIController.h"
    "Common/UI/UIFontData.cpp"
    "Common/UI/UIFontData.h"
    "Common/UI/UIGroup.cpp"
    "Common/UI/UIGroup.h"
    "Common/UI/UILayer.cpp"
    "Common/UI/UILayer.h"
    "Common/UI/UIScene.cpp"
    "Common/UI/UIScene.h"
    "Common/UI/UITTFFont.cpp"
    "Common/UI/UITTFFont.h"
)
source_group("Common\\Source Files\\UI" FILES ${Common__Source_Files__UI})

set(Common__Source_Files__UI__All_Platforms
    "../Utils/ArchiveFile.cpp"
    "../Utils/ArchiveFile.h"
    "Common/UI/IUIController.h"
    "Common/UI/IUIScene_AbstractContainerMenu.cpp"
    "Common/UI/IUIScene_AbstractContainerMenu.h"
    "Common/UI/IUIScene_AnvilMenu.cpp"
    "Common/UI/IUIScene_AnvilMenu.h"
    "Common/UI/IUIScene_BrewingMenu.cpp"
    "Common/UI/IUIScene_BrewingMenu.h"
    "Common/UI/IUIScene_ContainerMenu.cpp"
    "Common/UI/IUIScene_ContainerMenu.h"
    "Common/UI/IUIScene_CraftingMenu.cpp"
    "Common/UI/IUIScene_CraftingMenu.h"
    "Common/UI/IUIScene_CreativeMenu.cpp"
    "Common/UI/IUIScene_CreativeMenu.h"
    "Common/UI/IUIScene_DispenserMenu.cpp"
    "Common/UI/IUIScene_DispenserMenu.h"
    "Common/UI/IUIScene_EnchantingMenu.cpp"
    "Common/UI/IUIScene_EnchantingMenu.h"
    "Common/UI/IUIScene_FurnaceMenu.cpp"
    "Common/UI/IUIScene_FurnaceMenu.h"
    "Common/UI/IUIScene_InventoryMenu.cpp"
    "Common/UI/IUIScene_InventoryMenu.h"
    "Common/UI/IUIScene_PauseMenu.cpp"
    "Common/UI/IUIScene_PauseMenu.h"
    "Common/UI/IUIScene_TradingMenu.cpp"
    "Common/UI/IUIScene_TradingMenu.h"
    "Common/UI/UIEnums.h"
    "Common/UI/UIStructs.h"
)
source_group("Common\\Source Files\\UI\\All Platforms" FILES ${Common__Source_Files__UI__All_Platforms})

set(Common__Source_Files__UI__Components
    "Common/UI/UIComponent_Chat.cpp"
    "Common/UI/UIComponent_Chat.h"
    "Common/UI/UIComponent_DebugUIConsole.cpp"
    "Common/UI/UIComponent_DebugUIConsole.h"
    "Common/UI/UIComponent_DebugUIMarketingGuide.cpp"
    "Common/UI/UIComponent_DebugUIMarketingGuide.h"
    "Common/UI/UIComponent_Logo.cpp"
    "Common/UI/UIComponent_Logo.h"
    "Common/UI/UIComponent_MenuBackground.cpp"
    "Common/UI/UIComponent_MenuBackground.h"
    "Common/UI/UIComponent_Panorama.cpp"
    "Common/UI/UIComponent_Panorama.h"
    "Common/UI/UIComponent_PressStartToPlay.cpp"
    "Common/UI/UIComponent_PressStartToPlay.h"
    "Common/UI/UIComponent_Tooltips.cpp"
    "Common/UI/UIComponent_Tooltips.h"
    "Common/UI/UIComponent_TutorialPopup.cpp"
    "Common/UI/UIComponent_TutorialPopup.h"
    "Common/UI/UIScene_HUD.cpp"
    "Common/UI/UIScene_HUD.h"
)
source_group("Common\\Source Files\\UI\\Components" FILES ${Common__Source_Files__UI__Components})

set(Common__Source_Files__UI__Controls
    "Common/UI/UIControl.cpp"
    "Common/UI/UIControl.h"
    "Common/UI/UIControl_Base.cpp"
    "Common/UI/UIControl_Base.h"
    "Common/UI/UIControl_BitmapIcon.cpp"
    "Common/UI/UIControl_BitmapIcon.h"
    "Common/UI/UIControl_Button.cpp"
    "Common/UI/UIControl_Button.h"
    "Common/UI/UIControl_ButtonList.cpp"
    "Common/UI/UIControl_ButtonList.h"
    "Common/UI/UIControl_CheckBox.cpp"
    "Common/UI/UIControl_CheckBox.h"
    "Common/UI/UIControl_Cursor.cpp"
    "Common/UI/UIControl_Cursor.h"
    "Common/UI/UIControl_DLCList.cpp"
    "Common/UI/UIControl_DLCList.h"
    "Common/UI/UIControl_DynamicLabel.cpp"
    "Common/UI/UIControl_DynamicLabel.h"
    "Common/UI/UIControl_EnchantmentBook.cpp"
    "Common/UI/UIControl_EnchantmentBook.h"
    "Common/UI/UIControl_EnchantmentButton.cpp"
    "Common/UI/UIControl_EnchantmentButton.h"
    "Common/UI/UIControl_HTMLLabel.cpp"
    "Common/UI/UIControl_HTMLLabel.h"
    "Common/UI/UIControl_Label.cpp"
    "Common/UI/UIControl_Label.h"
    "Common/UI/UIControl_LeaderboardList.cpp"
    "Common/UI/UIControl_LeaderboardList.h"
    "Common/UI/UIControl_MinecraftPlayer.cpp"
    "Common/UI/UIControl_MinecraftPlayer.h"
    "Common/UI/UIControl_PlayerList.cpp"
    "Common/UI/UIControl_PlayerList.h"
    "Common/UI/UIControl_PlayerSkinPreview.cpp"
    "Common/UI/UIControl_PlayerSkinPreview.h"
    "Common/UI/UIControl_Progress.cpp"
    "Common/UI/UIControl_Progress.h"
    "Common/UI/UIControl_SaveList.cpp"
    "Common/UI/UIControl_SaveList.h"
    "Common/UI/UIControl_Slider.cpp"
    "Common/UI/UIControl_Slider.h"
    "Common/UI/UIControl_SlotList.cpp"
    "Common/UI/UIControl_SlotList.h"
    "Common/UI/UIControl_SpaceIndicatorBar.cpp"
    "Common/UI/UIControl_SpaceIndicatorBar.h"
    "Common/UI/UIControl_TextInput.cpp"
    "Common/UI/UIControl_TextInput.h"
    "Common/UI/UIControl_TexturePackList.cpp"
    "Common/UI/UIControl_TexturePackList.h"
    "Common/UI/UIControl_Touch.cpp"
    "Common/UI/UIControl_Touch.h"
)
source_group("Common\\Source Files\\UI\\Controls" FILES ${Common__Source_Files__UI__Controls})

set(Common__Source_Files__UI__Scenes
    "Common/UI/UIScene_ConnectingProgress.cpp"
    "Common/UI/UIScene_ConnectingProgress.h"
    "Common/UI/UIScene_FullscreenProgress.cpp"
    "Common/UI/UIScene_FullscreenProgress.h"
    "Common/UI/UIScene_Keyboard.cpp"
    "Common/UI/UIScene_Keyboard.h"
    "Common/UI/UIScene_MessageBox.cpp"
    "Common/UI/UIScene_MessageBox.h"
    "Common/UI/UIScene_QuadrantSignin.cpp"
    "Common/UI/UIScene_QuadrantSignin.h"
    "Common/UI/UIScene_Timer.cpp"
    "Common/UI/UIScene_Timer.h"
)
source_group("Common\\Source Files\\UI\\Scenes" FILES ${Common__Source_Files__UI__Scenes})

set(Common__Source_Files__UI__Scenes__Debug
    "Common/UI/UIScene_DebugCreateSchematic.cpp"
    "Common/UI/UIScene_DebugCreateSchematic.h"
    "Common/UI/UIScene_DebugOptions.cpp"
    "Common/UI/UIScene_DebugOptions.h"
    "Common/UI/UIScene_DebugOverlay.cpp"
    "Common/UI/UIScene_DebugOverlay.h"
    "Common/UI/UIScene_DebugSetCamera.cpp"
    "Common/UI/UIScene_DebugSetCamera.h"
)
source_group("Common\\Source Files\\UI\\Scenes\\Debug" FILES ${Common__Source_Files__UI__Scenes__Debug})

set(Common__Source_Files__UI__Scenes__Frontend_Menu_screens
    "Common/UI/IUIScene_StartGame.cpp"
    "Common/UI/IUIScene_StartGame.h"
    "Common/UI/UIScene_CreateWorldMenu.cpp"
    "Common/UI/UIScene_CreateWorldMenu.h"
    "Common/UI/UIScene_DLCMainMenu.cpp"
    "Common/UI/UIScene_DLCMainMenu.h"
    "Common/UI/UIScene_DLCOffersMenu.cpp"
    "Common/UI/UIScene_DLCOffersMenu.h"
    "Common/UI/UIScene_EULA.cpp"
    "Common/UI/UIScene_EULA.h"
    "Common/UI/UIScene_Intro.cpp"
    "Common/UI/UIScene_Intro.h"
    "Common/UI/UIScene_JoinMenu.cpp"
    "Common/UI/UIScene_JoinMenu.h"
    "Common/UI/UIScene_LaunchMoreOptionsMenu.cpp"
    "Common/UI/UIScene_LaunchMoreOptionsMenu.h"
    "Common/UI/UIScene_LeaderboardsMenu.cpp"
    "Common/UI/UIScene_LeaderboardsMenu.h"
    "Common/UI/UIScene_LoadMenu.cpp"
    "Common/UI/UIScene_LoadMenu.h"
    "Common/UI/UIScene_LoadOrJoinMenu.cpp"
    "Common/UI/UIScene_LoadOrJoinMenu.h"
    "Common/UI/UIScene_MainMenu.cpp"
    "Common/UI/UIScene_MainMenu.h"
    "Common/UI/UIScene_SaveMessage.cpp"
    "Common/UI/UIScene_SaveMessage.h"
    "Common/UI/UIScene_TrialExitUpsell.cpp"
    "Common/UI/UIScene_TrialExitUpsell.h"
)
source_group("Common\\Source Files\\UI\\Scenes\\Frontend Menu screens" FILES ${Common__Source_Files__UI__Scenes__Frontend_Menu_screens})

set(Common__Source_Files__UI__Scenes__Help__Options
    "Common/UI/UIScene_ControlsMenu.cpp"
    "Common/UI/UIScene_ControlsMenu.h"
    "Common/UI/UIScene_Credits.cpp"
    "Common/UI/UIScene_Credits.h"
    "Common/UI/UIScene_HelpAndOptionsMenu.cpp"
    "Common/UI/UIScene_HelpAndOptionsMenu.h"
    "Common/UI/UIScene_HowToPlay.cpp"
    "Common/UI/UIScene_HowToPlay.h"
    "Common/UI/UIScene_HowToPlayMenu.cpp"
    "Common/UI/UIScene_HowToPlayMenu.h"
    "Common/UI/UIScene_ReinstallMenu.cpp"
    "Common/UI/UIScene_ReinstallMenu.h"
    "Common/UI/UIScene_SettingsAudioMenu.cpp"
    "Common/UI/UIScene_SettingsAudioMenu.h"
    "Common/UI/UIScene_SettingsControlMenu.cpp"
    "Common/UI/UIScene_SettingsControlMenu.h"
    "Common/UI/UIScene_SettingsGraphicsMenu.cpp"
    "Common/UI/UIScene_SettingsGraphicsMenu.h"
    "Common/UI/UIScene_SettingsMenu.cpp"
    "Common/UI/UIScene_SettingsMenu.h"
    "Common/UI/UIScene_SettingsOptionsMenu.cpp"
    "Common/UI/UIScene_SettingsOptionsMenu.h"
    "Common/UI/UIScene_SettingsUIMenu.cpp"
    "Common/UI/UIScene_SettingsUIMenu.h"
    "Common/UI/UIScene_SkinSelectMenu.cpp"
    "Common/UI/UIScene_SkinSelectMenu.h"
)
source_group("Common\\Source Files\\UI\\Scenes\\Help & Options" FILES ${Common__Source_Files__UI__Scenes__Help__Options})

set(Common__Source_Files__UI__Scenes__In-Game_Menu_Screens
    "Common/UI/UIScene_CraftingMenu.cpp"
    "Common/UI/UIScene_CraftingMenu.h"
    "Common/UI/UIScene_DeathMenu.cpp"
    "Common/UI/UIScene_DeathMenu.h"
    "Common/UI/UIScene_EndPoem.cpp"
    "Common/UI/UIScene_EndPoem.h"
    "Common/UI/UIScene_InGameHostOptionsMenu.cpp"
    "Common/UI/UIScene_InGameHostOptionsMenu.h"
    "Common/UI/UIScene_InGameInfoMenu.cpp"
    "Common/UI/UIScene_InGameInfoMenu.h"
    "Common/UI/UIScene_InGamePlayerOptionsMenu.cpp"
    "Common/UI/UIScene_InGamePlayerOptionsMenu.h"
    "Common/UI/UIScene_InGameSaveManagementMenu.cpp"
    "Common/UI/UIScene_InGameSaveManagementMenu.h"
    "Common/UI/UIScene_PauseMenu.cpp"
    "Common/UI/UIScene_PauseMenu.h"
    "Common/UI/UIScene_SignEntryMenu.cpp"
    "Common/UI/UIScene_SignEntryMenu.h"
    "Common/UI/UIScene_TeleportMenu.cpp"
    "Common/UI/UIScene_TeleportMenu.h"
)
source_group("Common\\Source Files\\UI\\Scenes\\In-Game Menu Screens" FILES ${Common__Source_Files__UI__Scenes__In-Game_Menu_Screens})

set(Common__Source_Files__UI__Scenes__In-Game_Menu_Screens__Containers
    "Common/UI/UIScene_AbstractContainerMenu.cpp"
    "Common/UI/UIScene_AbstractContainerMenu.h"
    "Common/UI/UIScene_AnvilMenu.cpp"
    "Common/UI/UIScene_AnvilMenu.h"
    "Common/UI/UIScene_BrewingStandMenu.cpp"
    "Common/UI/UIScene_BrewingStandMenu.h"
    "Common/UI/UIScene_ContainerMenu.cpp"
    "Common/UI/UIScene_ContainerMenu.h"
    "Common/UI/UIScene_CreativeMenu.cpp"
    "Common/UI/UIScene_CreativeMenu.h"
    "Common/UI/UIScene_DispenserMenu.cpp"
    "Common/UI/UIScene_DispenserMenu.h"
    "Common/UI/UIScene_EnchantingMenu.cpp"
    "Common/UI/UIScene_EnchantingMenu.h"
    "Common/UI/UIScene_FurnaceMenu.cpp"
    "Common/UI/UIScene_FurnaceMenu.h"
    "Common/UI/UIScene_InventoryMenu.cpp"
    "Common/UI/UIScene_InventoryMenu.h"
    "Common/UI/UIScene_TradingMenu.cpp"
    "Common/UI/UIScene_TradingMenu.h"
)
source_group("Common\\Source Files\\UI\\Scenes\\In-Game Menu Screens\\Containers" FILES ${Common__Source_Files__UI__Scenes__In-Game_Menu_Screens__Containers})

set(Common__Source_Files__zlib
#    "Common/zlib/adler32.c"
#    "Common/zlib/compress.c"
#    "Common/zlib/crc32.c"
#    "Common/zlib/crc32.h"
#    "Common/zlib/deflate.c"
#    "Common/zlib/deflate.h"
#    "Common/zlib/gzclose.c"
#    "Common/zlib/gzguts.h"
#    "Common/zlib/gzlib.c"
#    "Common/zlib/gzread.c"
#    "Common/zlib/gzwrite.c"
#    "Common/zlib/infback.c"
#    "Common/zlib/inffast.c"
#    "Common/zlib/inffast.h"
#    "Common/zlib/inffixed.h"
#    "Common/zlib/inflate.c"
#    "Common/zlib/inflate.h"
#    "Common/zlib/inftrees.c"
#    "Common/zlib/inftrees.h"
#    "Common/zlib/trees.c"
#    "Common/zlib/trees.h"
#    "Common/zlib/uncompr.c"
#    "Common/zlib/zconf.h"
#    "Common/zlib/zlib.h"
#    "Common/zlib/zutil.c"
#    "Common/zlib/zutil.h"
)
source_group("Common\\Source Files\\zlib" FILES ${Common__Source_Files__zlib})

set(Durango
#    "Durango/Durango_App.cpp"
#    "Durango/Durango_App.h"
#    "Durango/Durango_UIController.cpp"
#    "Durango/Durango_UIController.h"
#    "Durango/Resource.h"
#    "Durango/SmallLogo.png"
#    "Durango/SplashScreen.png"
#    "Durango/StoreLogo.png"
)
source_group("Durango" FILES ${Durango})

set(Durango__4JLibs__inc
#    "Durango/4JLibs/inc/4J_Input.h"
#    "Durango/4JLibs/inc/4J_Profile.h"
#    "Durango/4JLibs/inc/4J_Render.h"
#    "Durango/4JLibs/inc/4J_Storage.h"
)
source_group("Durango\\4JLibs\\inc" FILES ${Durango__4JLibs__inc})

set(Durango__DurangoExtras
#    "Durango/DurangoExtras/DurangoStubs.cpp"
#    "Durango/DurangoExtras/DurangoStubs.h"
)
source_group("Durango\\DurangoExtras" FILES ${Durango__DurangoExtras})

set(Durango__Iggy__gdraw
#    "Durango/Iggy/gdraw/gdraw_d3d10_shaders.inl"
#    "Durango/Iggy/gdraw/gdraw_d3d11.cpp"
#    "Durango/Iggy/gdraw/gdraw_d3d11.h"
#    "Durango/Iggy/gdraw/gdraw_d3d1x_shared.inl"
#    "Durango/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Durango\\Iggy\\gdraw" FILES ${Durango__Iggy__gdraw})

set(Durango__Iggy__include
#    "Durango/Iggy/include/gdraw.h"
#    "Durango/Iggy/include/iggy.h"
#    "Durango/Iggy/include/iggyexpruntime.h"
#    "Durango/Iggy/include/iggyperfmon.h"
#    "Durango/Iggy/include/rrCore.h"
)
source_group("Durango\\Iggy\\include" FILES ${Durango__Iggy__include})

set(Durango__Miles_Sound_System__include
#    "Durango/Miles/include/mss.h"
#    "Durango/Miles/include/rrCore.h"
)
source_group("Durango\\Miles Sound System\\include" FILES ${Durango__Miles_Sound_System__include})

set(Durango__Network
#    "Durango/Network/base64.cpp"
#    "Durango/Network/base64.h"
#    "Durango/Network/ChatIntegrationLayer.cpp"
#    "Durango/Network/ChatIntegrationLayer.h"
#    "Durango/Network/DQRNetworkManager.cpp"
#    "Durango/Network/DQRNetworkManager.h"
#    "Durango/Network/DQRNetworkManager_FriendSessions.cpp"
#    "Durango/Network/DQRNetworkManager_Log.cpp"
#    "Durango/Network/DQRNetworkManager_SendReceive.cpp"
#    "Durango/Network/DQRNetworkManager_XRNSEvent.cpp"
#    "Durango/Network/DQRNetworkPlayer.cpp"
#    "Durango/Network/DQRNetworkPlayer.h"
#    "Durango/Network/NetworkPlayerDurango.cpp"
#    "Durango/Network/NetworkPlayerDurango.h"
#    "Durango/Network/PartyController.cpp"
#    "Durango/Network/PartyController.h"
#    "Durango/Network/PlatformNetworkManagerDurango.cpp"
#    "Durango/Network/PlatformNetworkManagerDurango.h"
)
source_group("Durango\\Network" FILES ${Durango__Network})

set(Durango__ServiceConfig
    "../Platform/Durango/ServiceConfig/Events-XBLA.8-149E11AEEvents.h"
)
source_group("Durango\\ServiceConfig" FILES ${Durango__ServiceConfig})

set(Durango__Source_Files
#    "Durango/ApplicationView.cpp"
#    "Durango/ApplicationView.h"
#    "Durango/Durango_Minecraft.cpp"
#    "Durango/Minecraft_Macros.h"
    "../Platform/Durango/PresenceIds.h"
)
source_group("Durango\\Source Files" FILES ${Durango__Source_Files})

set(Durango__Source_Files__Achievements
#    "Durango/Achievements/AchievementManager.cpp"
    "../Platform/Durango/Achievements/AchievementManager.h"
)
source_group("Durango\\Source Files\\Achievements" FILES ${Durango__Source_Files__Achievements})

set(Durango__Source_Files__Leaderboards
#    "Durango/Leaderboards/DurangoLeaderboardManager.cpp"
#    "Durango/Leaderboards/DurangoLeaderboardManager.h"
#    "Durango/Leaderboards/DurangoStatsDebugger.cpp"
#    "Durango/Leaderboards/DurangoStatsDebugger.h"
#    "Durango/Leaderboards/GameProgress.cpp"
#    "Durango/Leaderboards/GameProgress.h"
)
source_group("Durango\\Source Files\\Leaderboards" FILES ${Durango__Source_Files__Leaderboards})

set(Durango__Source_Files__Sentient
#    "Durango/Sentient/DurangoTelemetry.cpp"
#    "Durango/Sentient/DurangoTelemetry.h"
#    "Durango/Sentient/DynamicConfigurations.h"
#    "Durango/Sentient/MinecraftTelemetry.h"
#    "Durango/Sentient/SentientManager.h"
#    "Durango/Sentient/SentientStats.h"
#    "Durango/Sentient/SentientTelemetryCommon.h"
#    "Durango/Sentient/TelemetryEnum.h"
)
source_group("Durango\\Source Files\\Sentient" FILES ${Durango__Source_Files__Sentient})

set(Durango__Source_Files__Social
#    "Durango/Social/SocialManager.h"
)
source_group("Durango\\Source Files\\Social" FILES ${Durango__Source_Files__Social})

set(Durango__XML
#    "Durango/XML/ATGXmlParser.cpp"
#    "Durango/XML/ATGXmlParser.h"
#    "Durango/XML/xmlFilesCallback.h"
)
source_group("Durango\\XML" FILES ${Durango__XML})

set(Header_Files
    "../Textures/BufferedImage.h"
    "extraX64client.h"
    "../Textures/MemTexture.h"
    "../Textures/MemTextureProcessor.h"
    "../Textures/MobSkinMemTextureProcessor.h"
#    "Orbis/GameConfig/Minecraft.spa.h"
    "../Rendering/Models/SkinBox.h"
    "stdafx.h"
    "stubs.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Orbis
#    "Orbis/Orbis_App.cpp"
#    "Orbis/Orbis_App.h"
#    "Orbis/Orbis_PlayerUID.cpp"
#    "Orbis/Orbis_PlayerUID.h"
#    "Orbis/Orbis_UIController.cpp"
#    "Orbis/Orbis_UIController.h"
#    "Orbis/user_malloc.cpp"
#    "Orbis/user_malloc_for_tls.cpp"
#    "Orbis/user_new.cpp"
)
source_group("Orbis" FILES ${Orbis})

set(Orbis__4JLibs__inc
    "../Platform/Orbis/4JLibs/inc/4J_Input.h"
    "../Platform/Orbis/4JLibs/inc/4J_Profile.h"
    "../Platform/Orbis/4JLibs/inc/4J_Render.h"
    "../Platform/Orbis/4JLibs/inc/4J_Storage.h"
)
source_group("Orbis\\4JLibs\\inc" FILES ${Orbis__4JLibs__inc})

set(Orbis__4JLibs__libs
#    "Orbis/4JLibs/Libs/4J_Input.a"
#    "Orbis/4JLibs/Libs/4J_Input_d.a"
#    "Orbis/4JLibs/Libs/4J_Input_r.a"
#    "Orbis/4JLibs/Libs/4J_Render.a"
#    "Orbis/4JLibs/Libs/4J_Render_d.a"
)
source_group("Orbis\\4JLibs\\libs" FILES ${Orbis__4JLibs__libs})

set(Orbis__Iggy__gdraw
#    "Orbis/Iggy/gdraw/gdraw_orbis.cpp"
#    "Orbis/Iggy/gdraw/gdraw_orbis.h"
#    "Orbis/Iggy/gdraw/gdraw_orbis_shaders.inl"
#    "Orbis/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Orbis\\Iggy\\gdraw" FILES ${Orbis__Iggy__gdraw})

set(Orbis__Iggy__include
#    "Orbis/Iggy/include/gdraw.h"
#    "Orbis/Iggy/include/iggy.h"
#    "Orbis/Iggy/include/iggyexpruntime.h"
#    "Orbis/Iggy/include/iggyperfmon.h"
#    "Orbis/Iggy/include/iggyperfmon_orbis.h"
#    "Orbis/Iggy/include/rrCore.h"
)
source_group("Orbis\\Iggy\\include" FILES ${Orbis__Iggy__include})

set(Orbis__Miles_Sound_System__include
#    "Orbis/Miles/include/mss.h"
#    "Orbis/Miles/include/rrCore.h"
)
source_group("Orbis\\Miles Sound System\\include" FILES ${Orbis__Miles_Sound_System__include})

set(Orbis__Miles_Sound_System__lib
#    "Orbis/Miles/lib/mssorbis.a"
)
source_group("Orbis\\Miles Sound System\\lib" FILES ${Orbis__Miles_Sound_System__lib})

set(Orbis__Network
#    "Orbis/Network/Orbis_NPToolkit.cpp"
#    "Orbis/Network/Orbis_NPToolkit.h"
#    "Orbis/Network/PsPlusUpsellWrapper_Orbis.cpp"
#    "Orbis/Network/PsPlusUpsellWrapper_Orbis.h"
#    "Orbis/Network/SonyCommerce_Orbis.cpp"
#    "Orbis/Network/SonyCommerce_Orbis.h"
#    "Orbis/Network/SonyHttp_Orbis.cpp"
#    "Orbis/Network/SonyHttp_Orbis.h"
#    "Orbis/Network/SonyRemoteStorage_Orbis.cpp"
#    "Orbis/Network/SonyRemoteStorage_Orbis.h"
#    "Orbis/Network/SonyVoiceChat_Orbis.cpp"
#    "Orbis/Network/SonyVoiceChat_Orbis.h"
#    "Orbis/Network/SQRNetworkManager_Orbis.cpp"
#    "Orbis/Network/SQRNetworkManager_Orbis.h"
)
source_group("Orbis\\Network" FILES ${Orbis__Network})

set(Orbis__OrbisExtras
#    "Orbis/OrbisExtras/OrbisMaths.h"
#    "Orbis/OrbisExtras/OrbisStubs.cpp"
#    "Orbis/OrbisExtras/OrbisStubs.h"
#    "Orbis/OrbisExtras/OrbisTypes.h"
#    "Orbis/OrbisExtras/TLSStorage.cpp"
#    "Orbis/OrbisExtras/TLSStorage.h"
#    "Orbis/OrbisExtras/winerror.h"
)
source_group("Orbis\\OrbisExtras" FILES ${Orbis__OrbisExtras})

set(Orbis__Source_Files
#    "Orbis/Minecraft_Macros.h"
#    "Orbis/Orbis_Minecraft.cpp"
#    "Orbis/ps4__np_conf.h"
)
source_group("Orbis\\Source Files" FILES ${Orbis__Source_Files})

set(Orbis__Source_Files__Leaderboards
#    "Orbis/Leaderboards/base64.cpp"
#    "Orbis/Leaderboards/base64.h"
#    "Orbis/Leaderboards/OrbisLeaderboardManager.cpp"
#    "Orbis/Leaderboards/OrbisLeaderboardManager.h"
)
source_group("Orbis\\Source Files\\Leaderboards" FILES ${Orbis__Source_Files__Leaderboards})

set(Orbis__Source_Files__Sentient
#    "Orbis/Sentient/DynamicConfigurations.h"
#    "Orbis/Sentient/MinecraftTelemetry.h"
#    "Orbis/Sentient/SentientManager.h"
#    "Orbis/Sentient/SentientStats.h"
#    "Orbis/Sentient/SentientTelemetryCommon.h"
#    "Orbis/Sentient/TelemetryEnum.h"
)
source_group("Orbis\\Source Files\\Sentient" FILES ${Orbis__Source_Files__Sentient})

set(Orbis__Source_Files__Social
#    "Orbis/Social/SocialManager.h"
)
source_group("Orbis\\Source Files\\Social" FILES ${Orbis__Source_Files__Social})

set(Orbis__XML
#    "Orbis/XML/ATGXmlParser.h"
)
source_group("Orbis\\XML" FILES ${Orbis__XML})

set(PS3
#    "PS3/Passphrase/ps3__np_conf.h"
#    "PS3/PS3_App.cpp"
#    "PS3/PS3_App.h"
#    "PS3/PS3_UIController.cpp"
#    "PS3/PS3_UIController.h"
#    "PS3/PS3Extras/C4JSpursJob.cpp"
#    "PS3/PS3Extras/C4JSpursJob.h"
)
source_group("PS3" FILES ${PS3})

set(PS3__4JLibs
#    "PS3/4JLibs/STO_TitleSmallStorage.cpp"
#    "PS3/4JLibs/STO_TitleSmallStorage.h"
)
source_group("PS3\\4JLibs" FILES ${PS3__4JLibs})

set(PS3__4JLibs__inc
#    "PS3/4JLibs/inc/4J_Input.h"
#    "PS3/4JLibs/inc/4J_Profile.h"
#    "PS3/4JLibs/inc/4J_Render.h"
#    "PS3/4JLibs/inc/4J_Storage.h"
)
source_group("PS3\\4JLibs\\inc" FILES ${PS3__4JLibs__inc})

set(PS3__ChunkRebuild_SPU
#    "PS3/SPU_Tasks/ChunkUpdate/BedTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/BookshelfTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/BrewingStandTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Bush_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/ButtonTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/CactusTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/CakeTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/CauldronTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/ChestTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/ChunkRebuildData.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/ChunkRebuildData.h"
#    "PS3/SPU_Tasks/ChunkUpdate/CocoaTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/CropTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/DetectorRailTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/DiodeTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/DiodeTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Direction_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/Direction_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/DirectionalTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/DirtTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/DispenserTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/DoorTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/DoorTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/EggTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/EnchantmentTableTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/EntityTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Facing_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/Facing_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/FarmTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/FenceGateTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/FenceTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/FenceTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/FireTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/FurnaceTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/GlassTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/GrassTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/GrassTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/HalfSlabTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/HalfSlabTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/HalfTransparentTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/HugeMushroomTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/IceTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Icon_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/Icon_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/LadderTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/LeafTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/LeafTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/LeverTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/LiquidTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/LiquidTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Material_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/MelonTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Mushroom_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/MycelTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/NetherStalkTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/PistonBaseTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/PistonExtensionTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/PistonMovingPiece_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/PortalTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/PressurePlateTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/PressurePlateTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/PumpkinTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/RailTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/RecordPlayerTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/RedlightTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/RedStoneDustTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/ReedTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/SandStoneTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Sapling_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/SignTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/SmoothStoneBrickTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/StairTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/StairTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/stdafx.h"
#    "PS3/SPU_Tasks/ChunkUpdate/StemTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/StoneMonsterTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/stubs_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TallGrass_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/TallGrass_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Tesselator_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/Tesselator_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TheEndPortal_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TheEndPortalFrameTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/ThinFenceTile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/ThinFenceTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/Tile_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/Tile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TileRenderer_SPU.cpp"
#    "PS3/SPU_Tasks/ChunkUpdate/TileRenderer_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TntTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TopSnowTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TorchTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TrapDoorTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/TreeTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/VineTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/WaterLilyTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/WebTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/WoodTile_SPU.h"
#    "PS3/SPU_Tasks/ChunkUpdate/WorkbenchTile_SPU.h"
)
source_group("PS3\\ChunkRebuild_SPU" FILES ${PS3__ChunkRebuild_SPU})

set(PS3__CompressedTile_SPU
#    "PS3/SPU_Tasks/CompressedTile/CompressedTileStorage_SPU.cpp"
#    "PS3/SPU_Tasks/CompressedTile/CompressedTileStorage_SPU.h"
)
source_group("PS3\\CompressedTile_SPU" FILES ${PS3__CompressedTile_SPU})

set(PS3__Iggy__gdraw
#    "PS3/Iggy/gdraw/gdraw_ps3gcm.cpp"
#    "PS3/Iggy/gdraw/gdraw_ps3gcm.h"
#    "PS3/Iggy/gdraw/gdraw_ps3gcm_shaders.inl"
#    "PS3/Iggy/gdraw/gdraw_shared.inl"
)
source_group("PS3\\Iggy\\gdraw" FILES ${PS3__Iggy__gdraw})

set(PS3__Iggy__include
#    "PS3/Iggy/include/gdraw.h"
#    "PS3/Iggy/include/iggy.h"
#    "PS3/Iggy/include/iggyexpruntime.h"
#    "PS3/Iggy/include/iggyperfmon.h"
#    "PS3/Iggy/include/iggyperfmon_ps3.h"
#    "PS3/Iggy/include/rrCore.h"
)
source_group("PS3\\Iggy\\include" FILES ${PS3__Iggy__include})

set(PS3__Miles_Sound_System__include
#    "PS3/Miles/include/mss.h"
#    "PS3/Miles/include/rrCore.h"
)
source_group("PS3\\Miles Sound System\\include" FILES ${PS3__Miles_Sound_System__include})

set(PS3__Miles_Sound_System__lib
#    "PS3/Miles/lib/audps3.a"
#    "PS3/Miles/lib/fltps3.a"
#    "PS3/Miles/lib/mssps3.a"
)
source_group("PS3\\Miles Sound System\\lib" FILES ${PS3__Miles_Sound_System__lib})

set(PS3__Miles_Sound_System__lib__spu
#    "PS3/Miles/lib/spu/binkaspu.a"
#    "PS3/Miles/lib/spu/mssppu_raw.a"
#    "PS3/Miles/lib/spu/mssppu_spurs.a"
#    "PS3/Miles/lib/spu/mssppu_sputhreads.a"
#    "PS3/Miles/lib/spu/mssspu.a"
#    "PS3/Miles/lib/spu/mssspu_raw.a"
#    "PS3/Miles/lib/spu/mssspu_spurs.a"
#    "PS3/Miles/lib/spu/mssspu_sputhreads.a"
)
source_group("PS3\\Miles Sound System\\lib\\spu" FILES ${PS3__Miles_Sound_System__lib__spu})

set(PS3__PS3Extras
#    "PS3/PS3Extras/C4JThread_SPU.cpp"
#    "PS3/PS3Extras/C4JThread_SPU.h"
#    "PS3/PS3Extras/EdgeZLib.cpp"
#    "PS3/PS3Extras/EdgeZLib.h"
#    "PS3/PS3Extras/PS3Maths.h"
#    "PS3/PS3Extras/PS3Strings.cpp"
#    "PS3/PS3Extras/PS3Strings.h"
#    "PS3/PS3Extras/Ps3Stubs.cpp"
#    "PS3/PS3Extras/Ps3Stubs.h"
#    "PS3/PS3Extras/Ps3Types.h"
     "../Platform/PS3/PS3Extras/ShutdownManager.cpp"
    "../Platform/PS3/PS3Extras/ShutdownManager.h"
#    "PS3/PS3Extras/TLSStorage.cpp"
#    "PS3/PS3Extras/TLSStorage.h"
#    "PS3/PS3Extras/winerror.h"
)
source_group("PS3\\PS3Extras" FILES ${PS3__PS3Extras})

set(PS3__Source_Files
#    "PS3/Minecraft_Macros.h"
#    "PS3/PS3_Minecraft.cpp"
#    "PS3/PS3_PlayerUID.cpp"
#    "PS3/PS3_PlayerUID.h"
)
source_group("PS3\\Source Files" FILES ${PS3__Source_Files})

set(PS3__Source_Files__Audio
#    "PS3/Audio/PS3_SoundEngine.cpp"
)
source_group("PS3\\Source Files\\Audio" FILES ${PS3__Source_Files__Audio})

set(PS3__Source_Files__Leaderboards
#    "PS3/Leaderboards/base64.cpp"
#    "PS3/Leaderboards/base64.h"
#    "PS3/Leaderboards/PS3LeaderboardManager.cpp"
#    "PS3/Leaderboards/PS3LeaderboardManager.h"
)
source_group("PS3\\Source Files\\Leaderboards" FILES ${PS3__Source_Files__Leaderboards})

set(PS3__Source_Files__Network
#    "PS3/Network/SonyCommerce_PS3.cpp"
#    "PS3/Network/SonyCommerce_PS3.h"
#    "PS3/Network/SonyHttp_PS3.cpp"
#    "PS3/Network/SonyHttp_PS3.h"
#    "PS3/Network/SonyRemoteStorage_PS3.cpp"
#    "PS3/Network/SonyRemoteStorage_PS3.h"
#    "PS3/Network/SonyVoiceChat.cpp"
#    "PS3/Network/SonyVoiceChat.h"
#    "PS3/Network/SQRNetworkManager_PS3.cpp"
#    "PS3/Network/SQRNetworkManager_PS3.h"
)
source_group("PS3\\Source Files\\Network" FILES ${PS3__Source_Files__Network})

set(PS3__Source_Files__Sentient
#    "PS3/Sentient/DynamicConfigurations.h"
#    "PS3/Sentient/MinecraftTelemetry.h"
#    "PS3/Sentient/SentientManager.h"
#    "PS3/Sentient/SentientStats.h"
#    "PS3/Sentient/SentientTelemetryCommon.h"
#    "PS3/Sentient/TelemetryEnum.h"
)
source_group("PS3\\Source Files\\Sentient" FILES ${PS3__Source_Files__Sentient})

set(PS3__Source_Files__Social
#    "PS3/Social/SocialManager.h"
)
source_group("PS3\\Source Files\\Social" FILES ${PS3__Source_Files__Social})


set(Linux
    "../Platform/Linux/Linux_UIController.cpp"
    "../Platform/Linux/Linux_Minecraft.cpp"
    "../Platform/Linux/Linux_App.cpp"
    "../Platform/Linux/LinuxGL.cpp"
    "../Platform/Linux/linux_game_stubs.cpp"
    "../Platform/Linux/Iggy/gdraw/gdraw_glfw.c"
    "../Platform/Linux/Iggy/gdraw/gdraw_glfw.h"
)
source_group("Linux" FILES ${Linux})

set(PSVita
#    "PSVita/PSVita_App.cpp"
    "../Platform/PSVita/PSVita_App.h"
#    "PSVita/PSVita_UIController.cpp"
#    "PSVita/PSVita_UIController.h"
)
source_group("PSVita" FILES ${PSVita})

set(PSVita__4JLibs__inc
#    "PSVita/4JLibs/inc/4J_Input.h"
#    "PSVita/4JLibs/inc/4J_Profile.h"
#    "PSVita/4JLibs/inc/4J_Render.h"
#    "PSVita/4JLibs/inc/4J_Storage.h"
)
source_group("PSVita\\4JLibs\\inc" FILES ${PSVita__4JLibs__inc})

set(PSVita__GameConfig
    "../Platform/PSVita/GameConfig/Minecraft.gameconfig"
    "../Platform/PSVita/GameConfig/Minecraft.spa"
    "../Platform/PSVita/GameConfig/Minecraft.spa.h"
)
source_group("PSVita\\GameConfig" FILES ${PSVita__GameConfig})

set(PSVita__Iggy__gdraw
#    "PSVita/Iggy/gdraw/gdraw_psp2.cpp"
#    "PSVita/Iggy/gdraw/gdraw_psp2.h"
#    "PSVita/Iggy/gdraw/gdraw_psp2_shaders.inl"
#    "PSVita/Iggy/gdraw/gdraw_shared.inl"
)
source_group("PSVita\\Iggy\\gdraw" FILES ${PSVita__Iggy__gdraw})

set(PSVita__Iggy__include
#    "PSVita/Iggy/include/gdraw.h"
#    "PSVita/Iggy/include/iggy.h"
#    "PSVita/Iggy/include/iggyexpruntime.h"
#    "PSVita/Iggy/include/iggyperfmon.h"
#    "PSVita/Iggy/include/iggyperfmon_psp2.h"
#    "PSVita/Iggy/include/rrCore.h"
)
source_group("PSVita\\Iggy\\include" FILES ${PSVita__Iggy__include})

set(PSVita__Miles_Sound_System__Include
    "../Platform/PSVita/Miles/include/mss.h"
    "../Platform/PSVita/Miles/include/rrCore.h"
)
source_group("PSVita\\Miles Sound System\\Include" FILES ${PSVita__Miles_Sound_System__Include})

set(PSVita__PSVitaExtras
#    "PSVita/PSVitaExtras/Conf.h"
#    "PSVita/PSVitaExtras/CustomMap.cpp"
#    "PSVita/PSVitaExtras/CustomMap.h"
#    "PSVita/PSVitaExtras/CustomSet.cpp"
#    "PSVita/PSVitaExtras/CustomSet.h"
#    "PSVita/PSVitaExtras/libdivide.h"
#    "PSVita/PSVitaExtras/PSVitaMaths.h"
#    "PSVita/PSVitaExtras/PSVitaStrings.cpp"
#    "PSVita/PSVitaExtras/PSVitaStrings.h"
#    "PSVita/PSVitaExtras/PsVitaStubs.cpp"
#    "PSVita/PSVitaExtras/PSVitaStubs.h"
#    "PSVita/PSVitaExtras/PSVitaTLSStorage.cpp"
#    "PSVita/PSVitaExtras/PSVitaTLSStorage.h"
#    "PSVita/PSVitaExtras/PSVitaTypes.h"
#    "PSVita/PSVitaExtras/ShutdownManager.cpp"
#    "PSVita/PSVitaExtras/ShutdownManager.h"
#    "PSVita/PSVitaExtras/user_malloc.c"
#    "PSVita/PSVitaExtras/user_malloc_for_tls.c"
#    "PSVita/PSVitaExtras/user_new.cpp"
#    "PSVita/PSVitaExtras/zconf.h"
#    "PSVita/PSVitaExtras/zlib.h"
)
source_group("PSVita\\PSVitaExtras" FILES ${PSVita__PSVitaExtras})

set(PSVita__Source_Files
#    "PSVita/PSVita_Minecraft.cpp"
#    "PSVita/PSVita_PlayerUID.cpp"
#    "PSVita/PSVita_PlayerUID.h"
)
source_group("PSVita\\Source Files" FILES ${PSVita__Source_Files})

set(PSVita__Source_Files__Leaderboards
#    "PSVita/Leaderboards/base64.cpp"
#    "PSVita/Leaderboards/base64.h"
#    "PSVita/Leaderboards/PSVitaLeaderboardManager.cpp"
#    "PSVita/Leaderboards/PSVitaLeaderboardManager.h"
)
source_group("PSVita\\Source Files\\Leaderboards" FILES ${PSVita__Source_Files__Leaderboards})

set(PSVita__Source_Files__Network
#    "PSVita/Network/PSVita_NPToolkit.cpp"
#    "PSVita/Network/PSVita_NPToolkit.h"
#    "PSVita/Network/SonyCommerce_Vita.cpp"
#    "PSVita/Network/SonyCommerce_Vita.h"
#    "PSVita/Network/SonyHttp_Vita.cpp"
#    "PSVita/Network/SonyHttp_Vita.h"
#    "PSVita/Network/SonyRemoteStorage_Vita.cpp"
#    "PSVita/Network/SonyRemoteStorage_Vita.h"
#    "PSVita/Network/SonyVoiceChat_Vita.cpp"
#    "PSVita/Network/SonyVoiceChat_Vita.h"
#    "PSVita/Network/SQRNetworkManager_AdHoc_Vita.cpp"
#    "PSVita/Network/SQRNetworkManager_AdHoc_Vita.h"
#    "PSVita/Network/SQRNetworkManager_Vita.cpp"
#    "PSVita/Network/SQRNetworkManager_Vita.h"
)
source_group("PSVita\\Source Files\\Network" FILES ${PSVita__Source_Files__Network})

set(PSVita__Source_Files__Sentient
#    "PSVita/Sentient/DynamicConfigurations.h"
#    "PSVita/Sentient/MinecraftTelemetry.h"
#    "PSVita/Sentient/SentientManager.h"
#    "PSVita/Sentient/SentientStats.h"
#    "PSVita/Sentient/SentientTelemetryCommon.h"
#    "PSVita/Sentient/TelemetryEnum.h"
)
source_group("PSVita\\Source Files\\Sentient" FILES ${PSVita__Source_Files__Sentient})

set(PSVita__Source_Files__Social
    "../Platform/PSVita/Social/SocialManager.h"
)
source_group("PSVita\\Source Files\\Social" FILES ${PSVita__Source_Files__Social})

set(PSVita__XML
    "../Platform/PSVita/XML/ATGXmlParser.h"
)
source_group("PSVita\\XML" FILES ${PSVita__XML})

set(Source_Files
    "../Textures/BufferedImage.cpp"
    "Extrax64Stubs.cpp"
    "../Rendering/glWrapper.cpp"
    "stdafx.cpp"
    "stubs.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(Windows
#    "Xbox/Resource.h"
#    "Xbox/targetver.h"
)
source_group("Windows" FILES ${Windows})

set(Windows64
    "../Platform/Windows64/Resource.h"
#    "Windows64/Windows64_App.cpp"
#    "Windows64/Windows64_App.h"
)
source_group("Windows64" FILES ${Windows64})

set(Windows64__4JLibs__inc
#    "Windows64/4JLibs/inc/4J_Input.h"
#    "Windows64/4JLibs/inc/4J_Profile.h"
#    "Windows64/4JLibs/inc/4J_Render.h"
#    "Windows64/4JLibs/inc/4J_Storage.h"
)
source_group("Windows64\\4JLibs\\inc" FILES ${Windows64__4JLibs__inc})

set(Windows64__GameConfig
#    "Windows64/GameConfig/Minecraft.gameconfig"
#    "Windows64/GameConfig/Minecraft.spa"
#    "Windows64/GameConfig/Minecraft.spa.h"
)
source_group("Windows64\\GameConfig" FILES ${Windows64__GameConfig})

set(Windows64__Iggy__gdraw
    "../Platform/Windows64/Iggy/gdraw/gdraw_d3d10_shaders.inl"
#    "Windows64/Iggy/gdraw/gdraw_d3d11.cpp"
#    "Windows64/Iggy/gdraw/gdraw_d3d11.h"
    "../Platform/Windows64/Iggy/gdraw/gdraw_d3d1x_shared.inl"
    "../Platform/Windows64/Iggy/gdraw/gdraw_gl_shaders.inl"
    "../Platform/Windows64/Iggy/gdraw/gdraw_gl_shared.inl"
    "../Platform/Windows64/Iggy/gdraw/gdraw_shared.inl"
)
source_group("Windows64\\Iggy\\gdraw" FILES ${Windows64__Iggy__gdraw})

set(Windows64__Iggy__include
#    "Windows64/Iggy/include/gdraw.h"
#    "Windows64/Iggy/include/iggy.h"
#    "Windows64/Iggy/include/iggyexpruntime.h"
#    "Windows64/Iggy/include/iggyperfmon.h"
#    "Windows64/Iggy/include/rrCore.h"
)
source_group("Windows64\\Iggy\\include" FILES ${Windows64__Iggy__include})

set(Windows64__Miles_Sound_System__Include
#    "Windows64/Miles/include/mss.h"
#    "Windows64/Miles/include/rrcore.h"
)
source_group("Windows64\\Miles Sound System\\Include" FILES ${Windows64__Miles_Sound_System__Include})

set(Windows64__Source_Files
#    "Windows64/Minecraft_Macros.h"
#    "Windows64/Windows64_Minecraft.cpp"
)
source_group("Windows64\\Source Files" FILES ${Windows64__Source_Files})

set(Windows64__Source_Files__Leaderboards
    "../Platform/Windows64/Leaderboards/WindowsLeaderboardManager.cpp"
    "../Platform/Windows64/Leaderboards/WindowsLeaderboardManager.h"
)
source_group("Windows64\\Source Files\\Leaderboards" FILES ${Windows64__Source_Files__Leaderboards})

set(Windows64__Source_Files__Sentient
#    "Windows64/Sentient/DynamicConfigurations.h"
#    "Windows64/Sentient/MinecraftTelemetry.h"
#    "Windows64/Sentient/SentientManager.h"
#    "Windows64/Sentient/SentientStats.h"
#    "Windows64/Sentient/SentientTelemetryCommon.h"
#    "Windows64/Sentient/TelemetryEnum.h"
)
source_group("Windows64\\Source Files\\Sentient" FILES ${Windows64__Source_Files__Sentient})

set(Windows64__Source_Files__Social
#    "Windows64/Social/SocialManager.h"
)
source_group("Windows64\\Source Files\\Social" FILES ${Windows64__Source_Files__Social})

set(Windows64__XML
#    "Windows64/XML/ATGXmlParser.h"
)
source_group("Windows64\\XML" FILES ${Windows64__XML})

set(Xbox__4JLibs__Media
#    "Xbox/4JLibs/Media/4J_strings.resx"
)
source_group("Xbox\\4JLibs\\Media" FILES ${Xbox__4JLibs__Media})

set(Xbox__4JLibs__inc
#    "Xbox/4JLibs/inc/4J_Input.h"
#    "Xbox/4JLibs/inc/4J_Profile.h"
#    "Xbox/4JLibs/inc/4J_Render.h"
#    "Xbox/4JLibs/inc/4J_Storage.h"
    "../Platform/Xbox/4JLibs/inc/4J_xtms.h"
)
source_group("Xbox\\4JLibs\\inc" FILES ${Xbox__4JLibs__inc})

set(Xbox__GameConfig
#    "Xbox/GameConfig/Minecraft.gameconfig"
#    "Xbox/GameConfig/Minecraft.spa"
#    "Xbox/GameConfig/Minecraft.spa.h"
)
source_group("Xbox\\GameConfig" FILES ${Xbox__GameConfig})

set(Xbox__SentientLibs__inc
#    "Xbox/Sentient/Include/SenClientAvatar.h"
#    "Xbox/Sentient/Include/SenClientBoxArt.h"
#    "Xbox/Sentient/Include/SenClientConfig.h"
#    "Xbox/Sentient/Include/SenClientCore.h"
#    "Xbox/Sentient/Include/SenClientCulture.h"
#    "Xbox/Sentient/Include/SenClientCultureBackCompat_SenBoxArt.h"
#    "Xbox/Sentient/Include/SenClientCultureBackCompat_SenClientUGC.h"
#    "Xbox/Sentient/Include/SenClientCultureBackCompat_SenCore.h"
#    "Xbox/Sentient/Include/SenClientCultureBackCompat_SenNews.h"
#    "Xbox/Sentient/Include/SenClientCultureBackCompat_SenSuperstars.h"
#    "Xbox/Sentient/Include/SenClientDynamicConfig.h"
#    "Xbox/Sentient/Include/SenClientFame.h"
#    "Xbox/Sentient/Include/SenClientFile.h"
#    "Xbox/Sentient/Include/SenClientHelp.h"
#    "Xbox/Sentient/Include/SenClientMain.h"
#    "Xbox/Sentient/Include/SenClientMarkers.h"
#    "Xbox/Sentient/Include/SenClientNews.h"
#    "Xbox/Sentient/Include/SenClientPackage.h"
#    "Xbox/Sentient/Include/SenClientRawData.h"
#    "Xbox/Sentient/Include/SenClientResource.h"
#    "Xbox/Sentient/Include/SenClientStats.h"
#    "Xbox/Sentient/Include/SenClientSuperstars.h"
#    "Xbox/Sentient/Include/SenClientSys.h"
#    "Xbox/Sentient/Include/SenClientTypes.h"
#    "Xbox/Sentient/Include/SenClientUGC.h"
#    "Xbox/Sentient/Include/SenClientUGCLeaderboards.h"
#    "Xbox/Sentient/Include/SenClientUGCTypes.h"
#    "Xbox/Sentient/Include/SenClientUser.h"
#    "Xbox/Sentient/Include/SenClientUtil.h"
#    "Xbox/Sentient/Include/SenClientXML.h"
)
source_group("Xbox\\SentientLibs\\inc" FILES ${Xbox__SentientLibs__inc})

set(Xbox__Source_Files
#    "Xbox/Xbox_App.cpp"
#    "Xbox/Xbox_App.h"
#    "Xbox/Xbox_Minecraft.cpp"
#    "Xbox/Xbox_UIController.cpp"
#    "Xbox/Xbox_UIController.h"
)
source_group("Xbox\\Source Files" FILES ${Xbox__Source_Files})

set(Xbox__Source_Files__Audio
#    "Xbox/Audio/SoundEngine.cpp"
#    "Xbox/Audio/SoundEngine.h"
)
source_group("Xbox\\Source Files\\Audio" FILES ${Xbox__Source_Files__Audio})

set(Xbox__Source_Files__Font
#    "Xbox/Font/XUI_Font.cpp"
#    "Xbox/Font/XUI_Font.h"
#    "Xbox/Font/XUI_FontData.cpp"
#    "Xbox/Font/XUI_FontData.h"
#    "Xbox/Font/XUI_FontRenderer.cpp"
#    "Xbox/Font/XUI_FontRenderer.h"
)
source_group("Xbox\\Source Files\\Font" FILES ${Xbox__Source_Files__Font})

set(Xbox__Source_Files__Leaderboards
#    "Xbox/Leaderboards/XboxLeaderboardManager.cpp"
#    "Xbox/Leaderboards/XboxLeaderboardManager.h"
)
source_group("Xbox\\Source Files\\Leaderboards" FILES ${Xbox__Source_Files__Leaderboards})

set(Xbox__Source_Files__Network
    "../Platform/Xbox/Network/extra.h"
#    "Xbox/Network/NetworkPlayerXbox.cpp"
#    "Xbox/Network/NetworkPlayerXbox.h"
#    "Xbox/Network/PlatformNetworkManagerXbox.cpp"
#    "Xbox/Network/PlatformNetworkManagerXbox.h"
)
source_group("Xbox\\Source Files\\Network" FILES ${Xbox__Source_Files__Network})

set(Xbox__Source_Files__Sentient
#    "Xbox/Sentient/SentientManager.cpp"
#    "Xbox/Sentient/SentientManager.h"
)
source_group("Xbox\\Source Files\\Sentient" FILES ${Xbox__Source_Files__Sentient})

set(Xbox__Source_Files__Sentient__DynamicConf
#    "Xbox/Sentient/DynamicConfigurations.cpp"
#    "Xbox/Sentient/DynamicConfigurations.h"
#    "Xbox/Sentient/trialConfigv1.bin"
)
source_group("Xbox\\Source Files\\Sentient\\DynamicConf" FILES ${Xbox__Source_Files__Sentient__DynamicConf})

set(Xbox__Source_Files__Sentient__Telemetry
#    "Xbox/Sentient/MinecraftTelemetry.h"
#    "Xbox/Sentient/MinecraftTelemetry.xml"
#    "Xbox/Sentient/SentientStats.cpp"
#    "Xbox/Sentient/SentientStats.h"
#    "Xbox/Sentient/SentientTelemetryCommon.h"
#    "Xbox/Sentient/TelemetryEnum.h"
)
source_group("Xbox\\Source Files\\Sentient\\Telemetry" FILES ${Xbox__Source_Files__Sentient__Telemetry})

set(Xbox__Source_Files__Social
#    "Xbox/Social/SocialManager.cpp"
#    "Xbox/Social/SocialManager.h"
)
source_group("Xbox\\Source Files\\Social" FILES ${Xbox__Source_Files__Social})

set(Xbox__Source_Files__XUI
#    "Common/XUI/XUI_CustomMessages.h"
#    "Common/XUI/XUI_Reinstall.cpp"
#    "Common/XUI/XUI_Reinstall.h"
#    "Common/XUI/XUI_TextEntry.cpp"
#    "Common/XUI/XUI_TextEntry.h"
#    "Common/XUI/XUI_XZP_Icons.h"
)
source_group("Xbox\\Source Files\\XUI" FILES ${Xbox__Source_Files__XUI})

set(Xbox__Source_Files__XUI__Base_Scene
#    "Common/XUI/XUI_BasePlayer.cpp"
#    "Common/XUI/XUI_BasePlayer.h"
#    "Common/XUI/XUI_Chat.cpp"
#    "Common/XUI/XUI_Chat.h"
#    "Common/XUI/XUI_HUD.cpp"
#    "Common/XUI/XUI_HUD.h"
#    "Common/XUI/XUI_Scene_Base.cpp"
#    "Common/XUI/XUI_Scene_Base.h"
)
source_group("Xbox\\Source Files\\XUI\\Base Scene" FILES ${Xbox__Source_Files__XUI__Base_Scene})

set(Xbox__Source_Files__XUI__Containers
#    "Common/XUI/XUI_Scene_AbstractContainer.cpp"
#    "Common/XUI/XUI_Scene_AbstractContainer.h"
#    "Common/XUI/XUI_Scene_Anvil.cpp"
#    "Common/XUI/XUI_Scene_Anvil.h"
#    "Common/XUI/XUI_Scene_BrewingStand.cpp"
#    "Common/XUI/XUI_Scene_BrewingStand.h"
#    "Common/XUI/XUI_Scene_Container.cpp"
#    "Common/XUI/XUI_Scene_Container.h"
#    "Common/XUI/XUI_Scene_CraftingPanel.cpp"
#    "Common/XUI/XUI_Scene_CraftingPanel.h"
#    "Common/XUI/XUI_Scene_Enchant.cpp"
#    "Common/XUI/XUI_Scene_Enchant.h"
#    "Common/XUI/XUI_Scene_Furnace.cpp"
#    "Common/XUI/XUI_Scene_Furnace.h"
#    "Common/XUI/XUI_Scene_Inventory.cpp"
#    "Common/XUI/XUI_Scene_Inventory.h"
#    "Common/XUI/XUI_Scene_Inventory_Creative.cpp"
#    "Common/XUI/XUI_Scene_Inventory_Creative.h"
#    "Common/XUI/XUI_Scene_Trading.cpp"
#    "Common/XUI/XUI_Scene_Trading.h"
#    "Common/XUI/XUI_Scene_Trap.cpp"
#    "Common/XUI/XUI_Scene_Trap.h"
)
source_group("Xbox\\Source Files\\XUI\\Containers" FILES ${Xbox__Source_Files__XUI__Containers})

set(Xbox__Source_Files__XUI__Controls
#    "Common/XUI/XUI_Controls.h"
#    "Common/XUI/XUI_Ctrl_4JEdit.cpp"
#    "Common/XUI/XUI_Ctrl_4JEdit.h"
#    "Common/XUI/XUI_Ctrl_4JIcon.cpp"
#    "Common/XUI/XUI_Ctrl_4JIcon.h"
#    "Common/XUI/XUI_Ctrl_4JList.cpp"
#    "Common/XUI/XUI_Ctrl_4JList.h"
#    "Common/XUI/XUI_Ctrl_BrewProgress.cpp"
#    "Common/XUI/XUI_Ctrl_BrewProgress.h"
#    "Common/XUI/XUI_Ctrl_BubblesProgress.cpp"
#    "Common/XUI/XUI_Ctrl_BubblesProgress.h"
#    "Common/XUI/XUI_Ctrl_BurnProgress.cpp"
#    "Common/XUI/XUI_Ctrl_BurnProgress.h"
#    "Common/XUI/XUI_Ctrl_CraftIngredientSlot.cpp"
#    "Common/XUI/XUI_Ctrl_CraftIngredientSlot.h"
#    "Common/XUI/XUI_Ctrl_EnchantButton.cpp"
#    "Common/XUI/XUI_Ctrl_EnchantButton.h"
#    "Common/XUI/XUI_Ctrl_EnchantmentBook.cpp"
#    "Common/XUI/XUI_Ctrl_EnchantmentBook.h"
#    "Common/XUI/XUI_Ctrl_EnchantmentButtonText.cpp"
#    "Common/XUI/XUI_Ctrl_EnchantmentButtonText.h"
#    "Common/XUI/XUI_Ctrl_FireProgress.cpp"
#    "Common/XUI/XUI_Ctrl_FireProgress.h"
#    "Common/XUI/XUI_Ctrl_LoadingProgress.cpp"
#    "Common/XUI/XUI_Ctrl_LoadingProgress.h"
#    "Common/XUI/XUI_Ctrl_MinecraftPlayer.cpp"
#    "Common/XUI/XUI_Ctrl_MinecraftPlayer.h"
#    "Common/XUI/XUI_Ctrl_MinecraftSkinPreview.cpp"
#    "Common/XUI/XUI_Ctrl_MinecraftSkinPreview.h"
#    "Common/XUI/XUI_Ctrl_MinecraftSlot.cpp"
#    "Common/XUI/XUI_Ctrl_MinecraftSlot.h"
#    "Common/XUI/XUI_Ctrl_MobEffect.cpp"
#    "Common/XUI/XUI_Ctrl_MobEffect.h"
#    "Common/XUI/XUI_Ctrl_PassThroughList.cpp"
#    "Common/XUI/XUI_Ctrl_PassthroughList.h"
#    "Common/XUI/XUI_Ctrl_ProgressCtrlBase.cpp"
#    "Common/XUI/XUI_Ctrl_ProgressCtrlBase.h"
#    "Common/XUI/XUI_Ctrl_SliderWrapper.cpp"
#    "Common/XUI/XUI_Ctrl_SliderWrapper.h"
#    "Common/XUI/XUI_Ctrl_SlotItem.h"
#    "Common/XUI/XUI_Ctrl_SlotItemCtrlBase.cpp"
#    "Common/XUI/XUI_Ctrl_SlotItemCtrlBase.h"
#    "Common/XUI/XUI_Ctrl_SlotItemListItem.h"
#    "Common/XUI/XUI_Ctrl_SlotList.cpp"
#    "Common/XUI/XUI_Ctrl_SlotList.h"
#    "Common/XUI/XUI_Ctrl_SplashPulser.cpp"
#    "Common/XUI/XUI_Ctrl_SplashPulser.h"
)
source_group("Xbox\\Source Files\\XUI\\Controls" FILES ${Xbox__Source_Files__XUI__Controls})

set(Xbox__Source_Files__XUI__Menu_screens
#    "Common/XUI/XUI_ConnectingProgress.cpp"
#    "Common/XUI/XUI_ConnectingProgress.h"
#    "Common/XUI/XUI_Death.cpp"
#    "Common/XUI/XUI_Death.h"
#    "Common/XUI/XUI_DLCOffers.cpp"
#    "Common/XUI/XUI_DLCOffers.h"
#    "Common/XUI/XUI_FullscreenProgress.cpp"
#    "Common/XUI/XUI_FullscreenProgress.h"
#    "Common/XUI/XUI_Helper.h"
#    "Common/XUI/XUI_InGameHostOptions.cpp"
#    "Common/XUI/XUI_InGameHostOptions.h"
#    "Common/XUI/XUI_InGameInfo.cpp"
#    "Common/XUI/XUI_InGameInfo.h"
#    "Common/XUI/XUI_InGamePlayerOptions.cpp"
#    "Common/XUI/XUI_InGamePlayerOptions.h"
#    "Common/XUI/XUI_Intro.cpp"
#    "Common/XUI/XUI_Intro.h"
#    "Common/XUI/XUI_LoadSettings.cpp"
#    "Common/XUI/XUI_LoadSettings.h"
#    "Common/XUI/XUI_MainMenu.cpp"
#    "Common/XUI/XUI_MainMenu.h"
#    "Common/XUI/XUI_MultiGameCreate.cpp"
#    "Common/XUI/XUI_MultiGameCreate.h"
#    "Common/XUI/XUI_MultiGameInfo.cpp"
#    "Common/XUI/XUI_MultiGameInfo.h"
#    "Common/XUI/XUI_MultiGameJoinLoad.cpp"
#    "Common/XUI/XUI_MultiGameJoinLoad.h"
#    "Common/XUI/XUI_MultiGameLaunchMoreOptions.cpp"
#    "Common/XUI/XUI_MultiGameLaunchMoreOptions.h"
#    "Common/XUI/XUI_NewUpdateMessage.cpp"
#    "Common/XUI/XUI_NewUpdateMessage.h"
#    "Common/XUI/XUI_PartnernetPassword.cpp"
#    "Common/XUI/XUI_PartnernetPassword.h"
#    "Common/XUI/XUI_SaveMessage.cpp"
#    "Common/XUI/XUI_SaveMessage.h"
#    "Common/XUI/XUI_Scene_Win.cpp"
#    "Common/XUI/XUI_Scene_Win.h"
#    "Common/XUI/XUI_SignEntry.cpp"
#    "Common/XUI/XUI_SignEntry.h"
#    "Common/XUI/XUI_Teleport.cpp"
#    "Common/XUI/XUI_Teleport.h"
#    "Common/XUI/XUI_TransferToXboxOne.cpp"
#    "Common/XUI/XUI_TransferToXboxOne.h"
#    "Common/XUI/XUI_TrialExitUpsell.cpp"
#    "Common/XUI/XUI_TrialExitUpsell.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens" FILES ${Xbox__Source_Files__XUI__Menu_screens})

set(Xbox__Source_Files__XUI__Menu_screens__Debug
#    "Common/XUI/XUI_debug.cpp"
#    "Common/XUI/XUI_Debug.h"
#    "Common/XUI/XUI_DebugItemEditor.cpp"
#    "Common/XUI/XUI_DebugItemEditor.h"
#    "Common/XUI/XUI_DebugOverlay.cpp"
#    "Common/XUI/XUI_DebugOverlay.h"
#    "Common/XUI/XUI_DebugSchematicCreator.cpp"
#    "Common/XUI/XUI_DebugSchematicCreator.h"
#    "Common/XUI/XUI_DebugSetCamera.cpp"
#    "Common/XUI/XUI_DebugSetCamera.h"
#    "Common/XUI/XUI_DebugTips.cpp"
#    "Common/XUI/XUI_DebugTips.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Debug" FILES ${Xbox__Source_Files__XUI__Menu_screens__Debug})

set(Xbox__Source_Files__XUI__Menu_screens__Help__Options
#    "Common/XUI/XUI_HelpAndOptions.cpp"
#    "Common/XUI/XUI_HelpAndOptions.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Help & Options" FILES ${Xbox__Source_Files__XUI__Menu_screens__Help__Options})

set(Xbox__Source_Files__XUI__Menu_screens__Help__Options__Controls
#    "Common/XUI/XUI_HelpControls.cpp"
#    "Common/XUI/XUI_HelpControls.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Help & Options\\Controls" FILES ${Xbox__Source_Files__XUI__Menu_screens__Help__Options__Controls})

set(Xbox__Source_Files__XUI__Menu_screens__Help__Options__Credits
#    "Common/XUI/XUI_HelpCredits.cpp"
#    "Common/XUI/XUI_HelpCredits.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Help & Options\\Credits" FILES ${Xbox__Source_Files__XUI__Menu_screens__Help__Options__Credits})

set(Xbox__Source_Files__XUI__Menu_screens__Help__Options__How_To_Play
#    "Common/XUI/XUI_HelpHowToPlay.cpp"
#    "Common/XUI/XUI_HelpHowToPlay.h"
#    "Common/XUI/XUI_HowToPlayMenu.cpp"
#    "Common/XUI/XUI_HowToPlayMenu.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Help & Options\\How To Play" FILES ${Xbox__Source_Files__XUI__Menu_screens__Help__Options__How_To_Play})

set(Xbox__Source_Files__XUI__Menu_screens__Help__Options__Settings
#    "Common/XUI/XUI_SettingsAll.cpp"
#    "Common/XUI/XUI_SettingsAll.h"
#    "Common/XUI/XUI_SettingsAudio.cpp"
#    "Common/XUI/XUI_SettingsAudio.h"
#    "Common/XUI/XUI_SettingsControl.cpp"
#    "Common/XUI/XUI_SettingsControl.h"
#    "Common/XUI/XUI_SettingsGraphics.cpp"
#    "Common/XUI/XUI_SettingsGraphics.h"
#    "Common/XUI/XUI_SettingsOptions.cpp"
#    "Common/XUI/XUI_SettingsOptions.h"
#    "Common/XUI/XUI_SettingsUI.cpp"
#    "Common/XUI/XUI_SettingsUI.h"
#    "Common/XUI/XUI_SkinSelect.cpp"
#    "Common/XUI/XUI_SkinSelect.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Help & Options\\Settings" FILES ${Xbox__Source_Files__XUI__Menu_screens__Help__Options__Settings})

set(Xbox__Source_Files__XUI__Menu_screens__Leaderboards
#    "Common/XUI/XUI_Leaderboards.cpp"
#    "Common/XUI/XUI_Leaderboards.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Leaderboards" FILES ${Xbox__Source_Files__XUI__Menu_screens__Leaderboards})

set(Xbox__Source_Files__XUI__Menu_screens__Pause
#    "Common/XUI/XUI_PauseMenu.cpp"
#    "Common/XUI/XUI_PauseMenu.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Pause" FILES ${Xbox__Source_Files__XUI__Menu_screens__Pause})

set(Xbox__Source_Files__XUI__Menu_screens__Social
#    "Common/XUI/XUI_SocialPost.cpp"
#    "Common/XUI/XUI_SocialPost.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Social" FILES ${Xbox__Source_Files__XUI__Menu_screens__Social})

set(Xbox__Source_Files__XUI__Menu_screens__Tutorial
#    "Common/XUI/XUI_TutorialPopup.cpp"
#    "Common/XUI/XUI_TutorialPopup.h"
)
source_group("Xbox\\Source Files\\XUI\\Menu screens\\Tutorial" FILES ${Xbox__Source_Files__XUI__Menu_screens__Tutorial})

set(Xbox__XML
#    "Xbox/XML/ATGXmlParser.cpp"
#    "Xbox/XML/ATGXmlParser.h"
#    "Xbox/XML/xmlFilesCallback.h"
)
source_group("Xbox\\XML" FILES ${Xbox__XML})

set(Xbox__res
#    "Xbox/res/terrain_MipmapLevel2.bmp"
#    "Xbox/res/terrain_MipmapLevel3.bmp"
)
source_group("Xbox\\res" FILES ${Xbox__res})

set(Xbox__res__audio
#    "Xbox/res/audio/minecraft.xsb"
#    "Xbox/res/audio/resident.xwb"
#    "Xbox/res/audio/streamed.xwb"
)
source_group("Xbox\\res\\audio" FILES ${Xbox__res__audio})

set(Xbox__xexxml
#    "Xbox/xex-dev.xml"
#    "Xbox/xex.xml"
)
source_group("Xbox\\xexxml" FILES ${Xbox__xexxml})

set(net__minecraft__client
    "../Rendering/Camera.cpp"
    "../Rendering/Camera.h"
    "../ClientConstants.cpp"
    "../ClientConstants.h"
    "../GameState/DemoUser.cpp"
    "../GameState/DemoUser.h"
    "../UI/GuiMessage.cpp"
    "../UI/GuiMessage.h"
    "../Input/KeyMapping.cpp"
    "../Input/KeyMapping.h"
    "../Rendering/Lighting.cpp"
    "../Rendering/Lighting.h"
    "../Utils/MemoryTracker.cpp"
    "../Utils/MemoryTracker.h"
    "../Minecraft.cpp"
    "../Minecraft.h"
    "../GameState/Options.cpp"
    "../GameState/Options.h"
    "../Rendering/EntityRenderers/ProgressRenderer.cpp"
    "../Rendering/EntityRenderers/ProgressRenderer.h"
    "../Utils/Timer.cpp"
    "../Utils/Timer.h"
    "../Player/User.cpp"
    "../Player/User.h"
)
source_group("net\\minecraft\\client" FILES ${net__minecraft__client})

set(net__minecraft__client__gui
    "../UI/Button.cpp"
    "../UI/Button.h"
    "../UI/Screens/ChatScreen.cpp"
    "../UI/Screens/ChatScreen.h"
    "../UI/Screens/ConfirmScreen.cpp"
    "../UI/Screens/ConfirmScreen.h"
    "../UI/Screens/ControlsScreen.cpp"
    "../UI/Screens/ControlsScreen.h"
    "../UI/Screens/CreateWorldScreen.cpp"
    "../UI/Screens/CreateWorldScreen.h"
    "../UI/Screens/DeathScreen.cpp"
    "../UI/Screens/DeathScreen.h"
    "../UI/EditBox.cpp"
    "../UI/EditBox.h"
    "../UI/Screens/ErrorScreen.cpp"
    "../UI/Screens/ErrorScreen.h"
    "../UI/Font.cpp"
    "../UI/Font.h"
    "../UI/Gui.cpp"
    "../UI/Gui.h"
    "../UI/GuiComponent.cpp"
    "../UI/GuiComponent.h"
    "../UI/Screens/InBedChatScreen.cpp"
    "../UI/Screens/InBedChatScreen.h"
    "../UI/Screens/JoinMultiplayerScreen.cpp"
    "../UI/Screens/JoinMultiplayerScreen.h"
    "../Rendering/Minimap.cpp"
    "../Rendering/Minimap.h"
    "../UI/Screens/NameEntryScreen.cpp"
    "../UI/Screens/NameEntryScreen.h"
    "../UI/Screens/OptionsScreen.cpp"
    "../UI/Screens/OptionsScreen.h"
    "../UI/Screens/PauseScreen.cpp"
    "../UI/Screens/PauseScreen.h"
    "../UI/Screens/RenameWorldScreen.cpp"
    "../UI/Screens/RenameWorldScreen.h"
    "../UI/Screen.cpp"
    "../UI/Screen.h"
    "../UI/ScreenSizeCalculator.cpp"
    "../UI/ScreenSizeCalculator.h"
    "../UI/ScrolledSelectionList.cpp"
    "../UI/ScrolledSelectionList.h"
    "../UI/Screens/SelectWorldScreen.cpp"
    "../UI/Screens/SelectWorldScreen.h"
    "../UI/SlideButton.cpp"
    "../UI/SlideButton.h"
    "../UI/SmallButton.cpp"
    "../UI/SmallButton.h"
    "../UI/Screens/VideoSettingsScreen.cpp"
    "../UI/Screens/VideoSettingsScreen.h"
)
source_group("net\\minecraft\\client\\gui" FILES ${net__minecraft__client__gui})

set(net__minecraft__client__gui__achievement
    "../UI/Screens/AchievementPopup.cpp"
    "../UI/Screens/AchievementPopup.h"
    "../UI/Screens/AchievementScreen.cpp"
    "../UI/Screens/AchievementScreen.h"
    "../UI/Screens/StatsScreen.cpp"
    "../UI/Screens/StatsScreen.h"
)
source_group("net\\minecraft\\client\\gui\\achievement" FILES ${net__minecraft__client__gui__achievement})

set(net__minecraft__client__gui__inventory
    "../UI/Screens/AbstractContainerScreen.cpp"
    "../UI/Screens/AbstractContainerScreen.h"
    "../UI/Screens/ContainerScreen.cpp"
    "../UI/Screens/ContainerScreen.h"
    "../UI/Screens/CraftingScreen.cpp"
    "../UI/Screens/CraftingScreen.h"
    "../UI/Screens/FurnaceScreen.cpp"
    "../UI/Screens/FurnaceScreen.h"
    "../UI/Screens/InventoryScreen.cpp"
    "../UI/Screens/InventoryScreen.h"
    "../UI/Screens/TextEditScreen.cpp"
    "../UI/Screens/TextEditScreen.h"
    "../UI/Screens/TrapScreen.cpp"
    "../UI/Screens/TrapScreen.h"
)
source_group("net\\minecraft\\client\\gui\\inventory" FILES ${net__minecraft__client__gui__inventory})

set(net__minecraft__client__gui__particle
    "../Rendering/Particles/GuiParticle.cpp"
    "../Rendering/Particles/GuiParticle.h"
    "../Rendering/Particles/GuiParticles.cpp"
    "../Rendering/Particles/GuiParticles.h"
)
source_group("net\\minecraft\\client\\gui\\particle" FILES ${net__minecraft__client__gui__particle})

set(net__minecraft__client__level
    "../Level/DemoLevel.cpp"
    "../Level/DemoLevel.h"
)
source_group("net\\minecraft\\client\\level" FILES ${net__minecraft__client__level})

set(net__minecraft__client__model
    "../Rendering/Models/BlazeModel.cpp"
    "../Rendering/Models/BlazeModel.h"
    "../Rendering/Models/BoatModel.cpp"
    "../Rendering/Models/BoatModel.h"
    "../Rendering/Models/BookModel.cpp"
    "../Rendering/Models/BookModel.h"
    "../Rendering/Models/ChestModel.cpp"
    "../Rendering/Models/ChestModel.h"
    "../Rendering/Models/ChickenModel.cpp"
    "../Rendering/Models/ChickenModel.h"
    "../Rendering/Models/CowModel.cpp"
    "../Rendering/Models/CowModel.h"
    "../Rendering/Models/CreeperModel.cpp"
    "../Rendering/Models/CreeperModel.h"
    "../Rendering/Models/EnderManModel.cpp"
    "../Rendering/Models/EnderManModel.h"
    "../Rendering/Models/GhastModel.cpp"
    "../Rendering/Models/GhastModel.h"
    "../Rendering/Models/HumanoidModel.cpp"
    "../Rendering/Models/HumanoidModel.h"
    "../Rendering/Models/LargeChestModel.cpp"
    "../Rendering/Models/LargeChestModel.h"
    "../Rendering/Models/LavaSlimeModel.cpp"
    "../Rendering/Models/LavaSlimeModel.h"
    "../Rendering/Models/MinecartModel.cpp"
    "../Rendering/Models/MinecartModel.h"
    "../Rendering/Models/OcelotModel.cpp"
    "../Rendering/Models/OcelotModel.h"
    "../Rendering/Models/PigModel.cpp"
    "../Rendering/Models/PigModel.h"
    "../Rendering/Polygon.cpp"
    "../Rendering/Polygon.h"
    "../Rendering/Models/QuadrupedModel.cpp"
    "../Rendering/Models/QuadrupedModel.h"
    "../Rendering/Models/SheepFurModel.cpp"
    "../Rendering/Models/SheepFurModel.h"
    "../Rendering/Models/SheepModel.cpp"
    "../Rendering/Models/SheepModel.h"
    "../Rendering/Models/SignModel.cpp"
    "../Rendering/Models/SignModel.h"
    "../Rendering/Models/SilverfishModel.cpp"
    "../Rendering/Models/SilverfishModel.h"
    "../Rendering/Models/SkeletonHeadModel.cpp"
    "../Rendering/Models/SkeletonHeadModel.h"
    "../Rendering/Models/SkeletonModel.cpp"
    "../Rendering/Models/SkeletonModel.h"
    "../Rendering/Models/SlimeModel.cpp"
    "../Rendering/Models/SlimeModel.h"
    "../Rendering/Models/SnowManModel.cpp"
    "../Rendering/Models/SnowManModel.h"
    "../Rendering/Models/SpiderModel.cpp"
    "../Rendering/Models/SpiderModel.h"
    "../Rendering/Models/SquidModel.cpp"
    "../Rendering/Models/SquidModel.h"
    "../Rendering/Vertex.cpp"
    "../Rendering/Vertex.h"
    "../Rendering/Models/VillagerGolemModel.cpp"
    "../Rendering/Models/VillagerGolemModel.h"
    "../Rendering/Models/VillagerModel.cpp"
    "../Rendering/Models/VillagerModel.h"
    "../Rendering/Models/VillagerZombieModel.cpp"
    "../Rendering/Models/VillagerZombieModel.h"
    "../Rendering/Models/WolfModel.cpp"
    "../Rendering/Models/WolfModel.h"
    "../Rendering/Models/ZombieModel.cpp"
    "../Rendering/Models/ZombieModel.h"
)
source_group("net\\minecraft\\client\\model" FILES ${net__minecraft__client__model})

set(net__minecraft__client__model__dragon
    "../Rendering/Models/DragonModel.cpp"
    "../Rendering/Models/DragonModel.h"
    "../Rendering/Models/EnderCrystalModel.cpp"
    "../Rendering/Models/EnderCrystalModel.h"
)
source_group("net\\minecraft\\client\\model\\dragon" FILES ${net__minecraft__client__model__dragon})

set(net__minecraft__client__model__geom
    "../Rendering/Cube.cpp"
    "../Rendering/Cube.h"
    "../Rendering/Models/Model.cpp"
    "../Rendering/Models/Model.h"
    "../Rendering/Models/ModelPart.cpp"
    "../Rendering/Models/ModelPart.h"
    "../Rendering/TexOffs.cpp"
    "../Rendering/TexOffs.h"
)
source_group("net\\minecraft\\client\\model\\geom" FILES ${net__minecraft__client__model__geom})

set(net__minecraft__client__multiplayer
    "../Network/ClientConnection.cpp"
    "../Network/ClientConnection.h"
    "../UI/Screens/ConnectScreen.cpp"
    "../UI/Screens/ConnectScreen.h"
    "../UI/Screens/DisconnectedScreen.cpp"
    "../UI/Screens/DisconnectedScreen.h"
    "../Network/MultiPlayerChunkCache.cpp"
    "../Network/MultiPlayerChunkCache.h"
    "../Player/MultiPlayerGameMode.cpp"
    "../Player/MultiPlayerGameMode.h"
    "../Level/MultiPlayerLevel.cpp"
    "../Level/MultiPlayerLevel.h"
    "../Player/MultiPlayerLocalPlayer.cpp"
    "../Player/MultiPlayerLocalPlayer.h"
    "../Network/PlayerInfo.h"
    "../UI/Screens/ReceivingLevelScreen.cpp"
    "../UI/Screens/ReceivingLevelScreen.h"
)
source_group("net\\minecraft\\client\\multiplayer" FILES ${net__minecraft__client__multiplayer})

set(net__minecraft__client__particle
    "../Rendering/Particles/BreakingItemParticle.cpp"
    "../Rendering/Particles/BreakingItemParticle.h"
    "../Rendering/Particles/BubbleParticle.cpp"
    "../Rendering/Particles/BubbleParticle.h"
    "../Rendering/Particles/CritParticle.cpp"
    "../Rendering/Particles/CritParticle.h"
    "../Rendering/Particles/CritParticle2.cpp"
    "../Rendering/Particles/CritParticle2.h"
    "../Rendering/Particles/DragonBreathParticle.cpp"
    "../Rendering/Particles/DragonBreathParticle.h"
    "../Rendering/Particles/DripParticle.cpp"
    "../Rendering/Particles/DripParticle.h"
    "../Rendering/Particles/EnchantmentTableParticle.cpp"
    "../Rendering/Particles/EnchantmentTableParticle.h"
    "../Rendering/Particles/EnderParticle.cpp"
    "../Rendering/Particles/EnderParticle.h"
    "../Rendering/Particles/ExplodeParticle.cpp"
    "../Rendering/Particles/ExplodeParticle.h"
    "../Rendering/Particles/FlameParticle.cpp"
    "../Rendering/Particles/FlameParticle.h"
    "../Rendering/Particles/FootstepParticle.cpp"
    "../Rendering/Particles/FootstepParticle.h"
    "../Rendering/Particles/HeartParticle.cpp"
    "../Rendering/Particles/HeartParticle.h"
    "../Rendering/Particles/HugeExplosionParticle.cpp"
    "../Rendering/Particles/HugeExplosionParticle.h"
    "../Rendering/Particles/HugeExplosionSeedParticle.cpp"
    "../Rendering/Particles/HugeExplosionSeedParticle.h"
    "../Rendering/Particles/LavaParticle.cpp"
    "../Rendering/Particles/LavaParticle.h"
    "../Rendering/Particles/NetherPortalParticle.cpp"
    "../Rendering/Particles/NetherPortalParticle.h"
    "../Rendering/Particles/NoteParticle.cpp"
    "../Rendering/Particles/NoteParticle.h"
    "../Rendering/Particles/Particle.cpp"
    "../Rendering/Particles/Particle.h"
    "../Rendering/Particles/ParticleEngine.cpp"
    "../Rendering/Particles/ParticleEngine.h"
    "../Rendering/Particles/PlayerCloudParticle.cpp"
    "../Rendering/Particles/PlayerCloudParticle.h"
    "../Rendering/Particles/RedDustParticle.cpp"
    "../Rendering/Particles/RedDustParticle.h"
    "../Rendering/Particles/SmokeParticle.cpp"
    "../Rendering/Particles/SmokeParticle.h"
    "../Rendering/Particles/SnowShovelParticle.cpp"
    "../Rendering/Particles/SnowShovelParticle.h"
    "../Rendering/Particles/SpellParticle.cpp"
    "../Rendering/Particles/SpellParticle.h"
    "../Rendering/Particles/SplashParticle.cpp"
    "../Rendering/Particles/SplashParticle.h"
    "../Rendering/Particles/SuspendedParticle.cpp"
    "../Rendering/Particles/SuspendedParticle.h"
    "../Rendering/Particles/SuspendedTownParticle.cpp"
    "../Rendering/Particles/SuspendedTownParticle.h"
    "../Rendering/Particles/TakeAnimationParticle.cpp"
    "../Rendering/Particles/TakeAnimationParticle.h"
    "../Rendering/Particles/TerrainParticle.cpp"
    "../Rendering/Particles/TerrainParticle.h"
    "../Rendering/Particles/WaterDropParticle.cpp"
    "../Rendering/Particles/WaterDropParticle.h"
)
source_group("net\\minecraft\\client\\particle" FILES ${net__minecraft__client__particle})

set(net__minecraft__client__player
    "../Input/Input.cpp"
    "../Input/Input.h"
    "../Player/LocalPlayer.cpp"
    "../Player/LocalPlayer.h"
    "../Player/RemotePlayer.cpp"
    "../Player/RemotePlayer.h"
)
source_group("net\\minecraft\\client\\player" FILES ${net__minecraft__client__player})

set(net__minecraft__client__renderer
    "../Rendering/Chunk.cpp"
    "../Rendering/Chunk.h"
    "../Rendering/Culling/DirtyChunkSorter.cpp"
    "../Rendering/Culling/DirtyChunkSorter.h"
    "../Rendering/Culling/DistanceChunkSorter.cpp"
    "../Rendering/Culling/DistanceChunkSorter.h"
    "../Rendering/EntityRenderers/EntityTileRenderer.cpp"
    "../Rendering/EntityRenderers/EntityTileRenderer.h"
    "../Rendering/GameRenderer.cpp"
    "../Rendering/GameRenderer.h"
    "../Textures/HttpTexture.cpp"
    "../Textures/HttpTexture.h"
    "../Textures/HttpTextureProcessor.h"
    "../Rendering/EntityRenderers/ItemInHandRenderer.cpp"
    "../Rendering/EntityRenderers/ItemInHandRenderer.h"
    "../Rendering/LevelRenderer.cpp"
    "../Rendering/LevelRenderer.h"
    "../Textures/MemTexture.cpp"
    "../Textures/MobSkinMemTextureProcessor.cpp"
    "../Textures/MobSkinTextureProcessor.cpp"
    "../Textures/MobSkinTextureProcessor.h"
    "../Rendering/OffsettedRenderList.cpp"
    "../Rendering/OffsettedRenderList.h"
    "../UI/Rect2i.cpp"
    "../UI/Rect2i.h"
    "../Rendering/Tesselator.cpp"
    "../Rendering/Tesselator.h"
    "../Textures/Textures.cpp"
    "../Textures/Textures.h"
    "../Rendering/EntityRenderers/TileRenderer.cpp"
    "../Rendering/EntityRenderers/TileRenderer.h"
)
source_group("net\\minecraft\\client\\renderer" FILES ${net__minecraft__client__renderer})

set(net__minecraft__client__renderer__culling
    "../Rendering/Culling/AllowAllCuller.cpp"
    "../Rendering/Culling/AllowAllCuller.h"
    "../Rendering/Culling/Culler.h"
    "../Rendering/Frustum.cpp"
    "../Rendering/Frustum.h"
    "../Rendering/FrustumCuller.cpp"
    "../Rendering/FrustumCuller.h"
    "../Rendering/FrustumData.cpp"
    "../Rendering/FrustumData.h"
    "../Rendering/Culling/ViewportCuller.cpp"
    "../Rendering/Culling/ViewportCuller.h"
)
source_group("net\\minecraft\\client\\renderer\\culling" FILES ${net__minecraft__client__renderer__culling})

set(net__minecraft__client__renderer__entity
    "../Rendering/EntityRenderers/ArrowRenderer.cpp"
    "../Rendering/EntityRenderers/ArrowRenderer.h"
    "../Rendering/EntityRenderers/BlazeRenderer.cpp"
    "../Rendering/EntityRenderers/BlazeRenderer.h"
    "../Rendering/EntityRenderers/BoatRenderer.cpp"
    "../Rendering/EntityRenderers/BoatRenderer.h"
    "../Rendering/EntityRenderers/ChickenRenderer.cpp"
    "../Rendering/EntityRenderers/ChickenRenderer.h"
    "../Rendering/EntityRenderers/CowRenderer.cpp"
    "../Rendering/EntityRenderers/CowRenderer.h"
    "../Rendering/EntityRenderers/CreeperRenderer.cpp"
    "../Rendering/EntityRenderers/CreeperRenderer.h"
    "../Rendering/EntityRenderers/DefaultRenderer.cpp"
    "../Rendering/EntityRenderers/DefaultRenderer.h"
    "../Rendering/EntityRenderers/EnderCrystalRenderer.cpp"
    "../Rendering/EntityRenderers/EnderCrystalRenderer.h"
    "../Rendering/EntityRenderers/EnderDragonRenderer.cpp"
    "../Rendering/EntityRenderers/EnderDragonRenderer.h"
    "../Rendering/EntityRenderers/EnderManRenderer.cpp"
    "../Rendering/EntityRenderers/EnderManRenderer.h"
    "../Rendering/EntityRenderers/EntityRenderDispatcher.cpp"
    "../Rendering/EntityRenderers/EntityRenderDispatcher.h"
    "../Rendering/EntityRenderers/EntityRenderer.cpp"
    "../Rendering/EntityRenderers/EntityRenderer.h"
    "../Rendering/EntityRenderers/ExperienceOrbRenderer.cpp"
    "../Rendering/EntityRenderers/ExperienceOrbRenderer.h"
    "../Rendering/EntityRenderers/FallingTileRenderer.cpp"
    "../Rendering/EntityRenderers/FallingTileRenderer.h"
    "../Rendering/EntityRenderers/FireballRenderer.cpp"
    "../Rendering/EntityRenderers/FireballRenderer.h"
    "../Rendering/EntityRenderers/FishingHookRenderer.cpp"
    "../Rendering/EntityRenderers/FishingHookRenderer.h"
    "../Rendering/EntityRenderers/GhastRenderer.cpp"
    "../Rendering/EntityRenderers/GhastRenderer.h"
    "../Rendering/EntityRenderers/GiantMobRenderer.cpp"
    "../Rendering/EntityRenderers/GiantMobRenderer.h"
    "../Rendering/EntityRenderers/HumanoidMobRenderer.cpp"
    "../Rendering/EntityRenderers/HumanoidMobRenderer.h"
    "../Rendering/EntityRenderers/ItemFrameRenderer.cpp"
    "../Rendering/EntityRenderers/ItemFrameRenderer.h"
    "../Rendering/EntityRenderers/ItemRenderer.cpp"
    "../Rendering/EntityRenderers/ItemRenderer.h"
    "../Rendering/EntityRenderers/ItemSpriteRenderer.cpp"
    "../Rendering/EntityRenderers/ItemSpriteRenderer.h"
    "../Rendering/EntityRenderers/LavaSlimeRenderer.cpp"
    "../Rendering/EntityRenderers/LavaSlimeRenderer.h"
    "../Rendering/EntityRenderers/LightningBoltRenderer.cpp"
    "../Rendering/EntityRenderers/LightningBoltRenderer.h"
    "../Rendering/EntityRenderers/MinecartRenderer.cpp"
    "../Rendering/EntityRenderers/MinecartRenderer.h"
    "../Rendering/EntityRenderers/MobRenderer.cpp"
    "../Rendering/EntityRenderers/MobRenderer.h"
    "../Rendering/EntityRenderers/MushroomCowRenderer.cpp"
    "../Rendering/EntityRenderers/MushroomCowRenderer.h"
    "../Rendering/EntityRenderers/OcelotRenderer.cpp"
    "../Rendering/EntityRenderers/OcelotRenderer.h"
    "../Rendering/EntityRenderers/PaintingRenderer.cpp"
    "../Rendering/EntityRenderers/PaintingRenderer.h"
    "../Rendering/EntityRenderers/PigRenderer.cpp"
    "../Rendering/EntityRenderers/PigRenderer.h"
    "../Rendering/EntityRenderers/PlayerRenderer.cpp"
    "../Rendering/EntityRenderers/PlayerRenderer.h"
    "../Rendering/EntityRenderers/SheepRenderer.cpp"
    "../Rendering/EntityRenderers/SheepRenderer.h"
    "../Rendering/EntityRenderers/SilverfishRenderer.cpp"
    "../Rendering/EntityRenderers/SilverfishRenderer.h"
    "../Rendering/EntityRenderers/SlimeRenderer.cpp"
    "../Rendering/EntityRenderers/SlimeRenderer.h"
    "../Rendering/EntityRenderers/SnowManRenderer.cpp"
    "../Rendering/EntityRenderers/SnowManRenderer.h"
    "../Rendering/EntityRenderers/SpiderRenderer.cpp"
    "../Rendering/EntityRenderers/SpiderRenderer.h"
    "../Rendering/EntityRenderers/SquidRenderer.cpp"
    "../Rendering/EntityRenderers/SquidRenderer.h"
    "../Rendering/EntityRenderers/TntRenderer.cpp"
    "../Rendering/EntityRenderers/TntRenderer.h"
    "../Rendering/EntityRenderers/VillagerGolemRenderer.cpp"
    "../Rendering/EntityRenderers/VillagerGolemRenderer.h"
    "../Rendering/EntityRenderers/VillagerRenderer.cpp"
    "../Rendering/EntityRenderers/VillagerRenderer.h"
    "../Rendering/EntityRenderers/WolfRenderer.cpp"
    "../Rendering/EntityRenderers/WolfRenderer.h"
    "../Rendering/EntityRenderers/ZombieRenderer.cpp"
    "../Rendering/EntityRenderers/ZombieRenderer.h"
)
source_group("net\\minecraft\\client\\renderer\\entity" FILES ${net__minecraft__client__renderer__entity})

set(net__minecraft__client__renderer__texture
    "../Textures/Stitching/PreStitchedTextureMap.cpp"
    "../Textures/Stitching/PreStitchedTextureMap.h"
    "../UI/SimpleIcon.cpp"
    "../UI/SimpleIcon.h"
    "../Textures/Stitching/StitchedTexture.cpp"
    "../Textures/Stitching/StitchedTexture.h"
    "../Textures/Stitching/Stitcher.cpp"
    "../Textures/Stitching/Stitcher.h"
    "../Textures/Stitching/StitchSlot.cpp"
    "../Textures/Stitching/StitchSlot.h"
    "../Textures/Texture.cpp"
    "../Textures/Texture.h"
    "../Textures/TextureHolder.cpp"
    "../Textures/TextureHolder.h"
    "../Textures/TextureManager.cpp"
    "../Textures/TextureManager.h"
    "../Textures/Stitching/TextureMap.cpp"
    "../Textures/Stitching/TextureMap.h"
)
source_group("net\\minecraft\\client\\renderer\\texture" FILES ${net__minecraft__client__renderer__texture})

set(net__minecraft__client__renderer__texture__custom
    "../Textures/ClockTexture.cpp"
    "../Textures/ClockTexture.h"
    "../Textures/CompassTexture.cpp"
    "../Textures/CompassTexture.h"
)
source_group("net\\minecraft\\client\\renderer\\texture\\custom" FILES ${net__minecraft__client__renderer__texture__custom})

set(net__minecraft__client__renderer__tileentity
    "../Rendering/EntityRenderers/ChestRenderer.cpp"
    "../Rendering/EntityRenderers/ChestRenderer.h"
    "../Rendering/EntityRenderers/EnchantTableRenderer.cpp"
    "../Rendering/EntityRenderers/EnchantTableRenderer.h"
    "../Rendering/EntityRenderers/EnderChestRenderer.cpp"
    "../Rendering/EntityRenderers/EnderChestRenderer.h"
    "../Rendering/EntityRenderers/MobSpawnerRenderer.cpp"
    "../Rendering/EntityRenderers/MobSpawnerRenderer.h"
    "../Rendering/EntityRenderers/PistonPieceRenderer.cpp"
    "../Rendering/EntityRenderers/PistonPieceRenderer.h"
    "../Rendering/EntityRenderers/SignRenderer.cpp"
    "../Rendering/EntityRenderers/SignRenderer.h"
    "../Rendering/EntityRenderers/SkullTileRenderer.cpp"
    "../Rendering/EntityRenderers/SkullTileRenderer.h"
    "../Rendering/EntityRenderers/TheEndPortalRenderer.cpp"
    "../Rendering/EntityRenderers/TheEndPortalRenderer.h"
    "../Rendering/EntityRenderers/TileEntityRenderDispatcher.cpp"
    "../Rendering/EntityRenderers/TileEntityRenderDispatcher.h"
    "../Rendering/EntityRenderers/TileEntityRenderer.cpp"
    "../Rendering/EntityRenderers/TileEntityRenderer.h"
)
source_group("net\\minecraft\\client\\renderer\\tileentity" FILES ${net__minecraft__client__renderer__tileentity})

set(net__minecraft__client__skins
    "../Textures/Packs/AbstractTexturePack.cpp"
    "../Textures/Packs/AbstractTexturePack.h"
    "../Textures/Packs/DefaultTexturePack.cpp"
    "../Textures/Packs/DefaultTexturePack.h"
    "../Textures/Packs/DLCTexturePack.cpp"
    "../Textures/Packs/DLCTexturePack.h"
    "../Textures/Packs/FileTexturePack.cpp"
    "../Textures/Packs/FileTexturePack.h"
    "../Textures/Packs/FolderTexturePack.cpp"
    "../Textures/Packs/FolderTexturePack.h"
    "../Textures/Packs/TexturePack.cpp"
    "../Textures/Packs/TexturePack.h"
    "../Textures/Packs/TexturePackRepository.cpp"
    "../Textures/Packs/TexturePackRepository.h"
)
source_group("net\\minecraft\\client\\skins" FILES ${net__minecraft__client__skins})

set(net__minecraft__client__title
    "../UI/Screens/TitleScreen.cpp"
    "../UI/Screens/TitleScreen.h"
)
source_group("net\\minecraft\\client\\title" FILES ${net__minecraft__client__title})

set(net__minecraft__server
    "../Input/ConsoleInput.cpp"
    "../Input/ConsoleInput.h"
    "../Input/ConsoleInputSource.h"
    "../MinecraftServer.cpp"
    "../MinecraftServer.h"
    "../Network/PlayerList.cpp"
    "../Network/PlayerList.h"
    "../Network/ServerInterface.h"
    "../GameState/Settings.cpp"
    "../GameState/Settings.h"
)
source_group("net\\minecraft\\server" FILES ${net__minecraft__server})

set(net__minecraft__server__commands
    "../Network/ServerCommandDispatcher.cpp"
    "../Network/ServerCommandDispatcher.h"
    "../Commands/TeleportCommand.cpp"
    "../Commands/TeleportCommand.h"
)
source_group("net\\minecraft\\server\\commands" FILES ${net__minecraft__server__commands})

set(net__minecraft__server__level
    "../Level/DerivedServerLevel.cpp"
    "../Level/DerivedServerLevel.h"
    "../Player/EntityTracker.cpp"
    "../Player/EntityTracker.h"
    "../Network/PlayerChunkMap.cpp"
    "../Network/PlayerChunkMap.h"
    "../Network/ServerChunkCache.cpp"
    "../Network/ServerChunkCache.h"
    "../Level/ServerLevel.cpp"
    "../Level/ServerLevel.h"
    "../Level/ServerLevelListener.cpp"
    "../Level/ServerLevelListener.h"
    "../Player/ServerPlayer.cpp"
    "../Player/ServerPlayer.h"
    "../Player/ServerPlayerGameMode.cpp"
    "../Player/ServerPlayerGameMode.h"
    "../Player/TrackedEntity.cpp"
    "../Player/TrackedEntity.h"
)
source_group("net\\minecraft\\server\\level" FILES ${net__minecraft__server__level})

set(net__minecraft__server__network
    "../Network/PendingConnection.cpp"
    "../Network/PendingConnection.h"
    "../Network/PlayerConnection.cpp"
    "../Network/PlayerConnection.h"
    "../Network/ServerConnection.cpp"
    "../Network/ServerConnection.h"
)
source_group("net\\minecraft\\server\\network" FILES ${net__minecraft__server__network})

set(net__minecraft__stats
    "../GameState/StatsCounter.cpp"
    "../GameState/StatsCounter.h"
    "../GameState/StatsSyncer.cpp"
    "../GameState/StatsSyncer.h"
)
source_group("net\\minecraft\\stats" FILES ${net__minecraft__stats})

set(ALL_FILES
    ${no_group_source_files}
    ${Common}
    ${Common__Source_Files}
    ${Common__Source_Files__Audio}
    ${Common__Source_Files__BuildVer}
    ${Common__Source_Files__Colours}
    ${Common__Source_Files__DLC}
    ${Common__Source_Files__GameRules}
    ${Common__Source_Files__GameRules__LevelGeneration}
    ${Common__Source_Files__GameRules__LevelGeneration__StructureActions}
    ${Common__Source_Files__GameRules__LevelRules}
    ${Common__Source_Files__GameRules__LevelRules__RuleDefinitions}
    ${Common__Source_Files__GameRules__LevelRules__Rules}
    ${Common__Source_Files__Leaderboards}
    ${Common__Source_Files__Localisation}
    ${Common__Source_Files__Network}
    ${Common__Source_Files__Network__Sony}
    ${Common__Source_Files__Telemetry}
    ${Common__Source_Files__Trial}
    ${Common__Source_Files__Tutorial}
    ${Common__Source_Files__Tutorial__Constraints}
    ${Common__Source_Files__Tutorial__Hints}
    ${Common__Source_Files__Tutorial__Tasks}
    ${Common__Source_Files__UI}
    ${Common__Source_Files__UI__All_Platforms}
    ${Common__Source_Files__UI__Components}
    ${Common__Source_Files__UI__Controls}
    ${Common__Source_Files__UI__Scenes}
    ${Common__Source_Files__UI__Scenes__Debug}
    ${Common__Source_Files__UI__Scenes__Frontend_Menu_screens}
    ${Common__Source_Files__UI__Scenes__Help__Options}
    ${Common__Source_Files__UI__Scenes__In-Game_Menu_Screens}
    ${Common__Source_Files__UI__Scenes__In-Game_Menu_Screens__Containers}
    ${Common__Source_Files__zlib}
    ${Durango}
    ${Durango__4JLibs__inc}
    ${Durango__DurangoExtras}
    ${Durango__Iggy__gdraw}
    ${Durango__Iggy__include}
    ${Durango__Miles_Sound_System__include}
    ${Durango__Network}
    ${Durango__ServiceConfig}
    ${Durango__Source_Files}
    ${Durango__Source_Files__Achievements}
    ${Durango__Source_Files__Leaderboards}
    ${Durango__Source_Files__Sentient}
    ${Durango__Source_Files__Social}
    ${Durango__XML}
    ${Header_Files}
    ${Orbis}
    ${Orbis__4JLibs__inc}
    ${Orbis__4JLibs__libs}
    ${Orbis__Iggy__gdraw}
    ${Orbis__Iggy__include}
    ${Orbis__Miles_Sound_System__include}
    ${Orbis__Miles_Sound_System__lib}
    ${Orbis__Network}
    ${Orbis__OrbisExtras}
    ${Orbis__Source_Files}
    ${Orbis__Source_Files__Leaderboards}
    ${Orbis__Source_Files__Sentient}
    ${Orbis__Source_Files__Social}
    ${Orbis__XML}
    ${PS3}
    ${PS3__4JLibs}
    ${PS3__4JLibs__inc}
    ${PS3__ChunkRebuild_SPU}
    ${PS3__CompressedTile_SPU}
    ${PS3__Iggy__gdraw}
    ${PS3__Iggy__include}
    ${PS3__Miles_Sound_System__include}
    ${PS3__Miles_Sound_System__lib}
    ${PS3__Miles_Sound_System__lib__spu}
    ${PS3__PS3Extras}
    ${PS3__Source_Files}
    ${PS3__Source_Files__Audio}
    ${PS3__Source_Files__Leaderboards}
    ${PS3__Source_Files__Network}
    ${PS3__Source_Files__Sentient}
    ${PS3__Source_Files__Social}
    ${Linux}
    ${PSVita}
    ${PSVita__4JLibs__inc}
    ${PSVita__GameConfig}
    ${PSVita__Iggy__gdraw}
    ${PSVita__Iggy__include}
    ${PSVita__Miles_Sound_System__Include}
    ${PSVita__PSVitaExtras}
    ${PSVita__Source_Files}
    ${PSVita__Source_Files__Leaderboards}
    ${PSVita__Source_Files__Network}
    ${PSVita__Source_Files__Sentient}
    ${PSVita__Source_Files__Social}
    ${PSVita__XML}
    ${Source_Files}
    ${Windows}
    ${Windows64}
    ${Windows64__4JLibs__inc}
    ${Windows64__GameConfig}
    ${Windows64__Iggy__gdraw}
    ${Windows64__Iggy__include}
    ${Windows64__Miles_Sound_System__Include}
    ${Windows64__Source_Files}
    ${Windows64__Source_Files__Leaderboards}
    ${Windows64__Source_Files__Sentient}
    ${Windows64__Source_Files__Social}
    ${Windows64__XML}
    ${Xbox__4JLibs__Media}
    ${Xbox__4JLibs__inc}
    ${Xbox__GameConfig}
    ${Xbox__SentientLibs__inc}
    ${Xbox__Source_Files}
    ${Xbox__Source_Files__Audio}
    ${Xbox__Source_Files__Font}
    ${Xbox__Source_Files__Leaderboards}
    ${Xbox__Source_Files__Network}
    ${Xbox__Source_Files__Sentient}
    ${Xbox__Source_Files__Sentient__DynamicConf}
    ${Xbox__Source_Files__Sentient__Telemetry}
    ${Xbox__Source_Files__Social}
    ${Xbox__Source_Files__XUI}
    ${Xbox__Source_Files__XUI__Base_Scene}
    ${Xbox__Source_Files__XUI__Containers}
    ${Xbox__Source_Files__XUI__Controls}
    ${Xbox__Source_Files__XUI__Menu_screens}
    ${Xbox__Source_Files__XUI__Menu_screens__Debug}
    ${Xbox__Source_Files__XUI__Menu_screens__Help__Options}
    ${Xbox__Source_Files__XUI__Menu_screens__Help__Options__Controls}
    ${Xbox__Source_Files__XUI__Menu_screens__Help__Options__Credits}
    ${Xbox__Source_Files__XUI__Menu_screens__Help__Options__How_To_Play}
    ${Xbox__Source_Files__XUI__Menu_screens__Help__Options__Settings}
    ${Xbox__Source_Files__XUI__Menu_screens__Leaderboards}
    ${Xbox__Source_Files__XUI__Menu_screens__Pause}
    ${Xbox__Source_Files__XUI__Menu_screens__Social}
    ${Xbox__Source_Files__XUI__Menu_screens__Tutorial}
    ${Xbox__XML}
    ${Xbox__res}
    ${Xbox__res__audio}
    ${Xbox__xexxml}
    ${net__minecraft__client}
    ${net__minecraft__client__gui}
    ${net__minecraft__client__gui__achievement}
    ${net__minecraft__client__gui__inventory}
    ${net__minecraft__client__gui__particle}
    ${net__minecraft__client__level}
    ${net__minecraft__client__model}
    ${net__minecraft__client__model__dragon}
    ${net__minecraft__client__model__geom}
    ${net__minecraft__client__multiplayer}
    ${net__minecraft__client__particle}
    ${net__minecraft__client__player}
    ${net__minecraft__client__renderer}
    ${net__minecraft__client__renderer__culling}
    ${net__minecraft__client__renderer__entity}
    ${net__minecraft__client__renderer__texture}
    ${net__minecraft__client__renderer__texture__custom}
    ${net__minecraft__client__renderer__tileentity}
    ${net__minecraft__client__skins}
    ${net__minecraft__client__title}
    ${net__minecraft__server}
    ${net__minecraft__server__commands}
    ${net__minecraft__server__level}
    ${net__minecraft__server__network}
    ${net__minecraft__stats}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Xbox360Proj"
)
################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_CONTENTPACKAGE_NO_TU   "${OUTPUT_DIRECTORY}default$ENV{TargetExt}"
    TARGET_NAME_CONTENTPACKAGE_SYMBOLS "${OUTPUT_DIRECTORY}default$ENV{TargetExt}"
    TARGET_NAME_CONTENTPACKAGE         "${PROJECT_NAME}"
    TARGET_NAME_DEBUG                  "${PROJECT_NAME}"
    TARGET_NAME_RELEASE                "${PROJECT_NAME}"
    TARGET_NAME_RELEASEFORART          "${OUTPUT_DIRECTORY}default$ENV{TargetExt}"
)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_CONTENTPACKAGE_NO_TU   "${CMAKE_CURRENT_SOURCE_DIR}/"
    OUTPUT_DIRECTORY_CONTENTPACKAGE_SYMBOLS "${CMAKE_CURRENT_SOURCE_DIR}/"
    OUTPUT_DIRECTORY_CONTENTPACKAGE         "${CMAKE_CURRENT_SOURCE_DIR}/"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PDB_OUTPUT_DIRECTORY                        "${OUTPUT_DIRECTORY}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_CONTENTPACKAGE_NO_TU   "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_CONTENTPACKAGE_SYMBOLS "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_CONTENTPACKAGE         "TRUE"
)


################################################################################
# MSVC runtime library
################################################################################
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
       $<$<CONFIG:ContentPackage_NO_TU>:
        libstdc++
    >
    $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
        libstdc++
    >
    $<$<CONFIG:ContentPackage>:
        libstdc++
    >
    $<$<CONFIG:Debug>:
        libstdc++_debug
    >
    $<$<CONFIG:Release>:
        libstdc++
    >
    $<$<CONFIG:ReleaseForArt>:
        libstdc++
    >
    $<$<NOT:$<OR:$<CONFIG:ContentPackage_NO_TU>,$<CONFIG:CONTENTPACKAGE_SYMBOLS>,$<CONFIG:ContentPackage>,$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:ReleaseForArt>>>:${CXX_RUNTIME_LIBRARY_DEFAULT}>
)


set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
)


################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:ContentPackage_NO_TU>:"
        "_TU_BUILD;"
        "_FINAL_BUILD;"
        "_ITERATOR_DEBUG_LEVEL=0;"
        "NDEBUG;"
        "_LINUX;"
        "_CONTENT_PACKAGE;"
        "_MBCS"
    ">"
    "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
        "_TU_BUILD;"
        "_FINAL_BUILD;"
        "_ITERATOR_DEBUG_LEVEL=0;"
        "NDEBUG;"
        "_LINUX;"
        "_CONTENT_PACKAGE;"
        "_MBCS"
    ">"
    "$<$<CONFIG:ContentPackage>:"
        "SPLIT_SAVES;"
        "_LARGE_WORLDS;"
        "_EXTENDED_ACHIEVEMENTS;"
        "_FINAL_BUILD;"
        "_CONTENT_PACKAGE;"
        "NDEBUG;"
        "UNICODE;"
        "_UNICODE"
    ">"
    "$<$<CONFIG:Debug>:"
        "SPLIT_SAVES;"
        "_LARGE_WORLDS;"
        "_EXTENDED_ACHIEVEMENTS;"
        "UNICODE;"
        "_UNICODE;"
        "_DEBUG_MENUS_ENABLED;"
        "_DEBUG;"
        "_LINUX;"
        "UNICODE;"
        "_UNICODE"
    ">"
    "$<$<CONFIG:Release>:"
        "SPLIT_SAVES;"
        "_LARGE_WORLDS;"
        "_EXTENDED_ACHIEVEMENTS;"
        "PROFILE;"
        "NDEBUG;"
        "UNICODE;"
        "_UNICODE;"
        "_DEBUG_MENUS_ENABLED;"
        "_CRT_NON_CONFORMING_SWPRINTFS;"
        "_LINUX;"
        "UNICODE;"
        "_UNICODE"
    ">"
    "$<$<CONFIG:ReleaseForArt>:"
        "_TU_BUILD;"
        "_FINAL_BUILD;"
        "_ITERATOR_DEBUG_LEVEL=0;"
        "NDEBUG;"
        "_LINUX;"
        "_CONTENT_PACKAGE;"
        ";"
        "_MBCS"
    ">"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /Ox;
                /GF;
                /Ot;
                /GS-
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /Ox;
                /GF;
                /Ot;
                /GS-
            >
            $<$<CONFIG:ContentPackage>:
                /GL-;
                /O2;
                /Oi;
                /GF;
                /Ot;
                /GS-
            >
            $<$<CONFIG:Debug>:
                /Od;
                /Oi;
                /RTC1;
                /GS
            >
            $<$<CONFIG:Release>:
                /GL-;
                /O2;
                /Oi;
                /Ot;
                /GS
            >
            $<$<CONFIG:ReleaseForArt>:
                /Ox;
                /GF;
                /Ot;
                /GS-
            >
            /MP;
            /Gy;
            /W3;
            /Zi;
            /GR;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /MP
            >
            $<$<CONFIG:ContentPackage>:
                /MP
            >
            $<$<CONFIG:Debug>:
                /MP
            >
            $<$<CONFIG:Release>:
                /MP
            >
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            /GR;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PS3")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:ContentPackage>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:Debug>:
                /Gm;
                /Od
            >
            $<$<CONFIG:Release>:
                /Gm;
                /Od
            >
            $<$<CONFIG:ReleaseForArt>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            /MP;
            /W3;
            /Zi;
            /GR;
            /wd1700;
            /wd613;
            /wd1011;
            -Xpch_override=1;
            /GS-
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PSVita")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:ContentPackage>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:Debug>:
                /Gm;
                /Od;
                /wd1786;
                /wd2623;
                /wd2624;
                /wd1628
            >
            $<$<CONFIG:Release>:
                /Gm;
                /Od
            >
            $<$<CONFIG:ReleaseForArt>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            /MP;
            /W3;
            /Zi;
            /GR;
            /wd1700;
            /wd613;
            /wd1011;
            -Xpch_override=1;
            /GS-
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:ContentPackage>:
                /Ox;
                /GF;
                /Gy;
                /Ot
            >
            $<$<CONFIG:Debug>:
                /Od;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:Release>:
                /Ox;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            $<$<CONFIG:ReleaseForArt>:
                /Ox;
                /GF;
                /Gy;
                /Ot;
                ${DEFAULT_CXX_EXCEPTION_HANDLING}
            >
            /MP;
            /W3;
            /Zi;
            /GR;
            /GS-
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Xbox 360")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /Ox;
                /GF;
                /Gy;
                /Ot
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /Ox;
                /GF;
                /Gy;
                /Ot
            >
            $<$<CONFIG:ContentPackage>:
                /Ox;
                /GF;
                /Gy;
                /Ot
            >
            $<$<CONFIG:Debug>:
                /Gm-;
                /Od
            >
            $<$<CONFIG:Release>:
                /GL;
                /Gm-;
                /Ox;
                /GF;
                /Gy;
                /Ot
            >
            $<$<CONFIG:ReleaseForArt>:
                /GL;
                /Gm-;
                /Ox;
                /GF;
                /Gy;
                /Ot
            >
            /MP;
            /W3;
            /Zi;
            /GR;
            /GS-
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/adler32.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/compress.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/crc32.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/deflate.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/gzclose.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/gzlib.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/gzread.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/gzwrite.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/infback.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/inffast.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/inflate.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/inftrees.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/trees.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/uncompr.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:ContentPackage_NO_TU>:"
                "/Y-"
            ">"
            "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
            "$<$<CONFIG:Release>:"
                "/Y-"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(Common/zlib/zutil.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Xbox 360")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:Debug>:"
                "/showIncludes"
            ">"
            "$<$<CONFIG:Release>:"
                "/showIncludes"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/showIncludes"
            ">"
        )
    endif()
    source_file_compile_options(glWrapper.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Xbox 360")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:Release>:"
                "/GL-;/Od;/Ob0"
            ">"
            "$<$<CONFIG:ReleaseForArt>:"
                "/GL-;/Od;/Ob0"
            ">"
        )
    endif()
    source_file_compile_options(LevelRenderer.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PS3")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PSVita")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Xbox 360")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(Xbox/Sentient/SentientStats.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PS3")
        string(CONCAT FILE_CL_OPTIONS
            "-Xpch_override=1"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PSVita")
        string(CONCAT FILE_CL_OPTIONS
            "-Xpch_override=1"
        )
    endif()
    source_file_compile_options(Common/XUI/XUI_Ctrl_LoadingProgress.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ContentPackage>:
                /NODEFAULTLIB:kernel32.lib;
                /NODEFAULTLIB:oldnames.lib;
                /NODEFAULTLIB:runtimeobject.lib;
                /NODEFAULTLIB:ole32.lib;
                /SUBSYSTEM:CONSOLE;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:Debug>:
                /NODEFAULTLIB:kernel32.lib;
                /NODEFAULTLIB:oldnames.lib;
                /NODEFAULTLIB:runtimeobject.lib;
                /NODEFAULTLIB:ole32.lib;
                /NOLOGO;
                /MANIFEST;
                /SUBSYSTEM:CONSOLE;
                /OPT:NOREF;
                /OPT:NOICF;
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /NODEFAULTLIB:kernel32.lib;
                /NODEFAULTLIB:oldnames.lib;
                /NODEFAULTLIB:runtimeobject.lib;
                /NODEFAULTLIB:ole32.lib;
                /NOLOGO;
                /SUBSYSTEM:CONSOLE;
                /OPT:REF;
                /INCREMENTAL
            >
            $<$<CONFIG:ReleaseForArt>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /DEBUG:FULL
            >
            $<$<CONFIG:Debug>:
                /DEBUG
            >
            $<$<CONFIG:Release>:
                /DEBUG
            >
            $<$<CONFIG:ReleaseForArt>:
                /DEBUG:FULL
            >
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PS3")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO;
                --no-toc-restore;
                --strip-duplicates;
                --ppuguid
            >
            $<$<CONFIG:ContentPackage>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO;
                --no-toc-restore;
                --strip-duplicates;
                --ppuguid
            >
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /INCREMENTAL;
                --no-toc-restore;
                --strip-duplicates
            >
            $<$<CONFIG:ReleaseForArt>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PSVita")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ContentPackage>:
                /INCREMENTAL
            >
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /INCREMENTAL
            >
            $<$<CONFIG:ReleaseForArt>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            --strip-duplicates
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ContentPackage>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /INCREMENTAL
            >
            $<$<CONFIG:ReleaseForArt>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:NOREF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Xbox 360")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:ContentPackage_NO_TU>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ContentPackage>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:REF;
                /LTCG;
                /OPT:NOICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ReleaseForArt>:
                /NODEFAULTLIB:xapilib.lib;
                /OPT:REF;
                /LTCG;
                /OPT:NOICF;
                /INCREMENTAL:NO
            >
            /DEBUG
        )
    endif()
endif()

################################################################################
# Pre build events
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
    add_custom_command_if(
        TARGET ${PROJECT_NAME}
        PRE_BUILD
        COMMANDS
        COMMAND   $<CONFIG:ContentPackage_NO_TU> call $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>\\DurangoBuild\\AppxPrebuild.cmd $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>
        COMMAND $<CONFIG:CONTENTPACKAGE_SYMBOLS> call $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>\\DurangoBuild\\AppxPrebuild.cmd $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>
        COMMAND         $<CONFIG:ContentPackage> call $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>\\Build\\XboxOne\\AppxPrebuild.cmd $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>
        COMMAND                  $<CONFIG:Debug> call $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>\\Build\\XboxOne\\AppxPrebuild.cmd $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>
        COMMAND                $<CONFIG:Release> call $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>\\Build\\XboxOne\\AppxPrebuild.cmd $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>
    )
endif()

################################################################################
# Post build events
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
    add_custom_command_if(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMANDS
        COMMAND   $<CONFIG:ContentPackage_NO_TU> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\res $ENV{LayoutDir}Image\\Loose\\Common\\res
        COMMAND $<CONFIG:CONTENTPACKAGE_SYMBOLS> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\res $ENV{LayoutDir}Image\\Loose\\Common\\res
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\res $ENV{LayoutDir}Image\\Loose\\Common\\res
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\media\\font\\*.ttf $ENV{LayoutDir}Image\\Loose\\Common\\media\\font
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\*.png $ENV{LayoutDir}Image\\Loose
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\media\\MediaDurango.arc $ENV{LayoutDir}Image\\Loose\\Common\\media
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\Sound $ENV{LayoutDir}Image\\Loose\\Sound
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>music $ENV{LayoutDir}Image\\Loose\\music
        COMMAND         $<CONFIG:ContentPackage> copy /B /Y $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DurangoExtras\\xcompress.dll $ENV{LayoutDir}Image\\Loose\\
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DLCImages\\*.png $ENV{LayoutDir}Image\\Loose\\DLCImages\\
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DLCXbox1.cmp  $ENV{LayoutDir}Image\\Loose
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>DurangoMedia\\DLC  $ENV{LayoutDir}Image\\Loose\\DLC
        COMMAND         $<CONFIG:ContentPackage> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\CU  $ENV{LayoutDir}Image\\Loose\\CU
        COMMAND                  $<CONFIG:Debug> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\res $ENV{LayoutDir}Image\\Loose\\Common\\res
        COMMAND                  $<CONFIG:Debug> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\media\\font\\*.ttf $ENV{LayoutDir}Image\\Loose\\Common\\media\\font
        COMMAND                  $<CONFIG:Debug> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\*.png $ENV{LayoutDir}Image\\Loose
        COMMAND                  $<CONFIG:Debug> xcopy /q /y $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\media\\MediaDurango.arc $ENV{LayoutDir}Image\\Loose\\Common\\media
        COMMAND                  $<CONFIG:Debug> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\Sound $ENV{LayoutDir}Image\\Loose\\Sound
        COMMAND                  $<CONFIG:Debug> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>music $ENV{LayoutDir}Image\\Loose\\music
        COMMAND                  $<CONFIG:Debug> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>DurangoMedia\\DLC $ENV{LayoutDir}Image\\Loose\\DLC
        COMMAND                  $<CONFIG:Debug> copy /B /Y $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DurangoExtras\\xcompress.dll $ENV{LayoutDir}Image\\Loose\\
        COMMAND                  $<CONFIG:Debug> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DLCImages\\*.png $ENV{LayoutDir}Image\\Loose\\DLCImages\\
        COMMAND                  $<CONFIG:Debug> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DLCXbox1.cmp  $ENV{LayoutDir}Image\\Loose
        COMMAND                  $<CONFIG:Debug> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>DurangoMedia\\DLC  $ENV{LayoutDir}Image\\Loose\\DLC
        COMMAND                  $<CONFIG:Debug> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\CU  $ENV{LayoutDir}Image\\Loose\\CU
        COMMAND                $<CONFIG:Release> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\res $ENV{LayoutDir}Image\\Loose\\Common\\res
        COMMAND                $<CONFIG:Release> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\media\\font\\*.ttf $ENV{LayoutDir}Image\\Loose\\Common\\media\\font
        COMMAND                $<CONFIG:Release> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\*.png $ENV{LayoutDir}Image\\Loose
        COMMAND                $<CONFIG:Release> xcopy /q /y $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Common\\media\\MediaDurango.arc $ENV{LayoutDir}Image\\Loose\\Common\\media
        COMMAND                $<CONFIG:Release> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\Sound $ENV{LayoutDir}Image\\Loose\\Sound
        COMMAND                $<CONFIG:Release> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>music $ENV{LayoutDir}Image\\Loose\\music
        COMMAND                $<CONFIG:Release> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>DurangoMedia\\DLC $ENV{LayoutDir}Image\\Loose\\DLC
        COMMAND                $<CONFIG:Release> copy /B /Y $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DurangoExtras\\xcompress.dll $ENV{LayoutDir}Image\\Loose\\
        COMMAND                $<CONFIG:Release> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DLCImages\\*.png $ENV{LayoutDir}Image\\Loose\\DLCImages\\
        COMMAND                $<CONFIG:Release> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\DLCXbox1.cmp  $ENV{LayoutDir}Image\\Loose
        COMMAND                $<CONFIG:Release> xcopy /q /y  $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>DurangoMedia\\DLC  $ENV{LayoutDir}Image\\Loose\\DLC
        COMMAND                $<CONFIG:Release> xcopy /q /y /i /s /e $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>Durango\\CU  $ENV{LayoutDir}Image\\Loose\\CU
        COMMENT "Copying files for deployment"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PSVita")
    add_custom_command_if(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMANDS
        COMMAND         $<CONFIG:ContentPackage> xcopy /I /Y "$ENV{SCE_PSP2_SDK_DIR}\\target\\sce_module" "${OUTPUT_DIRECTORY}\\sce_module\\"
        COMMAND         $<CONFIG:ContentPackage> if not exist "${OUTPUT_DIRECTORY}\\savedata" mkdir "${OUTPUT_DIRECTORY}\\savedata"
        COMMAND                  $<CONFIG:Debug> xcopy /I /Y "$ENV{SCE_PSP2_SDK_DIR}\\target\\sce_module" "${OUTPUT_DIRECTORY}\\sce_module\\"
        COMMAND                  $<CONFIG:Debug> if not exist "${OUTPUT_DIRECTORY}\\savedata" mkdir "${OUTPUT_DIRECTORY}\\savedata"
        COMMAND                $<CONFIG:Release> xcopy /I /Y "$ENV{SCE_PSP2_SDK_DIR}\\target\\sce_module" "${OUTPUT_DIRECTORY}\\sce_module\\"
        COMMAND                $<CONFIG:Release> if not exist "${OUTPUT_DIRECTORY}\\savedata" mkdir "${OUTPUT_DIRECTORY}\\savedata"
        COMMENT "Copying files for deployment"
    )
endif()

################################################################################
# Custom build events
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
    add_custom_command_if(
        OUTPUT "$(ProjectDir)\Durango\Autogenerated.appxmanifest"
        COMMANDS
        COMMAND                  $<CONFIG:Debug> call $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>\\DurangoBuild\\AppxPrebuild.cmd $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>
        COMMENT "Creating Autogenerated.appxmanifest"
    )
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    Minecraft.World
)

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Durango")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:ContentPackage_NO_TU>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xboxkrnl;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xonline;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson"
        ">"
        "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xboxkrnl;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xonline;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson"
        ">"
        "$<$<CONFIG:ContentPackage>:"
            "ws2_32;"
            "d3d11_x;"
            "combase;"
            "kernelx;"
            "uuid;"
            "..\Minecraft.World\Durango_ContentPackage\Minecraft.World;"
            "EtwPlus;"
            "..\Minecraft.Client\Durango\DurangoExtras\xcompress"
        ">"
        "$<$<CONFIG:Debug>:"
            "ws2_32;"
            "pixEvt;"
            "d3d11_x;"
            "combase;"
            "kernelx;"
            "uuid;"
            "..\Minecraft.World\Durango_Debug\Minecraft.World;"
            "EtwPlus;"
            "..\Minecraft.Client\Durango\DurangoExtras\xcompress"
        ">"
        "$<$<CONFIG:Release>:"
            "ws2_32;"
            "pixEvt;"
            "d3d11_x;"
            "combase;"
            "kernelx;"
            "uuid;"
            "..\Minecraft.World\Durango_Release\Minecraft.World;"
            "EtwPlus;"
            "..\Minecraft.Client\Durango\DurangoExtras\xcompress"
        ">"
        "$<$<CONFIG:ReleaseForArt>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xboxkrnl;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xonline;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson"
        ">"
        "xaudio2"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Linux")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES "Minecraft.World")
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ORBIS")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
            "..\ORBIS_ContentPackage\Minecraft.World.a;"
            "Orbis\4JLibs\libs\4j_Render.a;"
            "Orbis\4JLibs\libs\4j_Input.a;"
            "Orbis\4JLibs\libs\4J_Storage.a;"
            "Orbis\4JLibs\libs\4J_Profile.a;"
            "Orbis\Iggy\lib\libiggy_orbis.a;"
            "Orbis\Miles\lib\mssorbis.a;"
            "Orbis\Miles\lib\binkaorbis.a;"
            "Common\Network\Sony\sceRemoteStorage\ps4\lib\sceRemoteStorage.a;"
            "-lSceGnmDriver_stub_weak;"
            "-lSceGnmx;"
            "-lSceGnm;"
            "-lSceGpuAddress;"
            "-lSceCes;"
            "-lSceVideoOut_stub_weak;"
            "-lScePad_stub_weak;"
            "-lScePngDec_stub_weak;"
            "-lScePngEnc_stub_weak;"
            "-lSceFios2_stub_weak;"
            "-lSceUlt_stub_weak;"
            "-lSceShaderBinary;"
            "-lSceUserService_stub_weak;"
            "-lSceSysmodule_stub_weak;"
            "-lSceImeDialog_stub_weak;"
            "-lScePosix_stub_weak;"
            "-lSceAudioOut_stub_weak;"
            "-lSceSaveData_stub_weak;"
            "-lSceRtc_stub_weak;"
            "-lSceSystemService_stub_weak;"
            "-lSceNetCtl_stub_weak;"
            "-lSceNpCommon_stub_weak;"
            "-lSceNpManager_stub_weak;"
            "-lSceNpToolkit_rtti;"
            "-lSceNpToolkitUtils_rtti;"
            "-lSceNpWebApi_stub_weak;"
            "-lSceNpAuth_stub_weak;"
            "-lSceNpTrophy_stub_weak;"
            "-lSceInvitationDialog_stub_weak;"
            "-lSceGameCustomDataDialog_stub_weak;"
            "-lSceNpCommerce_stub_weak;"
            "-lSceNet_stub_weak;"
            "-lSceHttp_stub_weak;"
            "-lSceSsl_stub_weak;"
            "-lSceNpMatching2_stub_weak;"
            "-lSceNpTus_stub_weak;"
            "-lSceNpUtility_stub_weak;"
            "-lSceNpScore_stub_weak;"
            "-lSceCommonDialog_stub_weak;"
            "-lSceNpSns_stub_weak;"
            "-lSceRudp_stub_weak;"
            "-lSceAppContent_stub_weak;"
            "-lSceVoice_stub_weak;"
            "-lSceAudioIn_stub_weak;"
            "-lSceRemoteplay_stub_weak;"
            "-lSceSaveDataDialog_stub_weak"
        ">"
        "$<$<CONFIG:ContentPackage>:"
            "..\ORBIS_ContentPackage\Minecraft.World.a;"
            "Orbis\4JLibs\libs\4j_Render.a;"
            "Orbis\4JLibs\libs\4j_Input.a;"
            "Orbis\4JLibs\libs\4J_Storage.a;"
            "Orbis\4JLibs\libs\4J_Profile.a;"
            "Orbis\Iggy\lib\libiggy_orbis.a;"
            "Orbis\Miles\lib\mssorbis.a;"
            "Orbis\Miles\lib\binkaorbis.a;"
            "Common\Network\Sony\sceRemoteStorage\ps4\lib\sceRemoteStorage.a;"
            "-lSceGnmDriver_stub_weak;"
            "-lSceGnmx;"
            "-lSceGnm;"
            "-lSceGpuAddress;"
            "-lSceCes;"
            "-lSceVideoOut_stub_weak;"
            "-lScePad_stub_weak;"
            "-lScePngDec_stub_weak;"
            "-lScePngEnc_stub_weak;"
            "-lSceFios2_stub_weak;"
            "-lSceUlt_stub_weak;"
            "-lSceShaderBinary;"
            "-lSceUserService_stub_weak;"
            "-lSceSysmodule_stub_weak;"
            "-lSceImeDialog_stub_weak;"
            "-lScePosix_stub_weak;"
            "-lSceAudioOut_stub_weak;"
            "-lSceSaveData_stub_weak;"
            "-lSceRtc_stub_weak;"
            "-lSceSystemService_stub_weak;"
            "-lSceNetCtl_stub_weak;"
            "-lSceNpCommon_stub_weak;"
            "-lSceNpManager_stub_weak;"
            "-lSceNpToolkit_rtti;"
            "-lSceNpToolkitUtils_rtti;"
            "-lSceNpWebApi_stub_weak;"
            "-lSceNpAuth_stub_weak;"
            "-lSceNpTrophy_stub_weak;"
            "-lSceInvitationDialog_stub_weak;"
            "-lSceGameCustomDataDialog_stub_weak;"
            "-lSceNpCommerce_stub_weak;"
            "-lSceNet_stub_weak;"
            "-lSceHttp_stub_weak;"
            "-lSceSsl_stub_weak;"
            "-lSceNpMatching2_stub_weak;"
            "-lSceNpTus_stub_weak;"
            "-lSceNpUtility_stub_weak;"
            "-lSceNpScore_stub_weak;"
            "-lSceCommonDialog_stub_weak;"
            "-lSceNpSns_stub_weak;"
            "-lSceRudp_stub_weak;"
            "-lSceAppContent_stub_weak;"
            "-lSceVoice_stub_weak;"
            "-lSceAudioIn_stub_weak;"
            "-lSceRemoteplay_stub_weak;"
            "-lSceSaveDataDialog_stub_weak;"
            "-lSceNpSnsFacebookDialog_stub_weak;"
            "-lSceErrorDialog_stub_weak;"
            "-lSceMsgDialog_stub_weak;"
            "-lSceGameLiveStreaming_stub_weak"
        ">"
        "$<$<CONFIG:Debug>:"
            "..\Minecraft.World\ORBIS_Debug\Minecraft.World.a;"
            "Orbis\4JLibs\libs\4j_Render_d.a;"
            "Orbis\4JLibs\libs\4j_Input_d.a;"
            "Orbis\4JLibs\libs\4J_Storage_d.a;"
            "Orbis\4JLibs\libs\4J_Profile_d.a;"
            "Orbis\Iggy\lib\libiggy_orbis.a;"
            "Orbis\Miles\lib\mssorbis.a;"
            "Orbis\Miles\lib\binkaorbis.a;"
            "Common\Network\Sony\sceRemoteStorage\ps4\lib\sceRemoteStorage.a;"
            "-lSceGnmDriver_stub_weak;"
            "-lSceGnmx;"
            "-lSceGnm;"
            "-lSceGpuAddress;"
            "-lSceCes;"
            "-lSceVideoOut_stub_weak;"
            "-lScePad_stub_weak;"
            "-lScePngDec_stub_weak;"
            "-lScePngEnc_stub_weak;"
            "-lSceFios2_stub_weak;"
            "-lSceUlt_stub_weak;"
            "-lSceShaderBinary;"
            "-lSceUserService_stub_weak;"
            "-lSceSysmodule_stub_weak;"
            "-lScePerf_stub_weak;"
            "-lSceImeDialog_stub_weak;"
            "-lScePosix_stub_weak;"
            "-lSceAudioOut_stub_weak;"
            "-lSceSaveData_stub_weak;"
            "-lSceRtc_stub_weak;"
            "-lSceSystemService_stub_weak;"
            "-lSceNetCtl_stub_weak;"
            "-lSceNpCommon_stub_weak;"
            "-lSceNpManager_stub_weak;"
            "-lSceNpToolkit_rtti;"
            "-lSceNpToolkitUtils_rtti;"
            "-lSceNpWebApi_stub_weak;"
            "-lSceNpAuth_stub_weak;"
            "-lSceNpTrophy_stub_weak;"
            "-lSceInvitationDialog_stub_weak;"
            "-lSceGameCustomDataDialog_stub_weak;"
            "-lSceNpCommerce_stub_weak;"
            "-lSceNet_stub_weak;"
            "-lSceHttp_stub_weak;"
            "-lSceSsl_stub_weak;"
            "-lSceNpMatching2_stub_weak;"
            "-lSceNpTus_stub_weak;"
            "-lSceNpUtility_stub_weak;"
            "-lSceNpScore_stub_weak;"
            "-lSceCommonDialog_stub_weak;"
            "-lSceNpSns_stub_weak;"
            "-lSceRudp_stub_weak;"
            "-lSceAppContent_stub_weak;"
            "-lSceVoice_stub_weak;"
            "-lSceAudioIn_stub_weak;"
            "-lSceNpSnsFacebookDialog_stub_weak;"
            "-lSceRemotePlay_stub_weak;"
            "-lSceSaveDataDialog_stub_weak;"
            "-lSceErrorDialog_stub_weak;"
            "-lSceMsgDialog_stub_weak;"
            "-lSceGameLiveStreaming_stub_weak"
        ">"
        "$<$<CONFIG:Release>:"
            "..\Minecraft.World\ORBIS_Release\Minecraft.World.a;"
            "Orbis\4JLibs\libs\4j_Render.a;"
            "Orbis\4JLibs\libs\4j_Input_r.a;"
            "Orbis\4JLibs\libs\4J_Storage_r.a;"
            "Orbis\4JLibs\libs\4J_Profile_r.a;"
            "Orbis\Iggy\lib\libiggy_orbis.a;"
            "Orbis\Miles\lib\mssorbis.a;"
            "Orbis\Miles\lib\binkaorbis.a;"
            "Common\Network\Sony\sceRemoteStorage\ps4\lib\sceRemoteStorage.a;"
            "-lSceGnmDriver_stub_weak;"
            "-lSceGnmx;"
            "-lSceGnm;"
            "-lSceGpuAddress;"
            "-lSceCes;"
            "-lSceVideoOut_stub_weak;"
            "-lScePad_stub_weak;"
            "-lScePngDec_stub_weak;"
            "-lScePngEnc_stub_weak;"
            "-lSceFios2_stub_weak;"
            "-lSceUlt_stub_weak;"
            "-lSceShaderBinary;"
            "-lSceUserService_stub_weak;"
            "-lSceSysmodule_stub_weak;"
            "-lSceImeDialog_stub_weak;"
            "-lScePosix_stub_weak;"
            "-lSceAudioOut_stub_weak;"
            "-lSceSaveData_stub_weak;"
            "-lSceRtc_stub_weak;"
            "-lSceSystemService_stub_weak;"
            "-lSceNetCtl_stub_weak;"
            "-lSceNpCommon_stub_weak;"
            "-lSceNpManager_stub_weak;"
            "-lSceNpToolkit_rtti;"
            "-lSceNpToolkitUtils_rtti;"
            "-lSceNpWebApi_stub_weak;"
            "-lSceNpAuth_stub_weak;"
            "-lSceNpTrophy_stub_weak;"
            "-lSceInvitationDialog_stub_weak;"
            "-lSceGameCustomDataDialog_stub_weak;"
            "-lSceNpCommerce_stub_weak;"
            "-lSceNet_stub_weak;"
            "-lSceHttp_stub_weak;"
            "-lSceSsl_stub_weak;"
            "-lSceNpMatching2_stub_weak;"
            "-lSceNpTus_stub_weak;"
            "-lSceNpUtility_stub_weak;"
            "-lSceNpScore_stub_weak;"
            "-lSceCommonDialog_stub_weak;"
            "-lSceNpSns_stub_weak;"
            "-lSceNpSnsFacebookDialog_stub_weak;"
            "-lSceRudp_stub_weak;"
            "-lSceAppContent_stub_weak;"
            "-lSceVoice_stub_weak;"
            "-lSceAudioIn_stub_weak;"
            "-lSceRemoteplay_stub_weak;"
            "-lSceSaveDataDialog_stub_weak;"
            "-lSceErrorDialog_stub_weak;"
            "-lSceMsgDialog_stub_weak;"
            "-lSceGameLiveStreaming_stub_weak"
        ">"
        "$<$<CONFIG:ReleaseForArt>:"
            "..\Minecraft.World\ORBIS_ReleaseForArt\Minecraft.World.a;"
            "Orbis\4JLibs\libs\4j_Render.a;"
            "Orbis\4JLibs\libs\4j_Input_r.a;"
            "Orbis\4JLibs\libs\4J_Storage_r.a;"
            "Orbis\4JLibs\libs\4J_Profile_r.a;"
            "Orbis\Iggy\lib\libiggy_orbis.a;"
            "Orbis\Miles\lib\mssorbis.a;"
            "Orbis\Miles\lib\binkaorbis.a;"
            "Common\Network\Sony\sceRemoteStorage\ps4\lib\sceRemoteStorage.a;"
            "-lSceGnmDriver_stub_weak;"
            "-lSceGnmx;"
            "-lSceGnm;"
            "-lSceGpuAddress;"
            "-lSceCes;"
            "-lSceVideoOut_stub_weak;"
            "-lScePad_stub_weak;"
            "-lScePngDec_stub_weak;"
            "-lScePngEnc_stub_weak;"
            "-lSceFios2_stub_weak;"
            "-lSceUlt_stub_weak;"
            "-lSceShaderBinary;"
            "-lSceUserService_stub_weak;"
            "-lSceSysmodule_stub_weak;"
            "-lSceImeDialog_stub_weak;"
            "-lScePosix_stub_weak;"
            "-lSceAudioOut_stub_weak;"
            "-lSceSaveData_stub_weak;"
            "-lSceRtc_stub_weak;"
            "-lSceSystemService_stub_weak;"
            "-lSceNetCtl_stub_weak;"
            "-lSceNpCommon_stub_weak;"
            "-lSceNpManager_stub_weak;"
            "-lSceNpToolkit_rtti;"
            "-lSceNpToolkitUtils_rtti;"
            "-lSceNpWebApi_stub_weak;"
            "-lSceNpAuth_stub_weak;"
            "-lSceNpTrophy_stub_weak;"
            "-lSceInvitationDialog_stub_weak;"
            "-lSceGameCustomDataDialog_stub_weak;"
            "-lSceNpCommerce_stub_weak;"
            "-lSceNet_stub_weak;"
            "-lSceHttp_stub_weak;"
            "-lSceSsl_stub_weak;"
            "-lSceNpMatching2_stub_weak;"
            "-lSceNpTus_stub_weak;"
            "-lSceNpUtility_stub_weak;"
            "-lSceNpScore_stub_weak;"
            "-lSceCommonDialog_stub_weak;"
            "-lSceNpSns_stub_weak;"
            "-lSceNpSnsFacebookDialog_stub_weak;"
            "-lSceRudp_stub_weak;"
            "-lSceAppContent_stub_weak;"
            "-lSceVoice_stub_weak;"
            "-lSceAudioIn_stub_weak;"
            "-lSceRemoteplay_stub_weak;"
            "-lSceSaveDataDialog_stub_weak;"
            "-lSceErrorDialog_stub_weak;"
            "-lSceMsgDialog_stub_weak"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PS3")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:ContentPackage_NO_TU>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xboxkrnl;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xonline;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson"
        ">"
        "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
            "$(OutDir)Minecraft.World.a;"
            "ps3\4JLibs\libs\4j_Render.a;"
            "ps3\4JLibs\libs\4j_Input.a;"
            "ps3\4JLibs\libs\4j_Storage.a;"
            "ps3\4JLibs\libs\4j_Profile.a;"
            "ps3\Miles\lib\mssps3.a;"
            "ps3\Miles\lib\audps3.a;"
            "ps3\Miles\lib\spu\mssppu_spurs.a;"
            "ps3\Miles\lib\BinkAPS3.A;"
            "PS3\Iggy\lib\libiggy_ps3.a;"
            "ps3\Miles\lib\mssspurs.o;"
            "ps3\Edge\lib\libedgezlib.a;"
            "Common\Network\Sony\sceRemoteStorage\ps3\lib\sceRemoteStorage.a;"
            "libsntuner.a;"
            "libpngdec_stub.a;"
            "libpngenc_stub.a;"
            "libnet_stub.a;"
            "libsysutil_savedata_stub.a;"
            "libsysutil_userinfo_stub.a;"
            "libsysutil_np_trophy_stub.a;"
            "libsysutil_game_stub.a;"
            "libhttp_stub.a;"
            "libhttp_util_stub.a;"
            "libssl_stub.a;"
            "libjpgdec_stub.a;"
            "libjpgenc_stub.a;"
            "libsysutil_avc2_stub.a;"
            "libsysutil_np_commerce2_stub.a;"
            "libsysutil_avconf_ext_stub.a;"
            "libsysutil_screenshot_stub.a;"
            "libsysutil_np_tus_stub.a;"
            "-lresc_stub;"
            "-lgcm_cmd;"
            "-lgcm_sys_stub;"
            "-lsysmodule_stub;"
            "-lm;"
            "-lsysutil_stub;"
            "-lio_stub;"
            "-ldbgfont_gcm;"
            "-lpthread;"
            "-lpadfilter;"
            "-lcgb;"
            "-laudio_stub;"
            "-lfs_stub;"
            "-lspurs_stub;"
            "-lspurs_jq_stub;"
            "-lrtc_stub;"
            "-lsysutil_oskdialog_ext_stub;"
            "-ll10n_stub;"
            "-lsysutil_np_stub;"
            "-lsysutil_np2_stub;"
            "-lnetctl_stub;"
            "-lnet_stub;"
            "-lrudp_stub;"
            "-lsysutil_avconf_ext_stub"
        ">"
        "$<$<CONFIG:ContentPackage>:"
            "$(OutDir)Minecraft.World.a;"
            "ps3\4JLibs\libs\4j_Render.a;"
            "ps3\4JLibs\libs\4j_Input.a;"
            "ps3\4JLibs\libs\4j_Storage.a;"
            "ps3\4JLibs\libs\4j_Profile.a;"
            "ps3\Miles\lib\mssps3.a;"
            "ps3\Miles\lib\audps3.a;"
            "ps3\Miles\lib\spu\mssppu_spurs.a;"
            "ps3\Miles\lib\BinkAPS3.A;"
            "PS3\Iggy\lib\libiggy_ps3.a;"
            "ps3\Miles\lib\mssspurs.o;"
            "ps3\Edge\lib\libedgezlib.a;"
            "Common\Network\Sony\sceRemoteStorage\ps3\lib\sceRemoteStorage.a;"
            "libsntuner.a;"
            "libpngdec_stub.a;"
            "libpngenc_stub.a;"
            "libnet_stub.a;"
            "libsysutil_savedata_stub.a;"
            "libsysutil_userinfo_stub.a;"
            "libsysutil_np_trophy_stub.a;"
            "libsysutil_game_stub.a;"
            "libhttp_stub.a;"
            "libhttp_util_stub.a;"
            "libssl_stub.a;"
            "libjpgdec_stub.a;"
            "libjpgenc_stub.a;"
            "libsysutil_avc2_stub.a;"
            "libsysutil_np_commerce2_stub.a;"
            "libsysutil_avconf_ext_stub.a;"
            "libsysutil_screenshot_stub.a;"
            "libsysutil_np_tus_stub.a;"
            "-lresc_stub;"
            "-lgcm_cmd;"
            "-lgcm_sys_stub;"
            "-lsysmodule_stub;"
            "-lm;"
            "-lsysutil_stub;"
            "-lio_stub;"
            "-ldbgfont_gcm;"
            "-lpthread;"
            "-lpadfilter;"
            "-lcgb;"
            "-laudio_stub;"
            "-lfs_stub;"
            "-lspurs_stub;"
            "-lspurs_jq_stub;"
            "-lrtc_stub;"
            "-lsysutil_oskdialog_ext_stub;"
            "-ll10n_stub;"
            "-lsysutil_np_stub;"
            "-lsysutil_np2_stub;"
            "-lnetctl_stub;"
            "-lnet_stub;"
            "-lrudp_stub;"
            "-lsysutil_avconf_ext_stub"
        ">"
        "$<$<CONFIG:Debug>:"
            "$(OutDir)Minecraft.World.a;"
            "ps3\4JLibs\libs\4j_Render_d.a;"
            "ps3\4JLibs\libs\4j_Input_d.a;"
            "ps3\4JLibs\libs\4j_Storage_d.a;"
            "ps3\4JLibs\libs\4j_Profile_d.a;"
            "ps3\Miles\lib\mssps3.a;"
            "ps3\Miles\lib\mssspurs.o;"
            "ps3\Miles\lib\audps3.a;"
            "ps3\Miles\lib\BinkAPS3.A;"
            "ps3\Miles\lib\spu\mssppu_spurs.a;"
            "PS3\Iggy\lib\libiggy_ps3.a;"
            "ps3\Edge\lib\libedgezlib_dbg.a;"
            "Common\Network\Sony\sceRemoteStorage\ps3\lib\sceRemoteStorage.a;"
            "PS3\PS3Extras\HeapInspector\Server\PS3\Debug_RTTI_EH\libHeapInspectorServer.a;"
            "libsntuner.a;"
            "libpngdec_stub.a;"
            "libpngenc_stub.a;"
            "libjpgdec_stub.a;"
            "libjpgenc_stub.a;"
            "libnet_stub.a;"
            "libsysutil_savedata_stub.a;"
            "libsysutil_userinfo_stub.a;"
            "libsysutil_np_trophy_stub.a;"
            "libsysutil_game_stub.a;"
            "libsysutil_avc2_stub.a;"
            "libsysutil_np_commerce2_stub.a;"
            "libsysutil_avconf_ext_stub.a;"
            "libhttp_stub.a;"
            "libhttp_util_stub.a;"
            "libssl_stub.a;"
            "libsysutil_screenshot_stub.a;"
            "libsysutil_np_tus_stub.a;"
            "-lresc_stub;"
            "-lgcm_cmddbg;"
            "-lgcm_sys_stub;"
            "-lsysmodule_stub;"
            "-lm;"
            "-lsysutil_stub;"
            "-lio_stub;"
            "-ldbgfont_gcm;"
            "-lpthread;"
            "-lpadfilter;"
            "-lcgb;"
            "-laudio_stub;"
            "-lfs_stub;"
            "-lspurs_stub;"
            "-lspurs_jq_stub;"
            "-lrtc_stub;"
            "-lsysutil_oskdialog_ext_stub;"
            "-ll10n_stub;"
            "-lsysutil_np_stub;"
            "-lsysutil_np2_stub;"
            "-lnetctl_stub;"
            "-lnet_stub;"
            "-lrudp_stub"
        ">"
        "$<$<CONFIG:Release>:"
            "$(OutDir)Minecraft.World.a;"
            "ps3\Miles\lib\mssps3.a;"
            "ps3\Miles\lib\mssspurs.o;"
            "ps3\Miles\lib\audps3.a;"
            "ps3\Miles\lib\BinkAPS3.A;"
            "ps3\Miles\lib\spu\mssppu_spurs.a;"
            "PS3\Iggy\lib\libiggy_ps3.a;"
            "ps3\Edge\lib\libedgezlib.a;"
            "Common\Network\Sony\sceRemoteStorage\ps3\lib\sceRemoteStorage.a;"
            "PS3\PS3Extras\HeapInspector\Server\PS3\Debug_RTTI_EH\libHeapInspectorServer.a;"
            "libsntuner.a;"
            "libpngdec_stub.a;"
            "libpngenc_stub.a;"
            "libjpgdec_stub.a;"
            "libjpgenc_stub.a;"
            "libnet_stub.a;"
            "libsysutil_savedata_stub.a;"
            "libsysutil_userinfo_stub.a;"
            "libsysutil_np_trophy_stub.a;"
            "libsysutil_game_stub.a;"
            "libsysutil_avc2_stub.a;"
            "libsysutil_np_commerce2_stub.a;"
            "libsysutil_avconf_ext_stub.a;"
            "libhttp_stub.a;"
            "libhttp_util_stub.a;"
            "libssl_stub.a;"
            "libsysutil_screenshot_stub.a;"
            "libsysutil_np_tus_stub.a;"
            "-lresc_stub;"
            "-lgcm_cmd;"
            "-lgcm_sys_stub;"
            "-lsysmodule_stub;"
            "-lm;"
            "-lsysutil_stub;"
            "-lio_stub;"
            "-ldbgfont_gcm;"
            "-lpthread;"
            "-lpadfilter;"
            "-lcgb;"
            "-laudio_stub;"
            "-lfs_stub;"
            "-lspurs_stub;"
            "-lspurs_jq_stub;"
            "-lrtc_stub;"
            "-lsysutil_oskdialog_ext_stub;"
            "-ll10n_stub;"
            "-lsysutil_np_stub;"
            "-lsysutil_np2_stub;"
            "-lnetctl_stub;"
            "-lnet_stub;"
            "-lrudp_stub"
        ">"
        "$<$<CONFIG:ReleaseForArt>:"
            "$(OutDir)Minecraft.World.a;"
            "ps3\4JLibs\libs\4j_Render_r.a;"
            "ps3\4JLibs\libs\4j_Input_r.a;"
            "ps3\4JLibs\libs\4j_Storage_r.a;"
            "ps3\4JLibs\libs\4j_Profile_r.a;"
            "ps3\Miles\lib\mssps3.a;"
            "ps3\Miles\lib\mssspurs.o;"
            "ps3\Miles\lib\audps3.a;"
            "ps3\Miles\lib\BinkAPS3.A;"
            "ps3\Miles\lib\spu\mssppu_spurs.a;"
            "PS3\Iggy\lib\libiggy_ps3.a;"
            "Common\Network\Sony\sceRemoteStorage\ps3\lib\sceRemoteStorage.a;"
            "PS3\PS3Extras\HeapInspector\Server\PS3\Release_RTTI_EH\libHeapInspectorServer.a;"
            "libsntuner.a;"
            "libpngdec_stub.a;"
            "libpngenc_stub.a;"
            "libjpgdec_stub.a;"
            "libjpgenc_stub.a;"
            "libnet_stub.a;"
            "libedgezlib_dbg.a;"
            "libsysutil_savedata_stub.a;"
            "libsysutil_userinfo_stub.a;"
            "libsysutil_np_trophy_stub.a;"
            "libsysutil_game_stub.a;"
            "libsysutil_avc2_stub.a;"
            "libsysutil_np_commerce2_stub.a;"
            "libsysutil_avconf_ext_stub.a;"
            "libhttp_stub.a;"
            "libhttp_util_stub.a;"
            "libssl_stub.a;"
            "libsysutil_screenshot_stub.a;"
            "libsysutil_np_tus_stub.a;"
            "-lresc_stub;"
            "-lgcm_cmd;"
            "-lgcm_sys_stub;"
            "-lsysmodule_stub;"
            "-lm;"
            "-lsysutil_stub;"
            "-lio_stub;"
            "-ldbgfont_gcm;"
            "-lpthread;"
            "-lpadfilter;"
            "-lcgb;"
            "-laudio_stub;"
            "-lfs_stub;"
            "-lspurs_stub;"
            "-lspurs_jq_stub;"
            "-lrtc_stub;"
            "-lsysutil_oskdialog_ext_stub;"
            "-ll10n_stub;"
            "-lsysutil_np_stub;"
            "-lsysutil_np2_stub;"
            "-lnetctl_stub;"
            "-lnet_stub;"
            "-lrudp_stub"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "PSVita")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:ContentPackage>:"
            "-lSceGxm_stub;"
            "-lSceAppUtil_stub;"
            "-lSceCommonDialog_stub;"
            "-lSceDisplay_stub;"
            "-lSceTouch_stub;"
            "-lSceCtrl_stub;"
            "-lSceAudio_stub;"
            "-lSceSysmodule_stub;"
            "-lSceDeflt;"
            "-lScePng;"
            "libSceRtc_stub.a;"
            "libSceFios2_stub_weak.a;"
            "libSceCes.a;"
            "libScePerf_stub.a;"
            "libScePerf_stub_weak.a;"
            "libSceUlt_stub.a;"
            "libSceUlt_stub_weak.a;"
            "libSceHttp_stub.a;"
            "libSceNet_stub.a;"
            "libSceSsl_stub.a;"
            "libSceNetCtl_stub.a;"
            "libSceNpManager_stub.a;"
            "libSceNpBasic_stub.a;"
            "libSceNpCommon_stub.a;"
            "libSceNpUtility_stub.a;"
            "libSceNpMatching2_stub.a;"
            "libSceNpScore_stub.a;"
            "libSceNpToolkit.a;"
            "libSceNpToolkitUtils.a;"
            "libSceNpTrophy_stub.a;"
            "libSceRudp_stub_weak.a;"
            "libSceVoice_stub.a;"
            "libSceNetAdhocMatching_stub.a;"
            "libScePspnetAdhoc_stub.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\msspsp2.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\binkapsp2.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\msspsp2midi.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\fltpsp2.a;"
            "libSceAppMgr_stub.a;"
            "libSceSysmodule_stub.a;"
            "libSceCommonDialog_stub.a;"
            "libSceCtrl_stub.a;"
            "libSceGxm_stub.a;"
            "libSceDisplay_stub.a;"
            "libSceSystemGesture_stub.a;"
            "libSceTouch_stub.a;"
            "libSceFios2_stub.a;"
            "libSceAppUtil_stub.a;"
            "libSceNearUtil_stub.a;"
            "libScePower_stub.a;"
            "..\Minecraft.Client\PSVita\4JLibs\libs\4J_Input.a;"
            "..\Minecraft.Client\PSVita\4JLibs\libs\4J_Profile.a;"
            "..\Minecraft.Client\PSVita\4JLibs\libs\4J_Render.a;"
            "..\Minecraft.Client\PSVita\4JLibs\libs\4J_Storage.a;"
            "..\Minecraft.Client\Common\Network\Sony\sceRemoteStorage\psvita\lib\sceRemoteStorage.a"
        ">"
        "$<$<CONFIG:Debug>:"
            "-lSceDbg_stub;"
            "-lSceGxm_stub;"
            "-lSceAppUtil_stub;"
            "-lSceCommonDialog_stub;"
            "-lSceDisplay_stub;"
            "-lSceTouch_stub;"
            "-lSceCtrl_stub;"
            "-lSceAudio_stub;"
            "-lSceDbgFont;"
            "-lSceRazorCapture_stub_weak;"
            "-lSceSysmodule_stub;"
            "-lSceDeflt;"
            "-lScePng;"
            "libSceRtc_stub.a;"
            "libSceFios2_stub_weak.a;"
            "libSceCes.a;"
            "libScePerf_stub.a;"
            "libScePerf_stub_weak.a;"
            "libSceUlt_stub.a;"
            "libSceUlt_stub_weak.a;"
            "libSceNpManager_stub_weak.a;"
            "libSceNpCommon_stub_weak.a;"
            "libSceNpCommerce2_stub.a;"
            "libSceHttp_stub.a;"
            "libSceNpTrophy_stub.a;"
            "libSceNpScore_stub.a;"
            "libSceRudp_stub_weak.a;"
            "libSceVoice_stub.a;"
            "libSceNetAdhocMatching_stub.a;"
            "libScePspnetAdhoc_stub.a;"
            "libScePower_stub.a;"
            "libSceAppUtil_stub.a;"
            "libSceAppMgr_stub.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\msspsp2.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\binkapsp2.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\msspsp2midi.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\fltpsp2.a;"
            "..\Minecraft.Client\Common\Network\Sony\sceRemoteStorage\psvita\lib\sceRemoteStorage.a"
        ">"
        "$<$<CONFIG:Release>:"
            "-lSceDbg_stub;"
            "-lSceGxm_stub;"
            "-lSceAppUtil_stub;"
            "-lSceCommonDialog_stub;"
            "-lSceDisplay_stub;"
            "-lSceTouch_stub;"
            "-lSceCtrl_stub;"
            "-lSceAudio_stub;"
            "-lSceDbgFont;"
            "-lSceRazorCapture_stub_weak;"
            "-lSceSysmodule_stub;"
            "-lSceDeflt;"
            "-lScePng;"
            "libSceRtc_stub.a;"
            "libSceFios2_stub_weak.a;"
            "libSceCes.a;"
            "libScePerf_stub.a;"
            "libScePerf_stub_weak.a;"
            "libSceUlt_stub.a;"
            "libSceUlt_stub_weak.a;"
            "libSceNpManager_stub_weak.a;"
            "libSceNpCommon_stub_weak.a;"
            "libSceHttp_stub.a;"
            "libSceNpTrophy_stub.a;"
            "libSceNpScore_stub.a;"
            "libSceRudp_stub_weak.a;"
            "libSceVoice_stub.a;"
            "libSceNetAdhocMatching_stub.a;"
            "libScePspnetAdhoc_stub.a;"
            "libScePower_stub.a;"
            "libSceAppUtil_stub.a;"
            "libSceAppMgr_stub.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\msspsp2.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\binkapsp2.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\msspsp2midi.a;"
            "..\Minecraft.Client\PSVita\Miles\lib\fltpsp2.a;"
            "..\Minecraft.Client\Common\Network\Sony\sceRemoteStorage\psvita\lib\sceRemoteStorage.a"
        ">"
        "$(OutDir)Minecraft.World.a"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Windows64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:ContentPackage_NO_TU>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xboxkrnl;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xonline;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson"
        ">"
        "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xboxkrnl;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xonline;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson"
        ">"
        "$<$<CONFIG:ContentPackage>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xboxkrnl;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xonline;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson;"
            "xbox\4JLibs\libs\4J_XTMS_r"
        ">"
        "$<$<CONFIG:Debug>:"
            "d3d11;"
            "..\Minecraft.World\x64_Debug\Minecraft.World;"
            "XInput9_1_0;"
            "..\Minecraft.Client\Windows64\Miles\Lib\mss64"
        ">"
        "$<$<CONFIG:Release>:"
            "d3d11;"
            "..\Minecraft.World\x64_Release\Minecraft.World;"
            "XInput9_1_0;"
            "Windows64\Iggy\lib\iggy_w64"
        ">"
        "$<$<CONFIG:ReleaseForArt>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xboxkrnl;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xonline;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Xbox 360")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:ContentPackage_NO_TU>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage_NO_TU\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson;"
            "xbox\4JLibs\libs\4J_XTMS_r"
        ">"
        "$<$<CONFIG:CONTENTPACKAGE_SYMBOLS>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson;"
            "xtms"
        ">"
        "$<$<CONFIG:ContentPackage>:"
            "xavatar2;"
            "xapilib;"
            "d3d9;"
            "d3dx9;"
            "xgraphics;"
            "xbox\Sentient\libs\SenCore;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xhv2;"
            "qnetxaudio2;"
            "xbox\4JLibs\libs\4J_Input;"
            "xbox\4JLibs\libs\4J_Storage;"
            "xbox\4JLibs\libs\4J_Profile;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\ContentPackage\Minecraft.World;"
            "xsocialpost;"
            "xrnm;"
            "xparty;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson;"
            "xbox\4JLibs\libs\4J_XTMS_r"
        ">"
        "$<$<CONFIG:Debug>:"
            "xavatar2d;"
            "xapilibd;"
            "d3d9d;"
            "d3dx9d;"
            "xgraphicsd;"
            "xnetd;"
            "xaudiod2;"
            "xactd3;"
            "x3daudiod;"
            "xmcored;"
            "xbdm;"
            "vcompd;"
            "xuirund;"
            "xuirenderd;"
            "xuihtmld;"
            "xhvd2;"
            "qnetxaudio2d;"
            "xpartyd;"
            "..\Minecraft.World\Debug\Minecraft.World;"
            "xbox\4JLibs\libs\4J_Input_d;"
            "xbox\4JLibs\libs\4J_Storage_d;"
            "xbox\4JLibs\libs\4J_Profile_d;"
            "xbox\4JLibs\libs\4J_Render_d;"
            "xsocialpostd;"
            "xrnmd;"
            "xbox\Sentient\libs\SenCoreD;"
            "xbox\Sentient\libs\SenNewsD;"
            "xbox\Sentient\libs\SenUGCD;"
            "xbox\Sentient\libs\SenBoxArtD;"
            "nuiapid;"
            "STd;"
            "NuiFitnessApid;"
            "NuiHandlesd;"
            "NuiSpeechd;"
            "xhttpd;"
            "xauthd;"
            "xgetserviceendpointd;"
            "xavd;"
            "xjsond;"
            "xbox\4JLibs\libs\4J_XTMS_d"
        ">"
        "$<$<CONFIG:Release>:"
            "xavatar2;"
            "xapilibi;"
            "d3d9i;"
            "d3dx9;"
            "xgraphics;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xhv2;"
            "qnetxaudio2;"
            "xparty;"
            "xbox\4JLibs\libs\4J_Input_r;"
            "xbox\4JLibs\libs\4J_Storage_r;"
            "xbox\4JLibs\libs\4J_Profile_r;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\Release\Minecraft.World;"
            "xbdm;"
            "xsocialpost;"
            "xrnm;"
            "xbox\Sentient\libs\SenCore;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson;"
            "xbox\4JLibs\libs\4J_XTMS_r"
        ">"
        "$<$<CONFIG:ReleaseForArt>:"
            "xavatar2;"
            "xapilibi;"
            "d3d9i;"
            "d3dx9;"
            "xgraphics;"
            "xnet;"
            "xaudio2;"
            "xact3;"
            "x3daudio;"
            "xmcore;"
            "vcomp;"
            "xuirun;"
            "xuirender;"
            "xuihtml;"
            "xhv2;"
            "qnetxaudio2;"
            "xparty;"
            "xbox\4JLibs\libs\4J_Input_r;"
            "xbox\4JLibs\libs\4J_Storage_r;"
            "xbox\4JLibs\libs\4J_Profile_r;"
            "xbox\4JLibs\libs\4J_Render;"
            "..\Minecraft.World\Release\Minecraft.World;"
            "xbdm;"
            "xsocialpost;"
            "xrnm;"
            "xbox\Sentient\libs\SenCore;"
            "xbox\Sentient\libs\SenNews;"
            "xbox\Sentient\libs\SenUGC;"
            "xbox\Sentient\libs\SenBoxArt;"
            "NuiApi;"
            "ST;"
            "NuiFitnessApi;"
            "NuiHandles;"
            "NuiSpeech;"
            "NuiAudio;"
            "xhttp;"
            "xauth;"
            "xgetserviceendpoint;"
            "xav;"
            "xjson;"
            "xbox\4JLibs\libs\4J_XTMS_r"
        ">"
        "xboxkrnl;"
        "xonline"
    )
endif()
add_compile_options(-fpermissive -Wtemplate-body)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
target_link_libraries(${PROJECT_NAME} PRIVATE Minecraft.World)

if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GLFW3_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/4J.Input
        ${CMAKE_SOURCE_DIR}/4J.Profile
        ${CMAKE_SOURCE_DIR}/4J.Render
        ${CMAKE_SOURCE_DIR}/4J.Storage
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        Minecraft.World
        OpenGL::GL
        GLU
        ${GLFW3_LIBRARIES}
        4J_Input
        4J_Profile
        4J_Render
        4J_Storage
        pthread
        dl
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE __linux__)
endif()
