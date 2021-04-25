#include <bits/stdc++.h>
//#include "Logging.h"
#include "AppendFile.h"
#include "cpupin.h"
#include "shmmap.h"
#include <unistd.h>
#include <sys/resource.h>
#include "timestamp.h"
using namespace std;

#include "../shm_mpsc_queue.h"
struct syscallEntry
{
	char buf[256];
	int len;
	int valid;
};
typedef SHMMPSCQueue<syscallEntry, 4000> GSyscallQueue;

GSyscallQueue* g_logq = nullptr;
//LogQueue* g_logq = nullptr;
bool g_delay_format_ts = true;

void bench(bool longLog) {
    int cnt = 0;
    const int kBatch = 10;
    const char* test_str ="SYSCALL:TEST, args:0, args:0, args:0 ";
    string empty = " ";
    string longStr(3000, 'X');
    GSyscallQueue::Entry* entry_ = nullptr;

    longStr += " ";

    for(int t = 0; t < 30; ++t) {
        int64_t start = now();
#if 0
        for(int i = 0; i < kBatch; ++i) {
            LOG_INFO << "Hello 0123456789"
                     << " abcdefghijklmnopqrstuvwxyz " << (longLog ? longStr : empty) << cnt;
            ++cnt;
        }
#endif
    	while((entry_ = g_logq->Alloc()) == nullptr);
    	memcpy(entry_->data.buf, test_str, strlen(test_str));
	entry_->data.len = strlen(test_str);
  	g_logq->Push(entry_);
        int64_t end = now();
        printf("time: %d us, %f\n", end-start,  static_cast<double>(end - start) / kBatch);
        struct timespec ts = {0, 500 * 1000 * 1000};
        nanosleep(&ts, NULL);
    }
}

int main(int argc, char* argv[]) {
    // cpupin(1);
    //g_logq = shmmap<LogQueue>("/LogQueue.shm");
    g_logq = shmmap<GSyscallQueue>("/GSyscallQueue.shm");
    if(!g_logq) return 1;

    {
        // set max virtual memory to 2GB.
        size_t kOneGB = 1000 * 1024 * 1024;
        rlimit rl = {2 * kOneGB, 2 * kOneGB};
        setrlimit(RLIMIT_AS, &rl);
    }
    printf("pid = %d\n", getpid());

    bool longLog = argc > 1;
    bench(longLog);
}



