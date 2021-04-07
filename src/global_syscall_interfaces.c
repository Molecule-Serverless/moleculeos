/*
 * Molecule Global Syscall Interfaces
 * 	The interfaces provided to processes
 * */

#include <stdlib.h>
#include <stdio.h>
#include <global_syscall_protocol.h>
#include <global_syscall_interfaces.h>
#include <global_syscall_runtime.h>

static int global_OS_id = -1;
static int self_global_id = -1;

int register_self_global(void) //return a global pid
{
	char buffer[256];

	fprintf(stderr, "[%s] invoked\n", __func__);

	/* Note: we should only register once */
	global_OS_id = connect_global_OS();

	sprintf(buffer, SYSCALL_REQ_FORMAT, 0, "RegisterSelfGlobal", 0, 0, 0, 0);
	self_global_id = invoke_global_syscall(global_OS_id, buffer);

	fprintf(stderr, "[%s] resp: self_global_id: %d\n", __func__, self_global_id);

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


