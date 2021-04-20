/*
 * Molecule Global Syscall Runtime
 * 	This is runtime provides interfaces to send commands to globalOS
 * 	and hadnle the responses
 * */
#ifndef GLOBAL_SYSCALL_RUNTIME_H
#define GLOBAL_SYSCALL_RUNTIME_H

//#define SYSCALL_MSG_FORMAT "id: %d func:%s args1:%d args2:%d args3:%d args4:%d"

/* GP Ops */
int register_self_global(int os_port); //return a global pid
int unregister_self_global(void); //return a status code

/* gFIFO Ops */
int global_fifo_init(int local_fifo); //return a global fifo_fd
int global_fifo_close(int global_fifo); //close a global fifo_fd
int global_fifo_read(int global_fifo, char*buf, int len); //read from a global fifo_fd
int global_fifo_write(int global_fifo, char*buf, int len); //write to a global fifo_fd
int global_fifo_init_uuid(int local_fifo, int gloabl_uuid); //return a global fifo_fd
int global_fifo_connect(int gloabl_uuid); //return a global fifo_fd

/* Permission containers Ops */
int global_grant_perm(int global_pid, int global_fd, int perm); //grant perm of a fifo to another process

/* global Spawn */
int global_spawn(int pu_id,
		int *global_pid,
		const char *restrict path,
		char *const argv[restrict],
		char* const envp[restrict]); //We do not allow file_actions and attrp in gspawn

//int posix_spawn(int * global_pid, const char *restrict path,
//		const posix_spawn_file_actions_t *file_actions,
//		const posix_spawnattr_t *restrict attrp,
//		char *const argv[restrict], char *const envp[restrict]);

#endif
