#pragma once

enum EGameCommand {
    eGameCommand_DefaultGameMode,
    eGameCommand_Effect,
    eGameCommand_EnchantItem,
    eGameCommand_Experience,
    eGameCommand_GameMode,
    eGameCommand_Give,
    eGameCommand_Kill,
    eGameCommand_Time,
    eGameCommand_ToggleDownfall,
    eGameCommand_Teleport,
    // 4J Added - extended command set
    eGameCommand_Summon,
    eGameCommand_Help,
    eGameCommand_Say,
    // Multiplayer admin / QoL commands
    eGameCommand_List,
    eGameCommand_Kick,
    eGameCommand_Msg,
    eGameCommand_Heal,
    eGameCommand_Feed,
    eGameCommand_Seed,
    // Op + ban management + diagnostics
    eGameCommand_Op,
    eGameCommand_DeOp,
    eGameCommand_Ban,
    eGameCommand_Pardon,
    eGameCommand_Tps,
    // Player QoL: spawn / home / back / reply
    eGameCommand_Spawn,
    eGameCommand_SetHome,
    eGameCommand_Home,
    eGameCommand_Back,
    eGameCommand_Reply,
    eGameCommand_COUNT
};