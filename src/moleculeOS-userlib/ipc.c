#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <molecule-ipc.h>

/* Helper Functions */
void throw(const char* message);
#if 0
void throw(const char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}
#endif


/* FIFO-based IPC */
int fifo_init(void) //return a self-fifo (named using self pid).
{
	char fifo_path[FIFO_PATH_LEN]="";
	int fifo_fd;
	int uuid = getpid();
	sprintf(fifo_path, FIFO_PATH_TEMPLATE, uuid);
	fprintf(stderr, "[%s] fifo_path: %s\n",__func__, fifo_path);
	if (mkfifo(fifo_path, 0666) > 0) {
		throw("Error creating FIFO in server\n");
	}
	fprintf(stderr, "[%s] mkfifo success\n",__func__);

	/* Open a self-fifo, read-only */
	if ((fifo_fd = open(fifo_path, O_RDWR)) <0){
		throw("Error opening FIFO in server\n");
	}
	fprintf(stderr, "[%s] open success\n",__func__);

	return fifo_fd;
}

int fifo_client_setup(int uuid) {
	char fifo_path[FIFO_PATH_LEN]="";
	int fifo_fd;
	sprintf(fifo_path, FIFO_PATH_TEMPLATE, uuid);
	if (mkfifo(fifo_path, 0666) > 0) {
		throw("Error creating FIFO in server\n");
	}

	/* Open a fifo */
	if ((fifo_fd = open(fifo_path, O_WRONLY)) <0){
		throw("Error opening FIFO in server\n");
	}

	return fifo_fd;
}

//int _fifo_server_setup(char* fifo_path) {
int fifo_server_setup(int uuid) {
	char fifo_path[FIFO_PATH_LEN]="";
	/* It always assume the client is alreay setup */
	int fifo_fd;

	sprintf(fifo_path, FIFO_PATH_TEMPLATE, uuid);

	/* Open a fifo */
	if ((fifo_fd = open(fifo_path, O_RDONLY)) <0){
		throw("Error opening FIFO in client\n");
	}
	return fifo_fd;
}

int fifo_connect(int uuid) //connect to fifo-uuid, return a fifo_fd
{
	char fifo_path[FIFO_PATH_LEN]="";
	/* It always assume the client is alreay setup */
	int fifo_fd;

	sprintf(fifo_path, FIFO_PATH_TEMPLATE, uuid);

	/* Open a fifo (of a server), write-only */
	if ((fifo_fd = open(fifo_path, O_WRONLY)) <0){
		throw("Error opening FIFO in client\n");
	}
	return fifo_fd;
}


void fifo_finish(int fifo_fd) {
	close(fifo_fd);
}

void fifo_close(int fifo_fd) {
	close(fifo_fd);
}

/*Note(DD): fifo_clean will not close a fifo_fd, please do it through fifo_finish */
void fifo_clean(void) //destory self-fifo
{
	char fifo_path[FIFO_PATH_LEN]="";
	int fifo_fd;
	int uuid = getpid();

	sprintf(fifo_path, FIFO_PATH_TEMPLATE, uuid);
	fprintf(stderr, "[%s] fifo_path: %s\n",__func__, fifo_path);

	if (remove(fifo_path) == -1) {
		throw("Error removing FIFO");
	}
}

int fifo_read(int fifo_fd, char* buf, int len){
	int i;
	assert(len>0);
	assert(len<=MAX_MSG_LEN);

	i = read(fifo_fd, buf, len);
#ifdef FIFO_DEBUG
	fprintf(stderr, "[%s] read content: %s, len(%d)\n", __func__, buf, i);
#endif
	return i;
}

int fifo_write(int fifo_fd, char*buf, int len){
	int i;
	assert(len>0);
	assert(len<=MAX_MSG_LEN);

#ifdef FIFO_DEBUG
	fprintf(stderr, "[%s] write content: %s, len:%d\n", __func__, buf, len);
#endif
	i = write(fifo_fd, buf, len);
#ifdef FIFO_DEBUG
	fprintf(stderr, "[%s] writeen len:%d\n", __func__, i);
#endif
	return i;
}


/* SHM-based IPC */


/* Signal-based IPC */
void signal_notify_server(int pid) {
	kill(pid, SIGUSR1);
}

void signal_notify_client(int pid) {
	kill(pid, SIGUSR2);
}

volatile int signalpid = -1;
void signal_server_handler(int sig, siginfo_t *info, void *context)
{
	signalpid = info->si_pid;
	//unsafe
	fprintf(stderr, "%s invoked\n", __func__);
}
int signal_register_server_handler()
{
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signal_server_handler;
	sigaction(SIGUSR1, &sa, NULL);
}


/* DMA/RDMA-based IPC */
