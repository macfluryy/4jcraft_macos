#ifndef STUBS_H
#define STUBS_H

#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <linux/mman.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <codecvt>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <string>

#include "d3d11_stubs.h"
#include "winapi_stubs.h"
#include "xbox_stubs.h"

#ifndef _ENABLEIGGY
#include "iggy_stubs.h"
#endif

#endif  // STUBS_H