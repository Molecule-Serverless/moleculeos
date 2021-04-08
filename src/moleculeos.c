/*
 * MoleculeOS: a global OS design for Molecule-Serverless
 * 	by Dong Du (IPADS @ SJTU)
 * */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if !defined(__FreeBSD__)
#include <malloc.h>
#endif

#include <global_syscall.h>

typedef struct Local_Arguments {
	int pu_id; //the pu_id of this pu
	int os_port; //the port of this global OS
} Local_Arguments;

void local_parse_arguments(Local_Arguments *arguments, int argc, char *argv[]) {
	// For getopt long options
	int long_index = 0;
	// For getopt chars
	int opt;

	// Reset the option index to 1 if it
	// was modified before (e.g. in check_flag)
	optind = 0;

	// Default values
	arguments->pu_id = -1;
	arguments->os_port = GLOBAL_OS_PORT; //default one

	// Command line arguments
	// clang-format off
	static struct option long_options[] = {
			{"pu id",  required_argument, NULL, 'i'},
			{"os port", required_argument, NULL, 'p'},
			{0,       0,                 0,     0}
	};
	while (1) {
		opt = getopt_long(argc, argv, "+:i:p:", long_options, &long_index);

		switch (opt) {
			case -1: return;
			case 'i': arguments->pu_id = atoi(optarg); break;
			case 'p': arguments->os_port = atoi(optarg); break;
			default: continue;
		}
	}
}


int main(int argc, char *argv[])
{
	struct Local_Arguments args;
	local_parse_arguments(&args, argc, argv);

	/* 1. Init global processes */
	global_os_init(args.pu_id, args.os_port);


	/* Loop1: wait for syscall events */
	return global_syscall_loop();
//	return 0;
}
