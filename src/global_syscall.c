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
global_process_t global_process_list[GLOBAL_PROCESS_LIST_SIZE];
static int global_process_now = 0;

#define LOCAL_FIFO_LIST_SIZE 512
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
int syscall_fifo_connect(int global_fifo);

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
		    //fprintf(stderr, "[%s] Global syscall invoked: %s\n", __func__, buf);

		    sscanf(buf, SYSCALL_REQ_FORMAT, &client_id, func_name, &func_args1,
				    &func_args2, &func_args3, &func_args4);

		    //fprintf(stderr, "[%s] after parse requests, func_name:%s\n", __func__, func_name);

		    /*Handler dispatch */
		    //RegisterSelfGlobal
		    if (strcmp(func_name, "RegisterSelfGlobal") == 0) {
			    global_process_now = (global_process_now+1)%GLOBAL_PROCESS_LIST_SIZE;
			    if (global_process_list[global_process_now].local_pid<0){
				key_t segment_key;
				int segment_id;
				int shm_uuid = 1;
				char shmid_string[256];

			    	global_process_list[global_process_now].pu_id = current_pu_id;
			    	global_process_list[global_process_now].local_pid = func_args1;
			    	global_process_list[global_process_now].global_pid = global_process_now;
				ret = global_process_now;

				/* establish shm now */
				shm_uuid = ret; //global_process_now as uuid
				sprintf(shmid_string, "%d", shm_uuid);
				segment_key = generate_key(shmid_string);
				segment_id = shmget(segment_key, 4096, IPC_CREAT | 0666);

				if (segment_id < 0) {
					throw("Could not get segment");
				}

			    	global_process_list[global_process_now].shm = shmat(segment_id, NULL, 0);
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
		   //fprintf(stderr, "[%s] Global syscall(%s) result: %d\n", __func__, func_name, ret);


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
	
	/* FIXME: the initialization should sync with other PUs */
	for (i=0; i<GLOBAL_PROCESS_LIST_SIZE; i++){
		global_process_list[i].pu_id = -1;
		global_process_list[i].local_pid = -1;
		global_process_list[i].global_pid = i;
		global_process_list[i].shm = NULL;
	}

	for (i=0; i<GLOBAL_FIFO_LIST_SIZE; i++) {
		//we use pu_id to indicate whether this is a valid fifo
		global_fifo_list[i].pu_id = -1;
	}

	//FIXME: different global-OS on different PU has different pu_id
	current_pu_id = pu_id;
	global_os_port = os_port + pu_id; // we always use pu_id as an offset to change the port

	global_fifo_now = current_pu_id * LOCAL_FIFO_LIST_SIZE;


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

	/* Static partition global_fifo_id */
	if (global_fifo<current_pu_id *LOCAL_FIFO_LIST_SIZE)
		return 0;
	if (global_fifo>= (current_pu_id+1) *LOCAL_FIFO_LIST_SIZE)
		return 0;


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

int syscall_fifo_connect(int global_fifo)
{
	fprintf(stderr, "[Warning@%s] syscall not supported\n", __func__);
	return 0;
}


int syscall_fifo_read(int global_fifo, int shmid, int length)
{
#ifdef DEBUG
	fprintf(stderr, "[Info@%s] syscall invoked\n", __func__);
#endif
#ifdef SMARTC
	if (!is_global_fifo_local(global_fifo)){
		//remote case
		//return read_remote_fifo(global_fifo, (char*)shared_memory, length);
		return -1;
	}
#endif

	//let the process read it locally
	return -EFIFOLOCAL;
}

#ifdef SMARTC
int read_remote_fifo(int global_fifo, char* shared_memory, int length)
{
	fprintf(stderr, "[Warning@%s] syscall not supported\n", __func__);
	return 0;

}
int write_remote_fifo(int global_fifo, char* shared_memory, int length)
{
//#define DSM_REQ_FORMAT "gpid: %d func:%s args1:%d args2:%d args3:%d args4:%d buf_len:%d "
//#define DSM_RSP_FORMAT "ret: %d"
	char buffer[4096];
	int ret;
	ret = sprintf(buffer, DSM_REQ_FORMAT, 0, "WRITEFIFO", global_fifo, 0, 0, 0, length);
	assert(ret+length<4096);
	memcpy(buffer+ret, shared_memory, length);
	buffer[ret+length] = '\0';

	dsm_call(buffer, 4096);

	return 0;
}
#endif
int write_local_fifo(int global_fifo, char* shared_memory, int length)
{
	int local_uuid = -1;
	int local_fifo;
	int ret;
	//TODO: check global_fifo to avoid attacks
	local_uuid = global_fifo_list[global_fifo].local_uuid;
	local_fifo = fifo_connect(local_uuid);
	ret = fifo_write(local_fifo, (char*)shared_memory, length);
	fifo_close(local_fifo);
	return ret;
}

int syscall_fifo_write(int global_fifo, int shmid, int length)
{
	/* Test of shm */
	char* shared_memory;
	// Key for the memory segment
#if 0
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
#else
	/* We can directly got the shm through process_list*/
	shared_memory = global_process_list[shmid].shm;
#endif

	if (shared_memory < (char*)0) {
		//throw("Could not attach segment");
		fprintf(stderr, "[Error@%s] can not attach segment\n", __func__);
		return -1;
	}
	//*((int *)shared_memory) = 0xbeef;
	shared_memory[length] = '\0';

	/* Check whether the syscall can finished locally */
#ifdef SMARTC
	if (!is_global_fifo_local(global_fifo)){
		//remote case
		return write_remote_fifo(global_fifo, (char*)shared_memory, length);
	}
#endif

	/* Write data locally */
	//1. write fifo
	{
	write_local_fifo(global_fifo, (char*)shared_memory, length);
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
