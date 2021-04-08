#ifndef GLOBAL_SYSCALL_H
#define GLOBAL_SYSCALL_H

#include<stdio.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>

#include <global_syscall_protocol.h>

int global_syscall_loop(void);
int global_os_init(void);

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
	perm_container_t perms; //The containers that can access this FIFO
} global_fifo_t;

#endif
