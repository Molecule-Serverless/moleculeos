/*
 * Molecule Global Syscall Interfaces
 * 	The interfaces provided to processes
 * */

#include <stdlib.h>
#include <stdio.h>
#include <global_syscall_interfaces.h>

int register_self_global(void) //return a global pid
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	return 0;
}


int unregister_self_global(void) //return a status code
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	return 0;
}
int global_fifo_init(int local_fifo) //return a global fifo_fd
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	return 0;
}
int global_fifo_close(int global_fifo) //close a global fifo_fd
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	return 0;
}
int global_fifo_read(int global_fifo, char*buf, int len) //read from a global fifo_fd
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	return 0;
}
int global_fifo_write(int global_fifo, char*buf, int len) //write to a global fifo_fd
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	return 0;
}
int global_grant_perm(int global_pid, int global_fd, int perm) //grant perm of a fifo to another process
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	return 0;
}


