


#pragma once
#if !defined(_WIN32) && !defined(__debugbreak)
#include <csignal>
#include <cassert>
#define __debugbreak() (raise(SIGTRAP))
#endif
