#include "types.h"
#include "fcntl.h"
#include "stat.h"
#include "user.h"

void userHandler(int signum) {
    printf(1, "User-defined handler called for usr1 signal %d\n", signum);
}

int main(int argc, char **argv) {
    signal(SIGUSR1, userHandler);

    printf(1, "Process %d: Sending SIGUSR1 to self\n", getpid());
    kill(getpid(), SIGUSR1);
    return 0;
}