/* macOS/Linux compatibility: define __debugbreak as a POSIX equivalent.
 * On MSVC this is a hardware breakpoint intrinsic.
 * On POSIX we use raise(SIGTRAP) which causes a debugger break or SIGABRT. */
#pragma once
#if !defined(_WIN32) && !defined(__debugbreak)
#include <csignal>
#include <cassert>
#define __debugbreak() (raise(SIGTRAP))
#endif
