/*
 * This is the codes of MoleculeOS main logics, including
 * 	Syscall handling for global requests issued by local processes
 * 	Operations implemented for global structures, e.g., global processe and global FIFO
 *
 * 	
 * 	Author: Dong Du
 * */
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>
#include<sys/shm.h>

#include <molecule-ipc.h>
#include <global_syscall_protocol.h>
#include <global_syscall.h>
#include <chos/errno.h>

#define MAXLEN 1024
#define SERV_PORT 0xfeeb //The reverse of 0xbeef
#define MAX_OPEN_FD 1024

/* Global Process Table */
#define GLOBAL_PROCESS_LIST_SIZE 4096
int global_process_list[GLOBAL_PROCESS_LIST_SIZE];
static int global_process_now = 0;

#define GLOBAL_FIFO_LIST_SIZE 4096
global_fifo_t global_fifo_list[GLOBAL_FIFO_LIST_SIZE];
static int global_fifo_now = 0;

static int current_pu_id = -1;
static int global_os_port = -1;

/* syscall handlers list */
int syscall_fifo_init(int local_uuid, int owner_pid);
int syscall_fifo_close(int global_fifo);
int syscall_fifo_read(int global_fifo, int shmid, int length);
int syscall_fifo_write(int global_fifo, int shmid, int length);

//End of syscall handler list

