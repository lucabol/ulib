#ifndef CSV_INCLUDE
#define CSV_INCLUDE

#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <stdlib.h>

#include "Span.h"

typedef enum { Value,  Newline,  End,  EEndInQuoted,  EFullBuffer } CsvTokenType;

#define EOCSV ((char)-1)

#define ADVANCE                                     \
  if(rest.len == 0) {                               \
    ch = EOCSV;                                     \
    rest.ptr++;                                     \
  } else {                                          \
    ch = (char)rest.ptr[0];                         \
    rest = SpanTail(rest, rest.len - 1);                \
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

inline CsvResult
CsvNextToken(Span csv) {

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
      if(rest.len > 0 && rest.ptr[0] == '"') {
        rest.ptr++;
        goto SQUOTED; // Jump over double quotes
      }
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

#endif

#ifdef CSV_IMPL

CsvResult CsvNextToken(Span csv);

#undef CSV_IMPL
#endif
