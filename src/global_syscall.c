#include<stdio.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>

#define MAXLEN 1024
#define SERV_PORT 0xfeeb //The reverse of 0xbeef
#define MAX_OPEN_FD 1024


int global_syscall_loop(void)
{
    int  listenfd,connfd,efd,ret;
    char buf[MAXLEN];
    struct sockaddr_in cliaddr,servaddr;
    socklen_t clilen = sizeof(cliaddr);
    struct epoll_event tep,ep[MAX_OPEN_FD];
    char * unsupported = "Syscall unsupported";

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
		    fprintf(stderr, "[%s] Global syscall invoked: %s\n", __func__, buf);
		    write(connfd, unsupported, strlen(unsupported));
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

#define DEBUG 0
#if DEBUG //for debug
int main(void)
{
	return global_syscall_loop();
}
#endif
