#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/resource.h>
#include <shm_mpsc_queue.h>
#include <shmmap.h>

using namespace std;

struct syscallEntry
{
	char buf[256];
	int len;
	int valid;
};
typedef SHMMPSCQueue<syscallEntry, 4000> GSyscallQueue;


int gsyscall_server_main(int argc, char* argv[]);

extern "C" void sayhello(char *str);
void sayhello(char *str)
{
	cout << "hello"<<str<<endl;
}

GSyscallQueue* g_logq = nullptr;
bool g_delay_format_ts = true;

volatile bool running = true;

void gSyscallServer(void) {
    while(running) {
        GSyscallQueue::Entry* list = g_logq->PopAll();
        if(!list) {
            this_thread::yield();
            continue;
        }
        auto cur = list;
        while(true) {
#if 0
            if(g_delay_format_ts) {
                cur->data.formatTime();
            }
#endif
	    assert(cur->data.len<256);
	    cur->data.buf[cur->data.len] = '\0';
	    fprintf(stderr, "[GSyscallServer] got req: %s\n", cur->data.buf);

            //file.append(cur->data.buf, cur->data.buflen);
            GSyscallQueue::Entry* next = g_logq->NextEntry(cur);
            if(!next) break;
            cur = next;
        }
        g_logq->Recycle(list, cur);
        //file.flush();
    }
}

int gsyscall_server_main(int argc, char* argv[]) {
    // cpupin(2);
    g_logq = shmmap<GSyscallQueue>("/GSyscallQueue.shm");
    if(!g_logq) return 1;
    {
        // set max virtual memory to 2GB.
        size_t kOneGB = 1000 * 1024 * 1024;
        rlimit rl = {2 * kOneGB, 2 * kOneGB};
        setrlimit(RLIMIT_AS, &rl);
    }
    printf("pid = %d\n", getpid());
    char name[256] = {0};
    strncpy(name, argv[0], sizeof name - 1);
    gSyscallServer();
}
