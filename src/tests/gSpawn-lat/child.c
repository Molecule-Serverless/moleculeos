#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>
#include <getopt.h>

#if 0
#include "common/common.h"
#include <molecule-ipc.h>
#include <chos/errno.h>
#include <global_syscall_protocol.h>
#include <global_syscall_interfaces.h>
#endif

int main(int argc, char* argv[]) {
	fprintf(stderr, "Hello, I am child in gSpawn\n");

	return EXIT_SUCCESS;
}
