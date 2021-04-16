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

#include <pthread.h>

#ifdef SMARTC
#include <molecule_dsm.h>
#endif
#include <global_syscall.h>

typedef struct Local_Arguments {
	int pu_id; //the pu_id of this pu
	int os_port; //the port of this global OS
#ifdef SMARTC
	char dsm_master_addr[48]; //the addr of dsm master
#endif
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
#ifdef SMARTC
	memset(arguments->dsm_master_addr, '\0', 48);
	memcpy(arguments->dsm_master_addr, "127.0.0.1", 9) ; //default one
	//fprintf(stderr, "debug: %s\n",arguments->dsm_master_addr);
#endif

	// Command line arguments
	// clang-format off
	static struct option long_options[] = {
			{"pu id",  required_argument, NULL, 'i'},
			{"os port", required_argument, NULL, 'p'},
#ifdef SMARTC
			{"dsm master addr", required_argument, NULL, 'd'},
#endif
			{0,       0,                 0,     0}
	};
	while (1) {
#ifdef SMARTC
		opt = getopt_long(argc, argv, "+:i:p:d:", long_options, &long_index);
#else
		opt = getopt_long(argc, argv, "+:i:p:", long_options, &long_index);
#endif

		switch (opt) {
			case -1: return;
			case 'i': arguments->pu_id = atoi(optarg); break;
			case 'p': arguments->os_port = atoi(optarg); break;
#ifdef SMARTC
			case 'd': memcpy(arguments->dsm_master_addr, optarg, strlen(optarg)); break;
#endif
			default: continue;
		}
	}
#ifdef SMARTC
	fprintf(stderr, "debug: %s\n",arguments->dsm_master_addr);
#endif
}

#ifdef SMARTC
/* 
 * The entrypoint of the globalOS DSM layer, which handles requests from remote globalOS
 * */

char dsm_master_addr[48]; //the addr of dsm master
#if 1
void * globalOS_DSM(void)
{
	int pu_id = get_current_pu_id();
        printf("[MoleculeOS] DSM layer started\n");
	if (pu_id == 0){
		// This is the main globalOS (working as server)
        	fprintf(stderr, "[MoleculeOS] I am master\n");
		//pu_id not used
		//molecule_dsm_init(NULL, 0);
		uct_molecule_dsm_init(NULL, 0, "enp125s0f0", "tcp");
	}
	else{
		//FIXME: how should we know the addr of server? 
		//molecule_dsm_init("127.0.0.1");
        	fprintf(stderr, "[MoleculeOS] Master at %s\n", dsm_master_addr);
		//pu_id not used
		//molecule_dsm_init(dsm_master_addr, 0);
		uct_molecule_dsm_init(dsm_master_addr, 0, "enp125s0f0", "tcp");
	}

	while (1){
	    sleep(1);
	}
}
#else
void * globalOS_DSM_server(void)
{
	int pu_id = get_current_pu_id();
        printf("[MoleculeOS] DSM layer started\n");
	// This is the main globalOS (working as server)
       	fprintf(stderr, "[MoleculeOS] I am master\n");

	/*Note: pu_id here is the current's pu_id */
	molecule_dsm_init(NULL, pu_id);

	while (1){
	    sleep(1);
	}
}
void * globalOS_DSM_client(void)
{
	int pu_id = get_current_pu_id();
        printf("[MoleculeOS] DSM layer started\n");

	//molecule_dsm_init("127.0.0.1");
        fprintf(stderr, "[MoleculeOS] Master at %s\n", dsm_master_addr);

	/* Note: 1 - pu_id is the remote's pu_id 
	 * FIXME: this imp can not support >2 PUs!
	 * */
	molecule_dsm_init(dsm_master_addr, 1 - pu_id);

	while (1){
	    sleep(1);
	}
}
#endif //if-1
#endif //SMARTC

int main(int argc, char *argv[])
{
	struct Local_Arguments args;
#ifdef SMARTC
	pthread_t DSM_thread_server;
	//pthread_t DSM_thread_client;
	int ret;
#endif

	local_parse_arguments(&args, argc, argv);

	/* 1. Init global processes */
	global_os_init(args.pu_id, args.os_port);

#ifdef SMARTC
	memset(dsm_master_addr, '\0', 48);
	memcpy(dsm_master_addr, args.dsm_master_addr, strlen(args.dsm_master_addr));

	//fprintf(stderr, "debug: %s\n", args.dsm_master_addr);
	//ret = pthread_create(&DSM_thread_server, NULL, (void*)globalOS_DSM_server, NULL);
	//ret = pthread_create(&DSM_thread_client, NULL, (void*)globalOS_DSM_client, NULL);
	ret = pthread_create(&DSM_thread_server, NULL, (void*)globalOS_DSM, NULL);
	if (ret){
	        fprintf(stderr, "Create pthread error!/n");
		return 1;
	}

#endif

	/* Loop1: wait for syscall events */
	global_syscall_loop();

#ifdef SMARTC
	//pthread_join(DSM_thread_server, NULL);
	//pthread_join(DSM_thread_client, NULL);
	pthread_join(DSM_thread_server, NULL);
#endif

	return 0;
}
