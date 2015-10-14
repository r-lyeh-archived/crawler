#pragma once

#ifdef _MSC_VER
#define inline __forceinline
#define __restrict__ 
#define noreturn
#define __typeof__ decltype
#define snprintf _snprintf
#else
#include <stdnoreturn.h>
#endif
