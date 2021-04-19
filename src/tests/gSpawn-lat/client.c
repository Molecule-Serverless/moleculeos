#if 0
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "common/common.h"
#include <molecule-ipc.h>
#include <global_syscall_interfaces.h>
#include <global_syscall_protocol.h>

typedef struct Local_Arguments {
	int size;
	int count;
	int server_id;
	int os_port;
} Local_Arguments;


void cleanup(FILE *stream, void *buffer) {
	free(buffer);
	fclose(stream);
}

//void communicate(FILE *stream,
//								 struct Arguments *args,
//								 struct sigaction *signal_action) {
void communicate(int fifo_self, int fifo_server,  struct Local_Arguments *args,
		struct sigaction *signal_action) {
	//void *buffer = malloc(args->size);
	char *buffer = (char*)malloc(256);
	const char* msg = "Echo: Hello World\n";
	int len;
	//memcpy(buffer, "Echo: Hello World\n");

	//#define FIFO_MSG_FORMAT "id: %d func:%s args1:%d args2:%d args3:%d args4:%d"
	sprintf(buffer, FIFO_MSG_FORMAT, getpid(), "Echo", 0, 0, 0, 0);


	// Server can go
	//notify_server();
	//notify_globalOS(args->globalOS);

#if 0
	for (; args->count > 0; --args->count) {
		//wait_for_signal(signal_action);

		if (fread(buffer, args->size, 1, stream) == 0) {
			throw("Error reading buffer");
		}

		//notify_server();
		notify_globalOS(args->globalOS);
	}
#endif
#if 0
	if (fread(buffer, args->size, 1, stream) == 0) {
		throw("Error reading buffer");
	}else{
		fprintf(stderr, "[Runtime] Got msg from GlobalOS:%s\n", (char*)buffer);
	}
#endif
	fprintf(stderr, "[Client] Send Req: %s\n", msg);
	//fifo_write(fifo_server, (char*)msg, strlen(msg));
	fifo_write(fifo_server, (char*)buffer, strlen(buffer));
	//signal_notify_server(args->server_id);
	//notify_globalOS(args->globalOS);

	//Wait for complection
	//wait_for_signal(signal_action);
	len = fifo_read(fifo_self, buffer, 256);
	if (len >0){
		fprintf(stderr, "[Client] Got Response: %s\n", buffer);
	}

	//cleanup(stream, buffer);
}

#if 0
FILE *open_fifo(struct sigaction *signal_action) {
	FILE *stream;

	// Wait for the server to create the FIFO
	//wait_for_signal(signal_action);

	// Because a FIFO is really just a file, we can
	// open a normal FILE* stream to it (in read mode)
	// Note that this call will block until the write-end
	// is opened by the server
	if ((stream = fopen(FIFO_PATH, "r")) == NULL) {
		throw("Error opening stream to FIFO on client-side");
	}

	return stream;
}
#endif

void local_parse_arguments(Local_Arguments *arguments, int argc, char *argv[]) {
	// For getopt long options
	int long_index = 0;
	// For getopt chars
	int opt;

	// Reset the option index to 1 if it
	// was modified before (e.g. in check_flag)
	optind = 0;

	// Default values
	arguments->size = DEFAULT_MESSAGE_SIZE;
	arguments->count = 1000;
	arguments->os_port = GLOBAL_OS_PORT;

	// Command line arguments
	// clang-format off
	static struct option long_options[] = {
		{"size",  required_argument, NULL, 's'},
		{"count", required_argument, NULL, 'c'},
		{"server Id", required_argument, NULL, 'i'},
		{"global os port", required_argument, NULL, 'p'},
		{0,       0,                 0,     0}
	};
	// clang-format on

	while (true) {
		opt = getopt_long(argc, argv, "+:s:c:i:p:", long_options, &long_index);

		switch (opt) {
			case -1: return;
			case 's': arguments->size = atoi(optarg); break;
			case 'c': arguments->count = atoi(optarg); break;
			case 'i': arguments->server_id = atoi(optarg); break;
			case 'p': arguments->os_port = atoi(optarg); break;
			default: continue;
		}
	}
}

int main(int argc, char *argv[]) {
	// The file pointer we will associate with the FIFO
	//FILE *stream;
	// For server/client signals
	struct sigaction signal_action;
	int fifo_self;
	int fifo_server;
	int global_fifo;
	int i;
	int size, count;
#define MAX_TEST_BUF_SIZE 2048
	char test_buf[MAX_TEST_BUF_SIZE];
	struct Benchmarks bench;

	struct Arguments tmp_args;
	struct Local_Arguments args;
	local_parse_arguments(&args, argc, argv);

	register_self_global(args.os_port);

	fprintf(stderr, "[Client] Before fifo_init\n");
	fifo_self = fifo_init();

	//Here, the getpid is the uuid used in local fifo
	global_fifo = global_fifo_init(getpid());

	/*FIXME: connect is ignored now */
	//fifo_server = fifo_connect(args.server_id);

	if (args.size<=0)
		size = 64; //default size
	else
		size = args.size;

	assert(size<=MAX_TEST_BUF_SIZE);
	memset(test_buf, 'd', MAX_TEST_BUF_SIZE);

	if (args.count<10)
		count = 10; //default count
	else
		count = args.count;

	setup_benchmarks(&bench);

	//warmup
	for (i=0;i<10; i++){
		global_fifo_write(args.server_id, test_buf, size);
	}

	//tests
	for (i=0; i<count; i++)
	{
		bench.single_start = now();
		fifo_server = global_fifo_write(args.server_id, test_buf, size);
		benchmark(&bench);
	}
	tmp_args.size = size;
	tmp_args.count = count;
	evaluate(&bench, &tmp_args);

#if 0
	fprintf(stderr, "[Client] Ready to communicate with server:%d\n", args.server_id);
	communicate(fifo_self, fifo_server, &args, &signal_action);

	fifo_finish(fifo_server);
	fifo_finish(fifo_self);
	fifo_clean();
#endif

	return EXIT_SUCCESS;
}
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

extern char **environ;

void run_cmd(char *cmd)
{
	pid_t pid;
	char *argv[] = {cmd, NULL};
	int status;
	printf("Start child: %s\n", cmd);
	status = posix_spawn(&pid, cmd, NULL, NULL, argv, environ);
	if (status == 0) {
		printf("Child pid: %i\n", pid);
		if (waitpid(pid, &status, 0) != -1) {
			printf("Child exited with status %i\n", status);
		} else {
			perror("waitpid");
		}
	} else {
		printf("posix_spawn: %s\n", strerror(status));
	}
}

int main(int argc, char* argv[])
{
	run_cmd(argv[1]);
	return 0;
}
