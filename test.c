#include "types.h"
#include "fcntl.h"
#include "stat.h"
#include "user.h"

// void handler(int sig) {
//     printf(1,"SIGSEGV caught! sig=%d\n", sig);

//     // Immediately send SIGSEGV to self again → infinite loop
//     kill(getpid(), SIGSEGV);
// }


void handler(int sig) {
  printf(1, "Handler: caught SIGINT (%d)\n", sig);
  sigreturn();
}

int
main(void)
{
  uint oldmask, newmask;

  // Install handler for SIGINT
  signal(SIGINT, handler);

  printf(1, "Test 1: Blocking SIGINT\n");
  newmask = (1 << SIGINT);

  sigprocmask(SIG_BLOCK, &newmask, &oldmask);

  printf(1, "Old mask was: 0x%x\n", oldmask);
  printf(1, "Now SIGINT is blocked. Sending SIGINT...\n");

  kill(getpid(), SIGINT);

  printf(1, "If BLOCK is working, no handler message should appear.\n");

  // Give time for signal (it should not trigger)
  sleep(100);

  printf(1, "Test 2: Unblocking SIGINT\n");
  sigprocmask(SIG_UNBLOCK, &newmask, &oldmask);
  printf(1, "Old mask was: 0x%x\n", oldmask);

  printf(1, "Sending SIGINT again. Handler SHOULD run now.\n");
  kill(getpid(), SIGINT);

  // Sleep so handler runs
  sleep(100);

  printf(1, "Test finished.\n");

  exit();
}