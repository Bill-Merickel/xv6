// init: The initial user-level program
// adjusted to utilize the login system

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "login.h"

char *argv[] = { "login", 0 };

int main(void) {
  int pid, wpid;

  if (open("console", O_RDWR) < 0) {
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  init_hook();

  for (;;) {
    printf(1, "\ninit: starting login\n");
    pid = fork();
    if (pid < 0) {
      printf(2, "init: fork failed\n");
      exit();
    }
    if (pid == 0) {
      exec("login", argv);
      printf(2, "init: exec login failed\n");
      exit();
    }
    while ((wpid = wait()) >= 0 && wpid != pid) {
      printf(2, "zombie!\n");
    }
  }
}
