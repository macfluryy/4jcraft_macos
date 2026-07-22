#pragma once





#include <cstdio>

#ifdef DEBUG_JAVA_PROXY
#define JPROXY_LOGF(...) fprintf(stderr, __VA_ARGS__)
#else
#define JPROXY_LOGF(...) ((void)0)
#endif
