#ifndef SDLAB_HTTP_CMD_H
#define SDLAB_HTTP_CMD_H

extern void cmd_reset(int sock, info_type *info);
extern void cmd_start(int sock, info_type *info);
extern void cmd_stop(int sock, info_type *info);

#endif

