#ifndef Os_STDC_INCLUDE
#define Os_STDC_INCLUDE

#include <stdbool.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#include "Utils.h"

inline noreturn void
OsTrap(void) {
  abort();
}

inline noreturn void
OsOom(void) {
  abort();
}

inline bool
OsSlurp(char* path, size_t maxsize, unsigned char buf[]) {

    assert(strlen(path) > 0);

    FILE *f = fopen((char *)path, "rb");
    if (!f) {
        return false;
    }

    size_t len = (size_t)fread(buf, 1, maxsize, f);
    (void)len;
    if(ferror(f)) return false;
    if(!feof(f)) return false;

    fclose(f);

    return true;
}

#endif // Header file

#ifdef OS_STDC_IMPL
#define OS_STDC_IMPL

noreturn void OsTrap(void);
noreturn void OsOom(void);
bool OsSlurp(char* path, size_t maxsize, unsigned char buf[]);

int themain(int argc, char** argv);
int main(int argc, char** argv) { return themain(argc, argv); }

#undef OS_STDC_IMPL
#endif
