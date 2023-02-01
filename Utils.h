#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <stdlib.h>

typedef unsigned char  Byte;
typedef intptr_t Size;

#ifdef _MSC_VER
noreturn void __debugbreak(void);
#  define TRAP __debugbreak()
#else
// This should work only for __GCC__ but it is supported by TCC as well, using assignment to 0 pointer upsets cppcheck
// and it is diffuclt to suppress because it is expanded at each point of call.
noreturn void __builtin_trap(void);
#  define TRAP __builtin_trap() //*(volatile int *)0 = 0
#endif

#ifdef DEBUG
#  ifndef ASSERT
#    define ASSERT(c) if (!(c)) {TRAP;}
#  endif
#else
#  define ASSERT(c)
#endif

#define SIZEOF(x) (Size)(sizeof(x))
#define ALIGN SIZEOF(void *)
#define COUNTOF(a) (SIZEOF((a))/SIZEOF((a)[0]))

#define S(s) (Span){(Byte *)(s), SIZEOF(s)-1}
#define Z(s) (Span){(Byte *)(s), SIZEOF(s)}

#endif
