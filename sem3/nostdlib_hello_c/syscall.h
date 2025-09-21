#ifndef _SYSCALL_H
#define _SYSCALL_H

long syscall_write_1(long fd, void *buffer, long size);
void syscall_exit_60(long status);

#endif /* _SYSCALL_H */
