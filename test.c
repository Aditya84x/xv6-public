#include "types.h"
#include "fcntl.h"
#include "stat.h"
#include "user.h"

void userHandler(int signum) {
    printf(1, "User-defined handler called for SIGSEGV signal %d\n", signum);
}

int main(int argc, char **argv) {
    signal(SIGSEGV, userHandler);
    int *p = (int*)0x80000000; // Invalid memory access to trigger SIGSEGV
    int val = *p; // This should cause a segmentation fault
    printf(1, "Value at invalid address: %d\n", val); // This line should not be reached
    exit();
}