#pragma once

#ifdef __linux__
#include <cassert>
#elif _WIN32
#include <assert.h>
#endif



#ifdef NDEBUG
#define DASSERT_K(x) x
#define DASSERT_E(x)
#else
#define DASSERT_K(x) assert(x)
#define DASSERT_E(x) assert(x)
#endif
