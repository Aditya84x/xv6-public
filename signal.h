#define NSIGNALS 32
typedef void (*sighandler_t)(int);

#define SIG_DFL (void (*)())0
#define SIG_IGN (void (*)())1

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

#define SIGINT    2   
#define SIGILL    4   
#define SIGKILL   9   // Kill (cannot be caught or ignored)
#define SIGUSR1   10  // User-defined signal 1
#define SIGSEGV   11  // Invalid memory reference
#define SIGTERM   15  // Termination signal
#define SIGCHLD   17  // Child process terminated or stopped
#define SIGCONT   18  // Continue executing, if stopped
#define SIGSTOP   19  
#define SIGVTALRM   26
