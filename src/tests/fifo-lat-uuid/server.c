#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>
#include <getopt.h>

#include "common/common.h"
#include <molecule-ipc.h>
#include <chos/errno.h>
#include <global_syscall_protocol.h>
#include <global_syscall_interfaces.h>

//#define FIFO_PATH "/tmp/ipc_bench_fifo"

#if 0
void cleanup(FILE* stream, void* buffer) {
	free(buffer);
	fclose(stream);
	if (remove(FIFO_PATH) == -1) {
		throw("Error removing FIFO");
	}
}
#endif

void communicate(int fifo_self,
								 struct Arguments* args,
								 struct sigaction* signal_action) {
	struct Benchmarks bench;
	int message;
	char* buffer;
	int len;
	int fifo_client;
	const char * msg = "By World\n";

	int client_id, func_args1, func_args2, func_args3, func_args4;
	char func_name[16];
	//buffer = malloc(args->size);
	buffer = (char*)malloc(256);
	//setup_benchmarks(&bench);

	//wait_for_signal(signal_action);

	//for (message = 0; message < args->count; ++message) {
	while (true){
		//bench.single_start = now();

		//wait_for_signal(signal_action);

		/* Read Req: should blocking */
		{
			//int msg_len = 0;
			//len = fifo_read(fifo_self, buffer, strlen(buffer));
			len = global_fifo_read(fifo_self, buffer, 256);
			fprintf(stderr, "[Server] Got Request: %s, len (%d)\n", buffer, len);
			sscanf(buffer, FIFO_MSG_FORMAT, &client_id, func_name, &func_args1, &func_args2, &func_args3, &func_args4);
			fprintf(stderr, "[Server] clientid:%d, func:%s\n", client_id, func_name);
		}

		/* Connect to the client */
		/* Note(DD): this maybe one reason to use signal: that we can ensure client'id */
#if 0
		if (signalpid >0)
			fifo_client = fifo_connect(signalpid);
		else{
			fprintf(stderr, "[Server] got error client id:%d\n", signalpid);
			exit(-1);
		}
#else
		fifo_client = fifo_connect(client_id);
#endif
		/* Write Response */
		fifo_write(fifo_client, (char*)msg, sizeof(msg));

		//signal_notify_client(signalpid);
		//fprintf(stderr, "[GlobalOS] signal invoked\n");

		//benchmark(&bench);
		fifo_finish(fifo_client);
	}

	//evaluate(&bench, args);
	//cleanup(stream, buffer);
}

#if 0
FILE* open_fifo() {
	FILE* stream;

	// Just in case it already exists
	//(void)remove(FIFO_PATH);

	// Create a FIFO object. Note that a FIFO is
	// really just a special file, which must be
	// opened by one process and to which then
	// both server and client can write using standard
	// c i/o functions. 0666 specifies read+write
	// access permissions for the user, group and world
	if (mkfifo(FIFO_PATH, 0666) > 0) {
		throw("Error creating FIFO");
	}

	// Tell the client the fifo now exists and
	// can be opened from the read end
	//notify_client();

	// Because a fifo is really just a file, we can
	// open a normal FILE* stream to it (in write mode)
	// Note that this call will block until the read-end
	// is opened by the client
	if ((stream = fopen(FIFO_PATH, "w")) == NULL) {
		throw("Error opening descriptor to FIFO on server side");
	}

	return stream;
}
#endif

int main(int argc, char* argv[]) {
	// The file pointer we will associate with the FIFO
	struct sigaction signal_action;
	int fifo_self;
	int fifo_server;
	int global_fifo;
	int ret;
	char* buffer = (char*)malloc(256);
#define MAX_TEST_BUF_SIZE 2048
	char test_buf[MAX_TEST_BUF_SIZE];

	//struct Arguments args;
	//parse_arguments(&args, argc, argv);

	//setup_server_signals(&signal_action);
	//signal_register_server_handler();

	register_self_global(GLOBAL_OS_PORT); //server always use the default globalOS

	fifo_self = fifo_init();
	//Here, the getpid is the uuid used in local fifo
	global_fifo = global_fifo_init_uuid(getpid(), 0x1beef);

	fprintf(stderr, "Server global fifo:%d\n", global_fifo);

	//Let the server run forever
	while (true){
		ret = global_fifo_read(global_fifo, test_buf, MAX_TEST_BUF_SIZE);

		if (ret == -EFIFOLOCAL){
		//	fifo_read(fifo_self);
			ret = fifo_read(fifo_self, test_buf, MAX_TEST_BUF_SIZE);
		}
	}

#if 0

	//communicate(fifo_self, NULL, &signal_action);
	communicate(global_fifo, NULL, &signal_action);

	fifo_finish(fifo_self);
	fifo_clean();
#endif

	return EXIT_SUCCESS;
}
