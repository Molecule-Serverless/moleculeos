/*
 * Molecule Global Syscall Runtime
 * 	This is runtime provides interfaces to send commands to globalOS
 * 	and hadnle the responses
 * */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <assert.h>

#include <global_syscall_runtime.h>

int connect_global_OS(int os_port)
{
	int sockfd = 0;
	struct sockaddr_in serv_addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	//serv_addr.sin_port = htons(GLOBAL_OS_PORT);
	serv_addr.sin_port = htons(os_port);
#if 1
	if(inet_pton(AF_INET, GLOBAL_OS_IP, &serv_addr.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		return 1;
	}
#endif

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect GlobalOS Failed \n");
		return 1;
	}

	return sockfd;
}

int invoke_global_syscall(int global_os_fd, char * syscall)
{
	char recvBuff[32];
	int n;
	int ret;

	/*FIXME: memset maybe necessary but costful */
	//memset(recvBuff, '0',sizeof(recvBuff));

#ifndef MOLECULE_CLEAN
	fprintf(stderr, "[GlobalOS Runtime] Issue Syscall: %s\n", syscall);
#endif
	write(global_os_fd, syscall, strlen(syscall));

#if 0
	while ( (n = read(global_os_fd, recvBuff, sizeof(recvBuff)-1)) > 0)
	{
		recvBuff[n] = 0;
		fprintf(stderr, "[GlobalOS Runtime] Syscall Return: %s\n", recvBuff);
	}
#else
	n = read(global_os_fd, recvBuff, sizeof(recvBuff)-1);
	assert(n>0);
	recvBuff[n] = 0;
	#ifndef MOLECULE_CLEAN
	fprintf(stderr, "[GlobalOS Runtime] Syscall Return: %s\n", recvBuff);
	#endif
#endif
	sscanf(recvBuff, SYSCALL_RSP_FORMAT, &ret);

	return ret;
	return 0;
}

#define DEBUG 0
#if DEBUG //for debug use
int main()
{
	int global_os = -1;
	global_os = connect_global_OS();
	invoke_global_syscall(global_os, "func:fifo_write msg:hello-world len:11");
	return 0;
}
#endif
