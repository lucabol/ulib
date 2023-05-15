#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef unsigned char  Byte;
typedef intptr_t Size;

#define FOREACHI(len) for(Size i = 0; i < len; ++i)

#ifdef _MSC_VER
#  define NORETURN __declspec(noreturn)
#  define TRAP __debugbreak()
#elif __GNUC__
#  define NORETURN __attribute__((noreturn))
#  define TRAP __builtin_trap() //*(volatile int *)0 = 0
#else
// This should work only for __GCC__ but it is supported by TCC as well, using assignment to 0 pointer upsets cppcheck
// and it is diffuclt to suppress because it is expanded at each point of call.
#  define NORETURN
#  define TRAP *(volatile int *)0 = 0
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

#define SM do {
#define EM } while(0)

#endif
