#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
The following code is based on
"Capture the output of a child process in C"
http://www.microhowto.info/howto/capture_the_output_of_a_child_process_in_c.html
*/
int main() {

  int filedes[2];
  if (pipe(filedes) == -1) {
    perror("pipe");
    exit(1);
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(1);
  } 

  else if (pid == 0) {
    while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) { }

    close(filedes[1]);
    close(filedes[0]);
    execl("/bin/pwd", "pwd", (char*)0);
    perror("execl");
    _exit(1);
  }

  close(filedes[1]);

  char buffer[4096];

  while (1) {
    ssize_t count = read(filedes[0], buffer, sizeof(buffer));
    if (count == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        perror("read");
        exit(1);
      }
    } else if (count == 0) {
       break;
    } else {
      // handle_child_process_output(buffer, count);
      printf("%s\n", buffer);
    }
  }

  close(filedes[0]);
  wait(0);

  return 0;
}

