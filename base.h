#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

typedef unsigned char  Byte;
typedef ptrdiff_t Size;

#ifndef __GNUC__
#  define TRAP __debugbreak()
#else
#  define TRAP __builtin_trap()
#endif

#ifdef DEBUG
#  ifndef assume
#    define assume(c) if (!(c)) {TRAP;}
#  endif
#else
#  define assume(c)
#endif

#define SM do {
#define EM } while(0)

#endif
