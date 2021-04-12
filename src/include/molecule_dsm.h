#ifndef MOLECULE_IPC_H
#define MOLECULE_IPC_H


int molecule_dsm_init(char *client_target_name, int pu_id);
int dsm_call(char* buf, int len);
int dsm_handlers(char* buf, int len);

#endif
