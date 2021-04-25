/*
 * This includes the declaration and structures of MoleculeOS
 *
 * 		Authors: Dong Du
 * */
#ifndef GLOBAL_SYSCALL_H
#define GLOBAL_SYSCALL_H

#include<stdio.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>

#include <global_syscall_protocol.h>

/* Syscall and global methods */
int global_syscall_loop(void);
int global_os_init(int pu_id, int os_port);
int get_current_pu_id(void);

/* Global FIFO methods */
int is_global_fifo_local(int global_fifo);
int write_local_fifo(int global_fifo, char* shared_memory, int length);
int local_fifo_connect(int global_uuid);


/* Structure of Permission containers */
typedef struct perm_container{
	/* Perm:
	 * 	0x0: No-perm
	 * 	0x1: read-only
	 * 	0x2: write-only
	 * 	0x3: read-write
	 * */
	int perm;
	int container_id; //globally unique
} perm_container_t;

/* Structure of Global FIFOs */
typedef struct global_fifo{
	int pu_id; //the PU of the fifo
	int owner_pid; //the local PID (in the pu) of the owner process of the global_fifo
	//int local_fifo; //The fifo id in the owner process
	int local_uuid; //The uuid of the fifo, i.e., the name
	int global_id; //The global fifo id
	int global_uuid; //The uuid of global fifo, which is used in gFIFO_connect
	perm_container_t perms; //The containers that can access this FIFO
} global_fifo_t;

/* Structure of Global Processes */
typedef struct global_process{
	int pu_id; //the PU of the fifo
	int local_pid; //the local PID (in the pu) of the owner process
	int global_pid; //The global pid
	volatile char * shm; //shm for communicating with each shm (only useful when globalOS and GP on same PU)
} global_process_t;

#endif
