#ifndef MOLECULE_IPC_H
#define MOLECULE_IPC_H


/* UCP version of Imps. */
int molecule_dsm_init(char *client_target_name, int pu_id);

/* UCT version of Imps. */
int uct_molecule_dsm_init(char* client_target_name, int pu_id, char* dev_name, char* tl_name);


/* Common functions */
int dsm_call(char* buf, int len);
int dsm_handlers(char* buf, int len);


#endif
