#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <stdlib.h>

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

typedef unsigned char  Byte;
typedef intptr_t Size;

typedef struct {
  Byte* ptr;
  Size  len;
} Span;

#define FOREACHI(len) for(Size i = 0; i < len; ++i)
bool
valid(Span s) { return (s.ptr != NULL) && (s.len >= 0);}

Span
head(Span s, Size size) {
  ASSERT(valid(s));
  ASSERT(size <= s.len);
  ASSERT(0 <= size);

  return (Span) { s.ptr, size }; 
}

Span
tail(Span s, Size size) {
  ASSERT(valid(s));
  ASSERT(size <= s.len);
  ASSERT(0 <= size);

  return (Span) { &s.ptr[s.len - size], size }; 
}

bool
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

Size
spanstrlen(char* str) {
  Size i = 0;
  while(str[i]) { i++;}
  return i;
}

bool
spanisspace(char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

Span
trimstart(Span s) {
  ASSERT(valid(s));

  int i = 0;
  for(; i < s.len; ++i) 
    if(!spanisspace(s.ptr[i])) break;

  return (Span) { &s.ptr[i], s.len - i };
}

Span
trimend(Span s) {
  ASSERT(valid(s));

  int i = s.len - 1;
  for(; i >= 0; --i) 
    if(!spanisspace(s.ptr[i])) break;

  return (Span) { s.ptr, i + 1 };
}

Span
trim(Span s) {
  return trimstart(trimend(s));
}

Span
fromzstring(char* str) {
  return (Span) { (Byte*) str, spanstrlen(str)};
}

typedef struct {
  Span head;
  Span tail;
} SpanPair;

SpanPair
cut(Span s, Byte b) {
  ASSERT(valid(s));
  ASSERT(s.len != 0);

  Size i = 0;
  while((i < s.len) && (s.ptr[i] != b)) {
    i++;
  }
  return (SpanPair) {
    .head = (Span) { s.ptr, i},
    .tail = (Span) { &s.ptr[i + 1], s.len - i - 1}
  };
}

typedef struct {
  Span data;
  Size index;
} Buffer;

Size
avail(Buffer* buf) { return buf->data.len - buf->index; }

bool
validb(Buffer* buf) {
  return valid(buf->data) && (0 <= buf->index) && (buf->index <= buf->data.len);
}

Buffer
bufinit(Byte* data, Size size) {
  ASSERT(data !=NULL);
  ASSERT(size > 0);

  return (Buffer) { (Span) {data, size}, 0};
}

typedef struct {
  Span data;
  bool error;
} SpanResult;

SpanResult
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

void
pushbyte(Buffer* b, Byte ch) {
  ASSERT(validb(b));

  b->data.ptr[b->index] = ch;
  b->index++;
}

bool
trypushbyte(Buffer* b, Byte ch) {
  ASSERT(validb(b));

  if(avail(b) <= 0) {
    return false;
  }

  pushbyte(b, ch);
  return true;
}

SpanResult
copytobuffer(Span s, Buffer* b) {
  ASSERT(validb(b));

  if(avail(b) < s.len + 1) {
    return (SpanResult) { {0}, true};
  }
  Span result = b->data;
  result.len = s.len;

  for(Size i = 0; i < s.len; i++) {
    pushbyte(b, s.ptr[i]);
  }    
  return (SpanResult) { result, false };
}



#define CSVRESULTTYPES Value,  Newline,  End,  EEndInQuoted,  EFullBuffer
#define CSVSTATETYPES  SValue, SNewline, SEnd, SEEndInQuoted, SEFullBuffer

typedef enum { CSVRESULTTYPES } CsvTokenType;

typedef struct {
  CsvTokenType type;
  Span rest;
} CsvToken;

typedef enum {
  StartValue,
  InsideQuoted,
  InsideValue,
  BlankAfterValue,
  BlankAfterQuoted,
  CSVSTATETYPES
} _CsvState;

#define EOCSV ((char)-1)

#define PUSHCHAR                      \
      if(!trypushbyte(value, ch)) {   \
        return SEFullBuffer;          \
      }

// Skip initial whitespaces
_CsvState
startvalue(char ch, Buffer* value) {
  switch(ch) {
    case EOCSV:
      return SEnd;
    case '\n':
      return SNewline;
    case '"':
      return InsideQuoted;
    default:
      PUSHCHAR;
      return InsideValue;
  }
}

_CsvState
insidequoted(char ch, Buffer* value) {
  switch(ch) {
    case EOCSV:
      return SEEndInQuoted;
    case '"':
      return BlankAfterQuoted;
    default:
      PUSHCHAR;
      return InsideQuoted;
  }
}
_CsvState
insidevalue(char ch, Buffer* value) {
  switch(ch) {
    case EOCSV:
    case ',':
      return SValue;
    default:
      PUSHCHAR;
      return InsideValue;
  }
}
_CsvState
blankafterquoted(char ch) {
  switch(ch) {
    case EOCSV:
    case ',':
      return SValue;
    default:
      return BlankAfterQuoted;
  }
}

CsvToken
nextcsvtoken(Span csv, Buffer* value) {

  _CsvState state = StartValue;

  Span rest = csv;

  while(true) {
    Span h = head(rest,1);
    rest   = tail(rest, rest.len - 1);

    char ch = (h.len == 0) ? EOCSV : (char)h.ptr[0];
    
    switch(state) {
      case StartValue:
        state = startvalue(ch, value);
        break;
      case InsideQuoted:
        state = insidequoted(ch, value);
        break;
      case InsideValue:
        state = insidevalue(ch, value);
        break;
      case BlankAfterQuoted:
        state = blankafterquoted(ch);
        break;
      case SValue:
        return (CsvToken) { Value, rest };
        break;
      case SNewline:
        return (CsvToken) { Newline, rest };
        break;
      case SEnd:
        return (CsvToken) { End, rest };
        break;
      case SEEndInQuoted:
        return (CsvToken) { EEndInQuoted, rest };
        break;
      case SEFullBuffer:
        return (CsvToken) { EFullBuffer, rest };
        break;
      default:
        ASSERT(false);
        break;
    }
  }
}

#define ADVANCE                                     \
  if(rest.len == 0) {                               \
    ch = EOCSV;                                     \
    rest.ptr++;                                     \
  } else {                                          \
    ch = (char)rest.ptr[0];                         \
    rest = tail(rest, rest.len - 1);                \
  }                                                 \
  switch(ch)

#define FIRSTSTATE                                \
  Span rest = csv;                                \
  char ch = EOCSV;                                \
  Byte* startPtr = csv.ptr;                       \
  Byte* endPtr   = 0;                             \
  ADVANCE

#define STATE(name)                               \
  name:                                           \
  ADVANCE

#define RETTOKEN(tokentype, startPtrIncl, len) \
  return (CsvResult) { tokentype, (Span) { (startPtrIncl), (len)}, rest }

typedef struct {
  CsvTokenType type;
  Span value;
  Span rest;
} CsvResult;

CsvResult
nextcsvtokeng(Span csv) {

  FIRSTSTATE {
    case EOCSV:
      RETTOKEN(End, startPtr,0);
    case '\n':
      RETTOKEN(Newline, startPtr, 1);
    case ',':
      RETTOKEN(Value,startPtr, rest.ptr - startPtr - 1);
    case '"':
      startPtr++;
      goto SQUOTED;
    default:
      goto SVALUE;
  }
  STATE(SQUOTED) {
    case EOCSV:
      RETTOKEN(EEndInQuoted, startPtr, rest.ptr - startPtr - 1);
    case '"':
      endPtr = rest.ptr;
      goto SVALUE; // the quote and blank after quote become part of value
    default:
      goto SQUOTED;
  }
  STATE(SVALUE) {
    case EOCSV:
    case ',':
      if(endPtr)
        RETTOKEN(Value,startPtr, endPtr - startPtr - 1);
      else
        RETTOKEN(Value,startPtr, rest.ptr - startPtr - 1);
    default:
      goto SVALUE; 
  }

}
