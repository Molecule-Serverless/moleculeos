#include<stdio.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>

#include <global_syscall_protocol.h>
#include <global_syscall.h>

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
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    listen(listenfd,20);
    efd = epoll_create(MAX_OPEN_FD);
    tep.events = EPOLLIN;
    tep.data.fd = listenfd;

    ret = epoll_ctl(efd,EPOLL_CTL_ADD,listenfd,&tep);

    fprintf(stderr, "[%s] globalOS connection inited\n", __func__);

    for (;;)
    {
        size_t nready = epoll_wait(efd,ep,MAX_OPEN_FD,-1);
        //printf("nready = %ld\n", nready);
        for (int i = 0; i < nready; ++i)
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
		    }else
		    //FIFO_INIT
		    if (strcmp(func_name, "FIFO_INIT")) {
			ret = syscall_fifo_init(func_args1, func_args2);
		    }
		    //FIFO_READ
		    if (strcmp(func_name, "FIFO_READ")) {
			ret = syscall_fifo_read(func_args1, func_args2, func_args3);
		    }
		    //FIFO_WRITE
		    if (strcmp(func_name, "FIFO_WRITE")) {
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

int global_os_init(void)
{
	int i;
	for (i=0; i<GLOBAL_PROCESS_LIST_SIZE; i++)
		global_process_list[i] = 0;
	for (i=0; i<GLOBAL_FIFO_LIST_SIZE; i++) {
		//we use pu_id to indicate whether this is a valid fifo
		global_fifo_list[i].pu_id = -1;
	}

	//FIXME: different global-OS on different PU has different pu_id
	current_pu_id = 0;
	return 0;
}

/*===================Begin of Global Process */
/*===================End of Global Process */


/*===================Begin of Global FIFO */
int syscall_fifo_init(int local_uuid, int owner_pid)
{
	int ret;

	global_fifo_now = (global_fifo_now+1) % GLOBAL_FIFO_LIST_SIZE;

	if (global_fifo_list[global_fifo_now].pu_id == -1){
		//find an empty entry
		global_fifo_list[global_fifo_now].pu_id = current_pu_id;
		global_fifo_list[global_fifo_now].global_id = global_fifo_now;
		ret = global_process_now;


		global_fifo_list[global_fifo_now].local_uuid = local_uuid;
		global_fifo_list[global_fifo_now].owner_pid = owner_pid;
		global_fifo_list[global_fifo_now].perms = {0x3, owner_pid};

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
	return 0;
}

int syscall_fifo_write(int global_fifo, int shmid, int length)
{
	fprintf(stderr, "[Warning@%s] syscall not supported\n", __func__);
	return 0;
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
