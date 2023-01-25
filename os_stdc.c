#include <stdbool.h>
#include <stdlib.h>
#include <stdnoreturn.h>

noreturn void os_trap(void);
noreturn void os_oom(void);
int themain(int argc, char** argv);

noreturn void
os_trap(void) {
  abort();
}

noreturn void
os_oom(void) {
  abort();
}

int
main(int argc, char** argv) {
  return themain(argc, argv);
}
