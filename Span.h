#ifndef SPAN_DEF_INCLUDE
#define SPAN_DEF_INCLUDE

#include "Utils.h"

typedef struct {
  Byte* ptr;
  Size  len;
} Span;

typedef struct {
  Span data;
  const char* error;
} SpanResult;

typedef struct {
  Span head;
  Span tail;
} SpanPair;

#define SPAN(ptr,len) (Span) {(ptr), (len)}
#define SPANERR(msg) (SpanResult) {SPAN(0,0), (msg)}
#define SPANRESULT(span) (SpanResult) {span, NULL}
#define SPANOK(ptr, len) (SpanResult) {SPAN((ptr),(len)), NULL}
#define SPAN0 SPAN(0,0)

inline bool
SpanValid(Span s) { return (s.ptr != NULL) && (s.len >= 0);}

inline Span
SpanHead(Span s, Size size) {
  ASSERT(SpanValid(s));
  ASSERT(size <= s.len);
  ASSERT(0 <= size);

  return SPAN(s.ptr, size);
}

inline Span
SpanTail(Span s, Size size) {
  ASSERT(SpanValid(s));
  ASSERT(size <= s.len);
  ASSERT(0 <= size);

  return SPAN(&s.ptr[s.len - size], size); 
}

inline Span
SpanSub(Span s, Size startIncl, Size endExcl) {
  ASSERT(SpanValid(s));
  ASSERT(0 <= startIncl && startIncl < s.len);
  ASSERT(0 <= endExcl && endExcl <= s.len);
  return SPAN(&s.ptr[startIncl], endExcl - startIncl);
}

inline bool
SpanEqual(Span s1, Span s2) {
  ASSERT(SpanValid(s1));
  ASSERT(SpanValid(s2));

  if(s1.len != s2.len) {
    return false;
  } else {
    // This exits early if different, but has bad branch prediction
    // Better branch prediciton with the below. Probably better for small spans
    // or when equal is more often true than false.
    // foreach(...) {count += s1 == s2;} return count == s1.len;
    FOREACHI(s1.len) {
      if(s1.ptr[i] != s2.ptr[i]) return false;
    }
    return true;
  }
}

// TODO: call builtins if available for perf
inline Size
spanstrlen(char* str) {
  Size i = 0;
  while(str[i]) { i++;}
  return i;
}

// TODO: wonder if the other spaces are needed.
//#define ISSPACE(c)  ((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\v' || (c) == '\f' || (c) == '\r')
#define ISSPACE(c)  ((c) == ' ' || (c) == '\r')


inline Span
SpanTrimStart(Span s) {
  ASSERT(SpanValid(s));

  FOREACHI(s.len) {
    if(!ISSPACE(s.ptr[i])) return SPAN(&s.ptr[i], s.len - i);
  }
  return SPAN(s.ptr,0);
}

inline Span
SpanTrimEnd(Span s) {
  ASSERT(SpanValid(s));

  Size i = s.len - 1;
  for(; i >= 0; --i) 
    if(!ISSPACE(s.ptr[i])) break;

  return SPAN(s.ptr,i + 1);
}

// Ok, this is going overboard. Defined as macro instead of inline function makes gcc inline it.
// CLang inlines it either way.
#define SpanTrim(s) SpanTrimStart(SpanTrimEnd((s)))

inline Span
SpanFromString(char* str) {
  return SPAN((Byte*) str, spanstrlen(str));
}

inline SpanPair
SpanCut(Span s, Byte b) {
  ASSERT(SpanValid(s));
  ASSERT(s.len != 0);

  FOREACHI(s.len) {
    if(s.ptr[i] == b)
      return (SpanPair) {SPAN(s.ptr, i), SPAN(&s.ptr[i + 1], s.len - i - 1)
      };
  }
  return (SpanPair) {s, SPAN(s.ptr, 0)
  };
}

inline SpanPair
SpanRCut(Span s, Byte b) {
  ASSERT(SpanValid(s));
  ASSERT(s.len != 0);

  for(Size i = s.len - 1; i >= 0; i--) {
    if(s.ptr[i] == b)
      return (SpanPair) {SPAN(s.ptr, i), SPAN(&s.ptr[i + 1], s.len - i - 1)
      };
  }
  return (SpanPair) {SPAN(s.ptr, 0), s };
}

inline bool
SpanContains(Span s, Byte b) {
  ASSERT(SpanValid(s));
  FOREACHI(s.len) {
    if(s.ptr[i] == b) return true;
  }
  return false;
}

#define SpanFromUlong_(_name, _value) Span _name; SM \
  int radix = 10; \
  static Byte buffer[32]; \
  Size index = sizeof(buffer); \
  Size len   = index; \
  do { \
    buffer[--index] = (Byte)('0' + (_value % radix)); \
    _value /= radix; \
  } while (_value != 0); \
  _name = SPAN(&buffer[index], len - index); EM

static inline
Span SpanFromUlong(unsigned long value) {
  SpanFromUlong_(ret, value);
  return ret;
}
static inline
Span SpanFromUlong1(unsigned long value) {
  SpanFromUlong_(ret, value);
  return ret;
}
inline unsigned long
SpanToUlong(Span s) {
  ASSERT(SpanValid(s));

  const Span digits = S("0123456789");
  const unsigned long base = 10;
  unsigned long acc = 0;

  for(Size i = 0; i < s.len; i++) {
    Byte c = s.ptr[i];
    if(!SpanContains(digits, c)) break;

    c -= '0';
    acc *= base;
    acc += c;
  }
  return acc;
}

static inline Byte*
SpanTo1KTempString(Span s) {
  static Byte temp[1024];
  for(Size i = 0; i < s.len; i++)
    temp[i] = s.ptr[i];
  temp[s.len] = 0;
  return temp;
}

inline Span
SpanExtractFileName(Byte sep, Span path) {
  return SpanCut(SpanRCut(path, sep).tail,'.').head;
}
static inline Span
Span1KToUpper(Span s) {
  const char OFFSET = 'a' - 'A';
  static Byte temp[1024];

  for(Size i = 0; i < s.len; i++) {
    char ch = s.ptr[i];
    temp[i] = (Byte)(ch >= 'a' && ch <= 'z' ? ch - OFFSET : ch) ;
  }
  return SPAN(temp, s.len);
}

#endif // Header file

#ifdef SPAN_IMPL

bool SpanValid(Span s);
Span SpanHead(Span s, Size size);
Span SpanTail(Span s, Size size);
bool SpanEqual(Span s1, Span s2);
Span SpanTrimStart(Span s);
Span SpanTrimEnd(Span s);
Span SpanFromString(char* str);
SpanPair SpanCut(Span s, Byte b);
SpanPair SpanRCut(Span s, Byte b);
Span SpanSub(Span s, Size startIncl, Size endExcl);
bool  SpanContains(Span s, Byte b);

Span SpanFromUlong(unsigned long value);
Span SpanFromUlong1(unsigned long value);
unsigned long SpanToUlong(Span s);
Byte* SpanTo1KTempString(Span s);
Span SpanExtractFileName(Byte sep, Span path);

Size spanstrlen(char* str);
Span Span1KToUpper(Span s);

#undef SPAN_IMPL
#endif
