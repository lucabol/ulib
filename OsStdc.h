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

SpanResult OsSlurp(char* path, Size maxsize, Buffer* buf);
#endif // Header file

#ifdef OS_STDC_IMPL
#define OS_STDC_IMPL

NORETURN void OsTrap(void);
NORETURN void OsOom(void);

int themain(int argc, char** argv);
int main(int argc, char** argv) { return themain(argc, argv); }

SpanResult
OsSlurp(char* path, Size maxsize, Buffer* buf) {
    assert(strlen(path) > 0);

    FILE *f = NULL;
    const char* err = NULL;
    Size len = 0;

    f = fopen((char *)path, "r");
    if (!f) {
      err = "Can't open file";
      goto exit;
    }
    SpanResult s = BufferTryAlloc(buf, maxsize);
    if(s.error) {
      err = "Can't allocate buffer this big";
      goto exit;
    }

    len = fread(s.data.ptr, 1, maxsize, f);

    if(ferror(f)) {
      err = "Error reading from file";
      len = 0;
      goto exit;
    }
    if(!feof(f)) {
      err = "File partially read. You may need a bigger buffer";
      len = 0;
      goto exit;
    }

    exit:
    if(f) fclose(f);

    BufferDealloc(buf, maxsize - len);

    if(err)
      return SPANERR(err);
    else
      return SPANOK(s.data.ptr, len);
}

#undef OS_STDC_IMPL
#endif
