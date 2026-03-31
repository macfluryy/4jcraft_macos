// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifdef _WINDOWS64
#include "Minecraft.Client/Windows64/Windows64_App.h"
#else
// Use the Linux runtime path with supported metadata/config headers only.
#include "Minecraft.Client/Linux/Linux_App.h"
// #include "Minecraft.Client/Linux/Stubs/LinuxStubs.h"
#endif
