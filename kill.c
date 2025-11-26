#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int i;

  if(argc < 2){
    printf(2, "usage: kill pid...\n");
    exit();
  }
  
  char *p = argv[1];
  if(p[0] == '-') {
    if(argc < 3){
      printf(2, "usage: kill -signum pid...\n");
      exit();
    }

    p++;
    int signum = atoi(p);
    for(int i = 2; i < argc; i++){
      int pid = atoi(argv[i]);
      if(kill(pid, signum) < 0){
        printf(2, "kill: failed to send signal %d to %d\n", signum, pid);
      }
    }
  } else {
    for(i = 1; i < argc; i++){
      int pid = atoi(argv[i]);
      if(kill(pid, SIGTERM) < 0){
        printf(2, "kill: failed to kill %d\n", pid);
      }
    }
  }

  exit();
}
