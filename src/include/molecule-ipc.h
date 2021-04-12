#ifndef MOLECULE_IPC_H
#define MOLECULE_IPC_H

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Gloabl settings */
#define FIFO_PATH_LEN 66
#define MAX_MSG_LEN 4096
//#define FIFO_DEBUG 1

//#define FIFO_PATH_TEMPLATE "/tmp/ipc_fifo_id-%d"
//#define FIFO_PATH_TEMPLATE "/env/ipc_fifo_id-%d"
#define FIFO_PATH_TEMPLATE "/tmp/ipc_fifo_id-%d"

#define FIFO_MSG_FORMAT "id: %d func:%s args1:%d args2:%d args3:%d args4:%d"

/*
 * Explain (DD): Each fifo is owned by the id (in the name)
 * 		 The owner will only read from the fifo
 * 		 Any sender can send data to the fifo (can not read)
 *
 * 	In C-S model:
 * 		Client: write data to server's FIFO (msg format with client's ID)
 * 		Server: read data from server's FIFO
 * 		Server: write response to client's FIFO
 * 		Client: read response from client's FIFO
 *
 * 	Issues:
 * 		1. How to ensure Client will not forge the "client's ID" in msg?
 * */

/* FIFO-based IPC */

//The two interfaces are deprecated, just for compaitbility use in Molecule
int fifo_client_setup(int uuid); //return a fifo_fd
int fifo_server_setup(int uuid); //return a fifo_fd

int fifo_init(void); //return a self-fifo (named using self pid).
int fifo_connect(int uuid); //connect to fifo-uuid, return a fifo_fd

void fifo_close(int fifo_fd) ;
void fifo_finish(int fifo_fd); //close a fifo_fd
void fifo_clean(void); //close and destory self-fifo

int fifo_read(int fifo_fd, char* buf, int len); //read from a fifo_fd
int fifo_write(int fifo_fd, char*buf, int len); //write to a fifo_fd


/* SHM-based IPC */


/* Signal-based IPC */
extern volatile int signalpid;
//static volatile int signalpid = -1;
void signal_notify_server(int pid);
void signal_notify_client(int pid);
int signal_register_server_handler(void);

/* DMA/RDMA-based IPC */

#endif
