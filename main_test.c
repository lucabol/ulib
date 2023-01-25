#include <stdbool.h>
#include <stdio.h>

#include "libu.c"
#include "os_stdc.c"

#define TASSERT(expr) do {                                          \
  __total++; __testt++;                                             \
  if(!(expr)) {                                                     \
    fprintf(stderr, "ERROR: " #expr ", line:%i\n", __LINE__);       \
  } else { __passed++; __testp++;}                                  \
} while(0)

#define TEST_INIT     static int __total = 0; static int __passed = 0; static int __testt = 0; static int __testp = 0
#define TEST_REPORT   printf("TESTS: %i/%i\n\n", __passed, __total);
#define TEST(name)    __testt = 0; __testp = 0; test_ ##name(); printf("%i/%i\t%s\n", __testp, __testt, #name)
#define ALLPASSED     (__total == __passed)

TEST_INIT;

static void
test_span() {
  Span s1 = S("Hello");
  TASSERT(valid(s1));
  TASSERT(s1.len == 5);

  Span s2 = S("");
  TASSERT(valid(s2));
  TASSERT(s2.len == 0);

  const char c3[] = "charray";
  Span s3 = S(c3);
  TASSERT(valid(s3));
  TASSERT(s3.len == 7);

  TASSERT(equal(head(s3, 2), S("ch")));
  TASSERT(equal(head(s3, 0), S("")));
  TASSERT(equal(head(s3, 7), s3));

  TASSERT(equal(S("bob"), S("bob")));
  TASSERT(!equal(S("bob"), S("boby")));
  TASSERT(!equal(S("bob"), S("bo")));
  TASSERT(!equal(S("bo"), S("bob")));
  TASSERT(equal(S(""), S("")));

  TASSERT(equal(tail(S("abcd"),2), S("cd")));
  TASSERT(equal(tail(S("abcd"),0), S("")));
  TASSERT(equal(tail(S("abcd"),4), S("abcd")));
  TASSERT(equal(tail(S(""),0), S("")));
}

static void
test_buffer() {
  Byte data[100];
  Buffer b1 = bufinit(data, 100);

  TASSERT(validb(&b1));
  TASSERT(alloc(&b1, 40).len == 40);
  TASSERT(alloc(&b1, 40).len == 40);
  TASSERT(tryalloc(&b1, 40).error);
  TASSERT(validb(&b1));
  TASSERT(!tryalloc(&b1, 20).error);
}

int
themain(int argc, char** argv) {
  (void)argc;
  (void)argv;

  TEST(span);
  TEST(buffer);

  TEST_REPORT;
  return ALLPASSED ? EXIT_SUCCESS : EXIT_FAILURE;
}

