#ifndef BUFFER_INCLUDE
#define BUFFER_INCLUDE

#include "Utils.h"
#include "Span.h"

typedef struct {
  Span data;
  Size index;
} Buffer;

inline Size
BufferAvail(Buffer* buf) { return buf->data.len - buf->index; }

inline bool
BufferValid(Buffer* buf) {
  return SpanValid(buf->data) && (0 <= buf->index) && (buf->index <= buf->data.len);
}

inline Buffer
BufferInit(Byte* data, Size size) {
  ASSERT(data !=NULL);
  ASSERT(size > 0);

  return (Buffer) { SPAN(data, size), 0};
}

inline SpanResult
BufferTryAlloc(Buffer* buf, Size size) {
  ASSERT(BufferValid(buf));
  ASSERT(0 < size);

  if(BufferAvail(buf) < size) {
    return SPANERR("Buffer too small for the allocation");
  }

  Span s = SPAN(&buf->data.ptr[buf->index], size);
  ASSERT(SpanValid(s));

  buf->index += size; 
  ASSERT(BufferValid(buf));

  return SPANOK(s.ptr, s.len);
}

inline void
BufferDealloc(Buffer* buf, Size size) {
  ASSERT(BufferValid(buf));
  ASSERT(size <= buf->index);

  size = size <= buf->index ? size : buf->index;
  buf->index -= size;
}

inline void
BufferPushByte(Buffer* b, Byte ch) {
  ASSERT(BufferValid(b));

  b->data.ptr[b->index] = ch;
  b->index++;
}

inline bool
TryBufferPushByte(Buffer* b, Byte ch) {
  ASSERT(BufferValid(b));

  if(BufferAvail(b) <= 0) {
    return false;
  }

  BufferPushByte(b, ch);
  return true;
}

inline SpanResult
BufferCopy(Span s, Buffer* b) {
  ASSERT(BufferValid(b));

  if(BufferAvail(b) < s.len + 1) {
    return SPANERR("Buffer too small for the copy");
  }
  Span result = b->data;
  result.len = s.len;

  for(Size i = 0; i < s.len; i++) {
    BufferPushByte(b, s.ptr[i]);
  }    
  return SPANOK(result.ptr, result.len);
}

#endif // Header file

#ifdef BUFFER_IMPL

Size BufferAvail(Buffer* buf);
bool BufferValid(Buffer* buf);
Buffer BufferInit(Byte* data, Size size);

SpanResult BufferTryAlloc(Buffer* buf, Size size);
void BufferDealloc(Buffer* buf, Size size);
void BufferPushByte(Buffer* b, Byte ch);

bool TryBufferPushByte(Buffer* b, Byte ch);
SpanResult BufferCopy(Span s, Buffer* b);

#undef BUFFER_IMPL
#endif
