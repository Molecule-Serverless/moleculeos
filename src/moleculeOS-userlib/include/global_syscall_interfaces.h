/*
 * Molecule Global Syscall Runtime
 * 	This is runtime provides interfaces to send commands to globalOS
 * 	and hadnle the responses
 * */
#ifndef GLOBAL_SYSCALL_RUNTIME_H
#define GLOBAL_SYSCALL_RUNTIME_H

//#define SYSCALL_MSG_FORMAT "id: %d func:%s args1:%d args2:%d args3:%d args4:%d"

int register_self_global(int os_port); //return a global pid
int unregister_self_global(void); //return a status code
int global_fifo_init(int local_fifo); //return a global fifo_fd
int global_fifo_close(int global_fifo); //close a global fifo_fd
int global_fifo_read(int global_fifo, char*buf, int len); //read from a global fifo_fd
int global_fifo_write(int global_fifo, char*buf, int len); //write to a global fifo_fd
int global_grant_perm(int global_pid, int global_fd, int perm); //grant perm of a fifo to another process


#endif
