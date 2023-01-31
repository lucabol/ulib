#ifndef OS_STDC_C
#define OS_STDC_C

#include <stdbool.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "libu.c"

int themain(int argc, char** argv);

noreturn void
os_trap(void) {
  abort();
}

noreturn void
os_oom(void) {
  abort();
}

bool
os_slurp(char* path, size_t maxsize, unsigned char buf[]) {

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
int
main(int argc, char** argv) {
  return themain(argc, argv);
}

#endif
