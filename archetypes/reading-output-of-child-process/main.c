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

  int stdout_file_descriptor[2];
  int stderr_file_descriptor[2];
  if (pipe(stdout_file_descriptor) == -1) {
    perror("stdout pipe");
    exit(1);
  }

  if (pipe(stderr_file_descriptor) == -1) {
    perror("stderr_pipe");
    exit(1);
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(1);
  } 

  else if (pid == 0) {
    while ((dup2(stdout_file_descriptor[1], STDOUT_FILENO) == -1) && (errno == EINTR)) { }
    while ((dup2(stderr_file_descriptor[1], STDERR_FILENO) == -1) && (errno == EINTR)) { }

    close(stdout_file_descriptor[1]);
    close(stdout_file_descriptor[0]);
    close(stderr_file_descriptor[1]);
    close(stderr_file_descriptor[0]);

    execl("printing_child", "printing_child", (char*)0);

    perror("execl");
    _exit(1);
  }

  close(stdout_file_descriptor[1]);
  close(stderr_file_descriptor[1]);

  char child_stdout_buffer[4096];
  char child_stderr_buffer[4096];

  while (1) {
    ssize_t count = read(stdout_file_descriptor[0], child_stdout_buffer, sizeof(child_stdout_buffer));
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
      // handle_child_process_output(child_stdout_buffer, count);
      printf("child_prints stdout: %s\n", child_stdout_buffer);
    }
  }
  while (1) {
    ssize_t count = read(stderr_file_descriptor[0], child_stderr_buffer, sizeof(child_stderr_buffer));
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
      printf("child_prints stderr: %s\n", child_stderr_buffer);
    }
  }

  close(stdout_file_descriptor[0]);
  close(stderr_file_descriptor[0]);

  wait(0);

  return 0;
}

