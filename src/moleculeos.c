/*
 * MoleculeOS: a global OS design for Molecule-Serverless
 * 	by Dong Du (IPADS @ SJTU)
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if !defined(__FreeBSD__)
#include <malloc.h>
#endif

#include <global_syscall.h>

int main(void)
{
	/* 1. Init global processes */
	global_os_init();


	/* Loop1: wait for syscall events */
	return global_syscall_loop();
//	return 0;
}
