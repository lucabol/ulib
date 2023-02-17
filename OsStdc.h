#ifndef Os_STDC_INCLUDE
#define Os_STDC_INCLUDE

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#include "Utils.h"
#include "Span.h"
#include "Buffer.h"

inline NORETURN void
OsTrap(void) {
  abort();
}

inline NORETURN void
OsOom(void) {
  abort();
}

inline SpanResult
OsSlurp(char* path, Size maxsize, Buffer* buf) {

    assert(strlen(path) > 0);

    FILE *f = fopen((char *)path, "rb");
    if (!f) {
      return SPANERR("Can't open file");
    }
    SpanResult s = BufferTryAlloc(buf, maxsize);
    if(s.error) {
      return SPANERR("Can't allocate buffer this big");
    }

    Size len = fread(s.data.ptr, 1, maxsize, f);
    if(ferror(f)) return SPANERR("Error reading from file");
    if(!feof(f)) return SPANERR("File partially read. You may need a bigger bugger.");

    fclose(f);

    return SPANOK(s.data.ptr, len);
}

#endif // Header file

#ifdef OS_STDC_IMPL
#define OS_STDC_IMPL

NORETURN void OsTrap(void);
NORETURN void OsOom(void);
inline SpanResult OsSlurp(char* path, Size maxsize, Buffer* buf);

int themain(int argc, char** argv);
int main(int argc, char** argv) { return themain(argc, argv); }

#undef OS_STDC_IMPL
#endif
