#include "syscall.h"

void _start(void) {

    char msg[] = "Hello, World!\n";

    long written = syscall_write_1(1, msg, sizeof(msg));

    syscall_exit_60(written);

}
