#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
   bool __gotassert = false;
#  define ASSERT(cond) do { if(!(cond)) {__gotassert = true;} } while(0)
#  define TFAILASSERT(stmt) do {                     \
    __gotassert = false; stmt; TASSERT(__gotassert); \
  } while(false) 
#else
#  define TFAILASSERT(stmt)
#endif

#include "libu.c"
#include "libu.c"
#include "os_stdc.c"

#define TASSERT(expr) do {                                          \
  __total++; __testt++;                                             \
  if(!(expr)) {                                                     \
    fprintf(stderr, "ERROR: " #expr ", line:%i\n", __LINE__);       \
  } else { __passed++; __testp++;}                                  \
} while(0)

#define TEST_INIT     int __total = 0; int __passed = 0; int __testt = 0; int __testp = 0
#define TEST_REPORT   printf("TESTS: %i/%i\n\n", __passed, __total);
#define TEST(name)    __testt = 0; __testp = 0; test_ ##name(); printf("%i/%i\t%s\n", __testp, __testt, #name)
#define ALLPASSED     (__total == __passed)

TEST_INIT;

void
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

  #define CUTEQ(s,h,t) \
    TASSERT(equal(cut(S((#s)), 'K').head, S(#h))); \
    TASSERT(equal(cut(S((#s)), 'K').tail, S(#t))) 

  CUTEQ(abKde,ab,de);
  CUTEQ(abK,ab,);
  CUTEQ(Kde,,de);
  CUTEQ(Ke,,e);
  CUTEQ(aK,a,);
  CUTEQ(aKb,a,b);
  CUTEQ(K,,);

  #define TSTART(x, y) TASSERT(equal(trimstart(S((x))), S((y))))
  #define TEND(x, y) TASSERT(equal(trimend(S((x))), S((y))))
  #define TRIM(x, y) TASSERT(equal(trim(S((x))), S((y))))
  TSTART("","");
  TSTART(" ","");
  TSTART("a","a");
  TSTART(" a","a");
  TSTART("  a","a");
  TSTART("  a ","a ");
  TEND("","");
  TEND("a","a");
  TEND("a ","a");
  TEND("a  ","a");
  TEND("  a ","  a");
  TRIM("","");
  TRIM("  ","");
  TRIM("a","a");
  TRIM("  a ","a");
  TRIM("a  ","a");
  TRIM("  a ","a");

}

void
test_buffer() {
  Byte data[100];
  Buffer b1 = bufinit(data, 100);

  #define OKALLOC40 (tryalloc(&b1, 40).data.len == 40)

  TASSERT(validb(&b1));
  TASSERT(OKALLOC40);
  TASSERT(OKALLOC40);
  TASSERT(!OKALLOC40);
  TASSERT(validb(&b1));
  TASSERT(!tryalloc(&b1, 20).error);
  TASSERT(validb(&b1));
  TASSERT(tryalloc(&b1, 1).error);

  b1.index = 0;
  TFAILASSERT(tryalloc(&b1,0));
}

void
tocsvstring(char* strings[], int items, char dest[], Size size) {
  memset(dest, 0, size); 
  for(Size i = 0; i < items; i++) {
    char* s = strings[i];
    strcat(dest, s);
    if(i != items - 1 && strcmp(s, "\n")) {
      strcat(dest, ",");
    }
  }
}

bool
equaltest(char* strings[], int items, Span csv) {

  Span rest = csv;
  CsvResult r = {0};
  Span str = {0};

  for(int i = 0; i < items; i++) {
    if(rest.len == 0) {
      printf("Exausthed csv?\n");
      return false;
    }
    r = nextcsvtokeng(rest);
    str = fromzstring(strings[i]);

    // Remove spaces and quotes from comparison string
    Span trimmed = trim(str);
    if(trimmed.ptr[0] == '"' && trimmed.ptr[trimmed.len - 1] == '"') {
      trimmed.ptr++;
      trimmed.len -= 2;
      str = trimmed;
    }

    if(!equal(r.value, str)) {
      printf("ERROR: %s != %s\n", r.value.ptr, str.ptr);
      return false;
    }
    rest = r.rest;
  }
  return true;
}

void
testcsvarray(char* values[], int n) {
  char dest[1024];
  tocsvstring(values, n, dest, 1024);
  bool res = equaltest(values, n, fromzstring(dest));
  TASSERT(res);
}

#define TESTCSV(...) {                                 \
  char* ar[] = {__VA_ARGS__};                          \
  int n = sizeof(ar) / sizeof(ar[0]);                  \
  testcsvarray(ar, n);                                 \
}

void
test_csv() {
  TESTCSV("123", "456", "789", "\n", "abc"); // simple
  TESTCSV("123 ", " 456", "78 9", " \n", " abc"); // spaces in various places
  TESTCSV("123 ", " 456", "78 9", " \n", " abc", "\n", "\n"); // multiple newlines
  TESTCSV("", " 456", "", "78 9", " \n", " abc", "\n", "", "\n"); // empty members

  TESTCSV("\"123\"", "\"456\"", "\"789\"", "\n", "\"abc\""); // standard
  TESTCSV("\"123\" ", "\"456\"  ", "\" 789\"", "\n", "\"abc\""); // spaces in various places
  TESTCSV("\"12,3\" ", "\",456,\"  ", "\" 789\"", "\n", "\"abc\""); // commas inside quotes
  TESTCSV("\"12\"\",3\" ", "\"\"\",456,\"  ", "\" 789\"\"\"", "\"\"\n", "\"abc\"\"\""); // jumps over double quotes
}
int
themain(int argc, char** argv) {
  (void)argc;
  (void)argv;

  TEST(span);
  TEST(buffer);
  TEST(csv);

  TEST_REPORT;
  return ALLPASSED ? EXIT_SUCCESS : EXIT_FAILURE;
}

