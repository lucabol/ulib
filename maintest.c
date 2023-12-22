#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// This has to come first as it changes the 'assume' semantic in the other include files
#include "test.h"

#define BUFFER_IMPL
#include "buffer.h"

#define SPAN_IMPL
#include "span.h"

#define OS_STDC_IMPL
#include "osstdc.h"

#define CSV_IMPL
#include "csv.h"

#include "dodstruct.h"

#define DATA "data/"

TEST_INIT;

void
test_span() {
  Span s1 = S("Hello");
  tassert(SpanValid(s1));
  tassert(s1.len == 5);

  Span s2 = S("");
  tassert(SpanValid(s2));
  tassert(s2.len == 0);

  const char c3[] = "charray";
  Span s3 = S(c3);
  tassert(SpanValid(s3));
  tassert(s3.len == 7);

  tassert(SpanEqual(SpanHead(s3, 2), S("ch")));
  tassert(SpanEqual(SpanHead(s3, 0), S("")));
  tassert(SpanEqual(SpanHead(s3, 7), s3));

  tassert(SpanEqual(S("bob"), S("bob")));
  tassert(!SpanEqual(S("bob"), S("boby")));
  tassert(!SpanEqual(S("bob"), S("bo")));
  tassert(!SpanEqual(S("bo"), S("bob")));
  tassert(SpanEqual(S(""), S("")));

  tassert(SpanEqual(SpanTail(S("abcd"),2), S("cd")));
  tassert(SpanEqual(SpanTail(S("abcd"),0), S("")));
  tassert(SpanEqual(SpanTail(S("abcd"),4), S("abcd")));
  tassert(SpanEqual(SpanTail(S(""),0), S("")));

  #define CUTEQ(s,h,t) \
    tassert(SpanEqual(SpanCut(S((#s)), 'K').head, S(#h))); \
    tassert(SpanEqual(SpanCut(S((#s)), 'K').tail, S(#t))) 

  CUTEQ(abKde,ab,de);
  CUTEQ(abK,ab,);
  CUTEQ(Kde,,de);
  CUTEQ(Ke,,e);
  CUTEQ(aK,a,);
  CUTEQ(aKb,a,b);
  CUTEQ(K,,);

  #define TSTART(x, y) tassert(SpanEqual(SpanTrimStart(S((x))), S((y))))
  #define TEND(x, y) tassert(SpanEqual(SpanTrimEnd(S((x))), S((y))))
  #define TTRIM(x, y) tassert(SpanEqual(SpanTrim(S((x))), S((y))))
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
  TTRIM("","");
  TTRIM("  ","");
  TTRIM("a","a");
  TTRIM("  a ","a");
  TTRIM("a  ","a");
  TTRIM("  a ","a");

  #define TUL(n) assert(SpanEqual(S(#n), SpanFromUlong(n)))
  TUL(0);
  TUL(345000);
  #define TSL(n) assert(n == SpanToUlong(S(#n)))
  TSL(0);
  TSL(34500);

  #define TEX(p,e) assert(SpanEqual(S(#e),SpanExtractFileName('/',S(#p))))
  TEX(/dev/foo.x,foo);
  TEX(/dev/foo,foo);
  TEX(/foo,foo);
  TEX(foo.ks,foo);
  TEX(foo,foo);
}

void
test_buffer() {
  Byte data[100];
  Buffer b1 = BufferInit(data, 100);

  #define OKALLOC40 (BufferTryAlloc(&b1, 40).data.len == 40)

  tassert(BufferValid(&b1));
  tassert(OKALLOC40);
  tassert(OKALLOC40);
  tassert(!OKALLOC40);
  tassert(BufferValid(&b1));
  tassert(!BufferTryAlloc(&b1, 20).error);
  tassert(BufferValid(&b1));
  tassert(BufferTryAlloc(&b1, 1).error);

  b1.index = 0;
  tfailassert(BufferTryAlloc(&b1,0));

}

void test_struct() {
  Struct1(P1,100, int,Count);
  P1Count[P1Idx] = 1;
  tassert(P1Count[P1Idx] == 1);

  Struct2(P2,200, int,Count, float, FVal);(void)P2FVal;
  P2Count[P2Idx] = 2;
  tassert(P2Count[P2Idx] == 2.0);

  Struct3(P3,200, int,Count, float, FVal,int,KVal);(void)P3FVal;(void)P3KVal;
  P3Count[P3Idx] = 2;
  tassert(P3Count[P2Idx] == 2.0);

  Struct4(P4,200, int,Count, float, FVal,int,KVal,int,CVal);(void)P4FVal;(void)P4KVal;(void)P4CVal;
  P4Count[P4Idx] = 2;
  tassert(P4Count[P2Idx] == 2.0);

  Struct5(P5,200, int,Count, float, FVal,int,KVal,int,CVal,int,GVal);(void)P5FVal;(void)P5KVal;(void)P5CVal;(void)P5GVal;
  P5Count[P5Idx] = 2;
  tassert(P5Count[P2Idx] == 2.0);
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
    r = CsvNextToken(rest);
    str = SpanFromString(strings[i]);

    // Remove spaces and quotes from comparison string
    Span trimmed = SpanTrim(str);
    if(trimmed.ptr[0] == '"' && trimmed.ptr[trimmed.len - 1] == '"') {
      trimmed.ptr++;
      trimmed.len -= 2;
      str = trimmed;
    }

    if(!SpanEqual(r.value, str)) {
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
  bool res = equaltest(values, n, SpanFromString(dest));
  tassert(res);
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
  TESTCSV("\"12\"\",3\" ", "\"\"\",456,\"  ", "\" 789\"\"\"", " \"\"\n", "\"abc\"\"\""); // jumps over double quotes
}

void
test_slurp() {
  Byte data[100];
  Buffer b1 = BufferInit(data, 100);

  SpanResult sr = OsSlurp(DATA "slurp1.txt", 100, &b1);
  tassert(!sr.error);
  tassert(BufferAvail(&b1) == 100);
  
  sr = OsSlurp(DATA "slurp2.txt", 100, &b1);
  tassert(!sr.error);
  tassert(SpanEqual(sr.data, S("0123456789\n")));

  sr = OsSlurp(DATA "errorslurp.txt", 100, &b1);
  tassert(sr.error);
}

int
themain(int argc, char** argv) {
  (void)argc;
  (void)argv;

  TEST(span);
  TEST(buffer);
  TEST(csv);
  TEST(slurp);
  TEST(struct);

  TEST_REPORT;
  return ALLPASSED ? EXIT_SUCCESS : EXIT_FAILURE;
}
