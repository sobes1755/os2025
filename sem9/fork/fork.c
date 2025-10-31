#define _POSIX_C_SOURCE 200809L	// getpgid

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int
main(void)
{

    printf("Before fork: ppid = %d, pid = %d, pgid = %d.\n", getppid(), getpid(), getpgid(getpid()));

#ifdef PARENT_FLUSHES_BEFORE_FORK
    fflush(stdout);
#endif

    pid_t pid = fork();

    printf("After fork: ppid = %d, pid = %d, pgid = %d.\n", getppid(), getpid(), getpgid(getpid()));

    if (pid > 0) {    // Parent

        int status;

        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Process %d exited with statud %d.\n\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process %d terminated with signal %d.\n\n", pid, WTERMSIG(status));
        }

    } else if (pid == 0) {    // Child

        setpgid(getpid(), getpid());

        printf("After fork: ppid = %d, pid = %d, pgid = %d.\n", getppid(), getpid(), getpgid(getpid()));
        
        sleep(2);

#ifdef CHILD_EXITS_WITH_UNDERSCORE
        _exit(EXIT_FAILURE);
#endif

    } else if (pid < 0) {

        printf("Fork failed: errno = %d, errmsg = %s!\n", errno, strerror(errno));

    }

    return EXIT_SUCCESS;

}
