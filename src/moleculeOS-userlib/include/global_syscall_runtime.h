/*
 * Molecule Global Syscall Runtime
 * 	This is runtime provides interfaces to send commands to globalOS
 * 	and hadnle the responses
 * */
#ifndef GLOBAL_SYSCALL_RUNTIME_H
#define GLOBAL_SYSCALL_RUNTIME_H
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

#include <global_syscall_protocol.h>

//#define GLOBAL_OS_IP "127.0.0.1"
//#define GLOBAL_OS_PORT 0xfeeb
//#define SYSCALL_REQ_FORMAT "id: %d func:%s args1:%d args2:%d args3:%d args4:%d"
//#define SYSCALL_RSP_FORMAT "ret: %d"


int connect_global_OS(int os_port);

int invoke_global_syscall(int global_os_fd, char * syscall);

#endif
