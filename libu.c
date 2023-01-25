#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <stdlib.h>

noreturn void os_trap(void);
noreturn void os_oom(void);

typedef char  Byte;
typedef intptr_t Size;

#ifdef DEBUG
#  ifndef ASSERT
#    define ASSERT(c) if (!(c)) {os_trap();}
#  endif
#else
#  define ASSERT(c)
#endif

#define SIZEOF(x) (Size)(sizeof(x))
#define ALIGN SIZEOF(void *)
#define COUNTOF(a) (SIZEOF((a))/SIZEOF((a)[0]))

#define S(s) (Span){(Byte *)(s), SIZEOF(s)-1}
#define Z(s) (Span){(Byte *)(s), SIZEOF(s)}

typedef struct {
  Byte* ptr;
  Size  len;
} Span;

static bool
valid(Span s) { return (s.ptr != NULL) && (s.len >= 0);}

static Span
head(Span s, Size size) {
  ASSERT(valid(s));
  ASSERT(size <= s.len);
  ASSERT(0 <= size);

  return (Span) { s.ptr, size }; 
}

static Span
tail(Span s, Size size) {
  ASSERT(valid(s));
  ASSERT(size <= s.len);
  ASSERT(0 <= size);

  return (Span) { &s.ptr[s.len - size], size }; 
}

static bool
equal(Span s1, Span s2) {
  ASSERT(valid(s1));
  ASSERT(valid(s2));

  if(s1.len != s2.len) {
    return false;
  } else {
    int i = 0;
    while((i < s1.len) && (s1.ptr[i] == s2.ptr[i])) { i++;}
    return i == s1.len;
  }
}

typedef struct {
  Span data;
  Size index;
} Buffer;

static Size
avail(Buffer* buf) { return buf->data.len - buf->index; }

static bool
validb(Buffer* buf) {
  return valid(buf->data) && (0 <= buf->index) && (buf->index <= buf->data.len);
}

static Buffer
bufinit(Byte* data, Size size) {
  ASSERT(data !=NULL);
  ASSERT(size > 0);

  return (Buffer) { (Span) {data, size}, 0};
}

typedef struct {
  Span data;
  bool error;
} SpanResult;

static SpanResult
tryalloc(Buffer* buf, Size size) {
  ASSERT(validb(buf));
  ASSERT(0 < size);

  if((buf->data.len < size) || avail(buf) < size) {
    return (SpanResult) { .error = true};
  }

  Span s = (Span) { &buf->data.ptr[buf->index], size};
  ASSERT(valid(s));

  buf->index += size; 
  ASSERT(validb(buf));

  return (SpanResult) { .data = s, .error = false };
}

static Span
alloc(Buffer* buf, Size size) {
  SpanResult sr = tryalloc(buf, size);
  if(sr.error) {
    os_oom();
  }

  return sr.data;
}


