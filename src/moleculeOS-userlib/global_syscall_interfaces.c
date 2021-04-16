/*
 * Molecule Global Syscall Interfaces
 * 	The interfaces provided to processes
 * */

#include <stdlib.h>
#include <stdio.h>
#include <global_syscall_protocol.h>
#include <global_syscall_runtime.h>
#include <global_syscall_interfaces.h>
#include <sys/shm.h>
#include <assert.h>

#include <common/common.h>

static int global_OS_id = -1;
static int self_global_id = -1;


static int global_OS_shm_client_uuid = -1;
static void* global_OS_shm_client_addr = NULL;
/*
 * It should be invoked before other global functions
 * */
int register_self_global(int os_port) //return a global pid
{
	char buffer[256];
	key_t segment_key;
	int segment_id;
	int shm_uuid = -1; //default connection_id
	char shmid_string[256];
	char* shared_memory;

	fprintf(stderr, "[%s] invoked\n", __func__);

	/* Note: we should only register once */
	global_OS_id = connect_global_OS(os_port);

	sprintf(buffer, SYSCALL_REQ_FORMAT, 0, "RegisterSelfGlobal", 0, 0, 0, 0);
	self_global_id = invoke_global_syscall(global_OS_id, buffer);

	fprintf(stderr, "[%s] resp: self_global_id: %d\n", __func__, self_global_id);

	/* 
	 * We pass one 1-shm to the globalOS now (for bulk data transfer),
	 * so we can avoid shm-establish costs after that 
	 * */
	//we use the self_global_id as shm_uuid, which is supposed managed by globalOS
	shm_uuid = self_global_id; 
	sprintf(shmid_string, "%d", shm_uuid);
	segment_key = generate_key(shmid_string);
	segment_id = shmget(segment_key, 4096, IPC_CREAT | 0666);

	if (segment_id < 0) {
		throw("Could not get segment");
	}

	shared_memory = (char*)shmat(segment_id, NULL, 0);

	if (shared_memory < (char*)0) {
		throw("Could not attach segment");
	}
	global_OS_shm_client_uuid = self_global_id;
	global_OS_shm_client_addr = shared_memory;


	return 0;
}


int unregister_self_global(void) //return a status code
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	return 0;
}

/*
 * Make a fifo global means:
 * 	1. The fifo has a global fifo ID
 * 	2. The fifo can be read/write by anyone in a smartcomputer
 * */
int global_fifo_init(int local_fifo) //return a global fifo_fd
{
	int global_fifo_id;
	char buffer[256];
	fprintf(stderr, "[%s] invoked\n", __func__);

	//fifo_init(local_fifo);
	sprintf(buffer, SYSCALL_REQ_FORMAT, self_global_id, "FIFO_INIT", local_fifo, 0, 0, 0);
	global_fifo_id = invoke_global_syscall(global_OS_id, buffer);

	fprintf(stderr, "[%s] resp: global_fifo_id: %d\n", __func__, global_fifo_id);

	return global_fifo_id;
}

int global_fifo_close(int global_fifo) //close a global fifo_fd
{
	int ret;
	char buffer[256];
	fprintf(stderr, "[%s] invoked\n", __func__);

	//fifo_close(global_fifo);
	sprintf(buffer, SYSCALL_REQ_FORMAT, self_global_id, "FIFO_CLOSE", global_fifo, 0, 0, 0);
	ret = invoke_global_syscall(global_OS_id, buffer);

	return ret;
}

int global_fifo_read(int global_fifo, char*buf, int len) //read from a global fifo_fd
{
	int ret;
	char buffer[64];
	//key_t segment_key;
	//int segment_id;
	//Note: Get shm from global
	int shm_uuid = global_OS_shm_client_uuid;
	char* shared_memory = global_OS_shm_client_addr;

#if 0 /* Establish shm on-demand*/
	char shmid_string[256];

	sprintf(shmid_string, "%d", shm_uuid);
	segment_key = generate_key(shmid_string);
	segment_id = shmget(segment_key, 4096, IPC_CREAT | 0666);

	if (segment_id < 0) {
		throw("Could not get segment");
	}

	shared_memory = (char*)shmat(segment_id, NULL, 0);

	if (shared_memory < (char*)0) {
		throw("Could not attach segment");
	}
	*((int *)shared_memory) = 0xbeef;
#endif

	/* FIXME: we need a shm mechanism here */
#ifndef MOLECULE_CLEAN
	fprintf(stderr, "[%s] invoked\n", __func__);
#endif

	sprintf(buffer, SYSCALL_REQ_FORMAT, self_global_id, "FIFO_READ", global_fifo, shm_uuid, len, 0);
	ret = invoke_global_syscall(global_OS_id, buffer);

	if (ret<0)
		return ret;

	if (ret>8192 || ret>len){
		fprintf(stderr, "[%s] msg too long\n", __func__);
		return -1;
	}

	memcpy(buf, shared_memory, ret);

	return ret;
}

int global_fifo_write(int global_fifo, char*buf, int len) //write to a global fifo_fd
{
	int ret;
	char buffer[256];
	// Key for the memory segment
#if 0
	key_t segment_key;
	int segment_id;
	int shm_uuid = 1;
	char shmid_string[256];
#endif
	int shm_uuid = global_OS_shm_client_uuid;
	char* shared_memory = global_OS_shm_client_addr;

#if 0
	sprintf(shmid_string, "%d", shm_uuid);

	//segment_key = generate_key(itoa(shm_uuid));
	segment_key = generate_key(shmid_string);
	segment_id = shmget(segment_key, 4096, IPC_CREAT | 0666);

	if (segment_id < 0) {
		throw("Could not get segment");
	}

	shared_memory = (char*)shmat(segment_id, NULL, 0);

	if (shared_memory < (char*)0) {
		throw("Could not attach segment");
	}
	*((int *)shared_memory) = 0xbeef;
	/* FIXME: we need a shm mechanism here */
#endif

	assert(len<=4096);

	memcpy(shared_memory, buf, len);

	sprintf(buffer, SYSCALL_REQ_FORMAT, self_global_id, "FIFO_WRITE", global_fifo, shm_uuid, len, 0);
	ret = invoke_global_syscall(global_OS_id, buffer);

#ifndef MOLECULE_CLEAN
	fprintf(stderr, "[%s] invoked\n", __func__);
#endif
	return ret;
}

int global_grant_perm(int global_pid, int global_fd, int perm) //grant perm of a fifo to another process
{
	fprintf(stderr, "[%s] invoked\n", __func__);
	fprintf(stderr, "[%s] unsupported yet\n", __func__);
	return 0;
}
