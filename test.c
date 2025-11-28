#include "types.h"
#include "fcntl.h"
#include "stat.h"
#include "user.h"

void handler_int(int sig) {
  printf(1,"This is from user defined handler for signal %d\n", sig);
}

void handler_segv(int sig) {
  printf(1,"Hi from segmentation fault handler SIGNO%d\n", sig);
  exit();
}

void handler_usr1(int sig) {
  printf(1,"This is user defined handler signal 1 received: %d\n", sig);
}

void handler_chld(int sig) {
  printf(1,"This is from child signal handler SIGNO%d\n", sig);
}

void handler_term(int sig) {
  printf(1,"This is from termination signal handler SIGNO%d\n", sig);
}

void handler_cont(int sig) {
  printf(1,"Hi from continue signal handler SIGNO%d\n", sig);
}

void handler_ill(int sig) {
  printf(1,"This is from illegal instruction signal handler SIGNO%d\n", sig);
}

void handler_vtalrm(int sig) {
  printf(1,"This is from virtual alarm signal handler SIGNO%d\n", sig);
}

void test_sigusr1() {
  printf(1,"BASIC SIGUSR1 TEST\n");
  signal(SIGUSR1, handler_usr1);
  kill(getpid(), SIGUSR1);
  sleep(20);
  printf(1,"BASIC SIGUSR1 TEST PASSED\n");

}

void test_pause() {
  printf(1,"PAUSE TEST\n");
  int pid = fork();
  if(pid == 0){
    signal(SIGUSR1, handler_usr1);
    printf(1, "Pause called by child\n");
    
    if(pause() != -1){
        printf(1, "Pause Failed\n");
    }
    
    printf(1, "Child resumed from pause\n");
    exit();
  }

  sleep(20);
  printf(1, "Parent sending SIGUSR1 to child %d\n", pid);
  kill(pid, SIGUSR1);
  
  wait();
  printf(1, "Pause Test passed.\n");
}

void test_sigprocmask() {
  printf(1,"SIGPROCMASK TEST\n");
  signal(SIGINT, handler_int);

  uint newmask = (1 << SIGINT);
  uint oldmask;

  printf(1, "Blocking SIGINT...\n");
  sigprocmask(SIG_BLOCK, &newmask, &oldmask);

  printf(1, "SIGINT sent while blocked, Handler shouldn't run.\n");
  kill(getpid(), SIGINT);
  sleep(50);

  printf(1, "Unblocking SIGINT...\n");
  sigprocmask(SIG_UNBLOCK, &newmask, &oldmask);
  printf(1, "Handler should run now if any SIGINT pending.\n");
  sleep(50);

  printf(1, "SIGINT sent again Handler should run now.\n");
  kill(getpid(), SIGINT);
  sleep(50);

  printf(1, "Sigprocmask test done.\n");
}

void test_sigterm() {
  printf(1,"SIGTERM TEST\n");
  signal(SIGTERM, handler_term);
  printf(1, "Sending SIGTERM.\n");
  kill(getpid(), SIGTERM);
  printf(1, "SIGTERM test passed.\n");
}

void test_sigkill() {
  printf(1,"SIGKILL TEST\n");
  int pid = fork();
  if(pid == 0){
    signal(SIGKILL, handler_int); // this should not work
    while(1) {
      printf(1, "Child running...\n");
      sleep(20);
    }
    exit();
  }

  sleep(50);
  printf(1, "Parent sending SIGKILL to child %d\n", pid);
  kill(pid, SIGKILL);
  
  wait();
  printf(1, "SIGKILL Test passed.\n");
}

void test_sigill_sigsev() {
  printf(1,"SIGILL, SIGSEGV TEST\n");

  signal(SIGILL, handler_ill);
  printf(1, "Sending SIGILL.\n");
  kill(getpid(), SIGILL);
  sleep(10);

  printf(1, "Forking to test Seg Fault...\n");
  int pid = fork();
  if(pid == 0){
      signal(SIGSEGV, handler_segv);
      printf(1, "Causing Seg Fault to raise SIGSEGV.\n");
      int *p = (int*)0x80000000; 
      *p = 10; 
  }
  wait(); //parent waits for child to die from segfault

  printf(1, "SIGILL, SIGSEGV test done.\n");
}

void test_alarm() {
  printf(1,"ALARM SYSCALL TEST (SIGVTALRM)\n");

  signal(SIGVTALRM, handler_vtalrm);

  printf(1, "Setting alarm for 10 ticks...\n");
  alarm(10);

  printf(1, "Sleeping to wait for alarm...\n");
  sleep(20);

  printf(1, "ALARM SYSCALL TEST FINISHED\n");
}

void test_sigchld() {
  printf(1,"SIGCHLD TEST\n");
  signal(SIGCHLD, handler_chld);

  int pid = fork();
  if(pid == 0){
    printf(1, "Child process exiting and will raise SIGCHLD in parent.\n");
    exit();
  }

  sleep(50); 
  wait();
  printf(1, "SIGCHLD test done.\n");
}

void test_sigstop_sigcont() {
  printf(1,"SIGSTOP and SIGCONT TEST\n");
  signal(SIGSTOP, handler_int); // this should not work
  signal(SIGCONT, handler_cont);

  int pid = fork();
  if(pid == 0){
    while(1) {
      printf(1, "Child running...\n");
      sleep(20);
    }
    exit();
  }

  sleep(50);
  printf(1, "Parent sending SIGSTOP to child %d\n", pid);
  kill(pid, SIGSTOP);

  sleep(100);
  printf(1, "Parent sending SIGCONT to child %d\n", pid);
  kill(pid, SIGCONT);

  sleep(50);
  printf(1, "Parent sending SIGKILL to child %d to end test\n", pid);
  kill(pid, SIGKILL);
  
  wait();
  printf(1, "SIGSTOP and SIGCONT Test passed.\n");
}

int main(int argc, char *argv[]) {
  test_sigusr1();
  test_pause();
  test_sigprocmask();
  test_sigterm();
  test_sigkill();
  test_sigchld();
  test_sigstop_sigcont();
  test_sigill_sigsev();
  test_alarm();
  printf(1, "All signal tests completed.\n");
  printf(1,"OS was good!");
  exit();
}