int global_syscall_loop(void)
{
    int  listenfd,connfd,efd,ret;
    char buf[MAXLEN];
    struct sockaddr_in cliaddr,servaddr;
    socklen_t clilen = sizeof(cliaddr);
    struct epoll_event tep,ep[MAX_OPEN_FD];
    //char * unsupported = "Syscall unsupported";

    listenfd = socket(AF_INET,SOCK_STREAM,0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_port = htons(global_os_port);
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    listen(listenfd,20);
    efd = epoll_create(MAX_OPEN_FD);
    tep.events = EPOLLIN;
    tep.data.fd = listenfd;

    ret = epoll_ctl(efd,EPOLL_CTL_ADD,listenfd,&tep);

    fprintf(stderr, "[%s] globalOS connection inited\n", __func__);

    for (;;)
    {
	int i;
        size_t nready = epoll_wait(efd,ep,MAX_OPEN_FD,-1);
        //printf("nready = %ld\n", nready);
        for (i = 0; i < nready; ++i)
        {
            if (ep[i].data.fd == listenfd){
                connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
                tep.events = EPOLLIN;
                tep.data.fd = connfd;
                ret = epoll_ctl(efd,EPOLL_CTL_ADD,connfd,&tep);
		fprintf(stderr, "[%s] new process connected\n", __func__);
            }else{
                connfd = ep[i].data.fd;
                int bytes = read(connfd,buf,MAXLEN);
                if (bytes == 0){
                    ret =epoll_ctl(efd,EPOLL_CTL_DEL,connfd,NULL);
                    close(connfd);
                    printf("process[%d] closed\n", i); //FIXME: the i is not a proper ID
                }
                else
                {
		    //handle global syscall here
		    int client_id, func_args1, func_args2, func_args3, func_args4;
		    char func_name[256];
		    char sys_resp[256];
		    int ret;
		    fprintf(stderr, "[%s] Global syscall invoked: %s\n", __func__, buf);

		    sscanf(buf, SYSCALL_REQ_FORMAT, &client_id, func_name, &func_args1,
				    &func_args2, &func_args3, &func_args4);

		    fprintf(stderr, "[%s] after parse requests, func_name:%s\n", __func__, func_name);

		    /*Handler dispatch */
		    //RegisterSelfGlobal
		    if (strcmp(func_name, "RegisterSelfGlobal") == 0) {
			    global_process_now = (global_process_now+1)%GLOBAL_PROCESS_LIST_SIZE;
			    if (!global_process_list[global_process_now]){
			    	global_process_list[global_process_now] = 1;
				ret = global_process_now;
			    }else{
				//FIXME: here, we do not fully use the space of the process_list
		    		fprintf(stderr, "[Error@%s] global process full\n", __func__);
				ret = -1;
			    }
		    } else
		    //FIFO_INIT
		    if (strcmp(func_name, "FIFO_INIT") == 0) {
			ret = syscall_fifo_init(func_args1, func_args2);
		    } else
		    //FIFO_READ
		    if (strcmp(func_name, "FIFO_READ") == 0) {
			ret = syscall_fifo_read(func_args1, func_args2, func_args3);
		    } else
		    //FIFO_WRITE
		    if (strcmp(func_name, "FIFO_WRITE") == 0) {
			ret = syscall_fifo_write(func_args1, func_args2, func_args3);
		    }
		    //Default
		    else{
			    //unsupported
			ret = -1;
		    	fprintf(stderr, "[%s] Global syscall(%s) unknown\n", __func__, func_name);
		    }
		    /* End of Handler dispatch */
		   fprintf(stderr, "[%s] Global syscall(%s) result: %d\n", __func__, func_name, ret);


		    sprintf(sys_resp, SYSCALL_RSP_FORMAT, ret);

		    write(connfd, sys_resp, strlen(sys_resp));
		    //write(connfd, unsupported, strlen(unsupported));
#if 0
                    for (int j = 0; j < bytes; ++j)
                    {
                        buf[j] = toupper(buf[j]);
                    }
                    write(connfd,buf,bytes);
#endif
                }
            }
        }
    }
    return 0;
}

int global_os_init(int pu_id, int os_port)
{
	int i;
	for (i=0; i<GLOBAL_PROCESS_LIST_SIZE; i++)
		global_process_list[i] = 0;
	for (i=0; i<GLOBAL_FIFO_LIST_SIZE; i++) {
		//we use pu_id to indicate whether this is a valid fifo
		global_fifo_list[i].pu_id = -1;
	}

	//FIXME: different global-OS on different PU has different pu_id
	current_pu_id = pu_id;
	global_os_port = os_port + pu_id; // we always use pu_id as an offset to change the port

	fprintf(stderr, "MoleculeOS inited, PU_ID: %d, OS_port: %d\n",
			current_pu_id, global_os_port);
	return 0;
}

int get_current_pu_id(void){
	return current_pu_id;
}

/*===================Begin of Global Process */
/*===================End of Global Process */


/*===================Begin of Global FIFO */
int check_global_fifo_bound(int global_fifo)
{
	if (global_fifo>=0 && global_fifo<GLOBAL_FIFO_LIST_SIZE){
		return 0;
	}

	/*Just Panic*/
	fprintf(stderr, "[MoleculeOS@%s] bound check error\n", __func__);
	exit(-1);

	return 0;
}

int is_global_fifo_local(int global_fifo)
{
	check_global_fifo_bound(global_fifo);
	return global_fifo_list[global_fifo].pu_id == current_pu_id;
}

int syscall_fifo_init(int local_uuid, int owner_pid)
{
	int ret;

	global_fifo_now = (global_fifo_now+1) % GLOBAL_FIFO_LIST_SIZE;

	if (global_fifo_list[global_fifo_now].pu_id == -1){
		//find an empty entry
		struct perm_container perm = {0x3, owner_pid};
		global_fifo_list[global_fifo_now].pu_id = current_pu_id;
		global_fifo_list[global_fifo_now].global_id = global_fifo_now;
		ret = global_process_now;



		global_fifo_list[global_fifo_now].local_uuid = local_uuid;
		global_fifo_list[global_fifo_now].owner_pid = owner_pid;
		//global_fifo_list[global_fifo_now].perms = {0x3, owner_pid};
		global_fifo_list[global_fifo_now].perms = perm;

	}else{
		//FIXME: here, we do not fully use the space of the list
	    	fprintf(stderr, "[Error@%s] global fifo full\n", __func__);
		ret = -1;
	}

    	//fprintf(stderr, "[%s] Global syscall(%s) result: %d\n", __func__, func_name, ret);

	return ret;
}

int syscall_fifo_close(int global_fifo)
{
	fprintf(stderr, "[Warning@%s] syscall not supported\n", __func__);
	return 0;
}

int syscall_fifo_read(int global_fifo, int shmid, int length)
{
	fprintf(stderr, "[Warning@%s] syscall not supported\n", __func__);


	//let the process read it locally
	return -EFIFOLOCAL;
	//return 0;
}

int syscall_fifo_write(int global_fifo, int shmid, int length)
{
	/* Test of shm */
	char* shared_memory;
	// Key for the memory segment
	key_t segment_key;
	int segment_id;
	int shm_uuid = 1;
	char shmid_string[256];

	sprintf(shmid_string, "%d", shmid);

	//segment_key = ftok(itoa(shmid), 'X');
	segment_key = ftok(shmid_string, 'X');
	segment_id = shmget(segment_key, 4096, IPC_CREAT | 0666);

	if (segment_id < 0) {
		//throw("Could not get segment");
		fprintf(stderr, "[Error@%s] can not get segment\n", __func__);
		return -1;
	}

	shared_memory = (char*)shmat(segment_id, NULL, 0);

	if (shared_memory < (char*)0) {
		//throw("Could not attach segment");
		fprintf(stderr, "[Error@%s] can not attach segment\n", __func__);
		return -1;
	}
	//*((int *)shared_memory) = 0xbeef;
	shared_memory[length] = '\0';

	/* TODO: Check whether the syscall can finished locally */

	/* Write data locally */
	//1. write fifo
	{
		int local_uuid = -1;
		int local_fifo;
		//TODO: check global_fifo to avoid attacks
		local_uuid = global_fifo_list[global_fifo].local_uuid;
		local_fifo = fifo_connect(local_uuid);
		fifo_write(local_fifo, (char*)shared_memory, length);
	}


	//2. check pengding and wakeup thems (?)

	//fprintf(stderr, "[Warning@%s] syscall not supported\n", __func__);
	//fprintf(stderr, "[Info@%s] shm:%s\n", __func__, shared_memory);
	return length;
}

/*===================End of Global FIFO */




#if 0 //for debug
int main(void)
{

	//init
	int i;
	for (i=0; i<GLOBAL_PROCESS_LIST_SIZE; i++)
		global_process[i] = 0;
	fprintf(stderr, "finish init\n");
	return global_syscall_loop();
}
#endif
