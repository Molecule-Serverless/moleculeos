/*
 * This is the handlers of DSM routines in Molecule
 *
 * 	Authors: Dong Du
 * */
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>
#include<sys/shm.h>
#include<assert.h>

#include <chos/errno.h>

#include <molecule_dsm.h>
#include <global_syscall_protocol.h>
#include <global_syscall.h>

/* DSM handlers list */
//int syscall_fifo_init(int local_uuid, int owner_pid);
//int syscall_fifo_close(int global_fifo);
//int syscall_fifo_read(int global_fifo, int shmid, int length);
//int syscall_fifo_write(int global_fifo, int shmid, int length);

//End of DSM handler list
	
int dsm_handlers(char* dsm_call_buf, int len)
{
	int client_id, func_args1, func_args2, func_args3, func_args4, buf_len, buf_offset;
	char func_name[256];
	char sys_resp[256];
	int ret;

	fprintf(stderr, "[%s] DSM handlers invoked: %s\n", __func__, dsm_call_buf);

	ret = sscanf(dsm_call_buf, DSM_REQ_FORMAT_CALLEE, &client_id, func_name, &func_args1,
			    &func_args2, &func_args3, &func_args4, &buf_len, &buf_offset);

	//assert(ret + buf_len < len);
	assert(buf_offset + buf_len < len);

	if (strcmp(func_name, "WRITEFIFO") == 0) {
	   	fprintf(stderr, "[%s] func(%s) buf_len(%d) buf(%s)\n", __func__, func_name,
				buf_len, dsm_call_buf+buf_offset);
		ret = write_local_fifo(func_args1, dsm_call_buf+buf_offset, buf_len);
	}
	else{
	   //unsupported
		ret = -1;
	   	fprintf(stderr, "[%s] DSM handlers (%s) unknown\n", __func__, func_name);
	}
	
	return ret;
}
