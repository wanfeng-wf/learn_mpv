#ifndef MPV_IPC_H
#define MPV_IPC_H

#include <stddef.h>
#include <cjson/cJSON.h>

#define MPV_SOCKET_PATH "/tmp/mpvsocket"

int mpv_connect(void);
int mpv_start(void);
int mpv_send(int sockfd, const char *cmd);
int mpv_recv_reply_line(int sockfd, char *buf, size_t bufsize);
int mpv_command(int sockfd, const char *cmd);
int mpv_request_property(int sockfd, const char *name, int request_id, cJSON **root);
int mpv_get_double_property(int sockfd, const char *name, int request_id, double *value);
int mpv_get_bool_property(int sockfd, const char *name, int request_id, int *value);

#endif
