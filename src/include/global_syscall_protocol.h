/*
 * Molecule Global Syscall PROTOCOL
 * 	Basic neogitaion between processes and globalOS
 * */
#ifndef GLOBAL_SYSCALL_PROTOCOL_H
#define GLOBAL_SYSCALL_PROTOCOL_H

#define GLOBAL_OS_IP "127.0.0.1"
#define GLOBAL_OS_PORT 0xfeeb
#define SYSCALL_REQ_FORMAT "id: %d func:%s args1:%d args2:%d args3:%d args4:%d "
#define SYSCALL_RSP_FORMAT "ret: %d "

#define DSM_REQ_FORMAT "gpid: %d func:%s args1:%d args2:%d args3:%d args4:%d buf_len:%d "
#define DSM_REQ_FORMAT_CALLEE "gpid: %d func:%s args1:%d args2:%d args3:%d args4:%d buf_len:%d %n"
#define DSM_RSP_FORMAT "ret: %d "

#endif
