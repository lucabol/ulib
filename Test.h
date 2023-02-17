#ifndef TEST_INCLUDED
#define TEST_INCLUDED

bool __gotassert = false;

#ifdef DEBUG
#  define ASSERT(cond) do { if(!(cond)) {__gotassert = true;} } while(0)
#  define TFAILASSERT(stmt) do {                     \
    __gotassert = false; stmt; if(!__gotassert) TASSERT(__gotassert); __gotassert = false;\
  } while(false) 
#else
#  define TFAILASSERT(stmt)
#endif

#   define TASSERT(expr) do {                                         \
    __total++; __testt++;                                             \
    if(__gotassert) fprintf(stderr, "ASSERT: " #expr ", line:%i\n", __LINE__);       \
    else if(!(expr)) fprintf(stderr, "ERROR: " #expr ", line:%i\n", __LINE__);       \
    else { __passed++; __testp++;}                                  \
  } while(0)

#define TEST_INIT     int __total = 0; int __passed = 0; int __testt = 0; int __testp = 0
#define TEST_REPORT   printf("TESTS: %i/%i\n\n", __passed, __total);
#define TEST(name)    __testt = 0; __testp = 0; test_ ##name(); printf("%i/%i\t%s\n", __testp, __testt, #name)
#define ALLPASSED     (__total == __passed)

#endif
