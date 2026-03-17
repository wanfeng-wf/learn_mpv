#include "mpv_ipc.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int mpv_connect(void)
{
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		return -1;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, MPV_SOCKET_PATH);

	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}

int mpv_start(void)
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		execlp("mpv", "mpv", "--idle=yes", "--no-terminal",
		       "--input-ipc-server=" MPV_SOCKET_PATH, (char *)NULL);
		perror("execlp");
		_exit(1);
	}

	sleep(1);
	return 0;
}

int mpv_send(int sockfd, const char *cmd)
{
	if (write(sockfd, cmd, strlen(cmd)) < 0) {
		perror("write");
		return -1;
	}

	return 0;
}

int mpv_recv_reply_line(int sockfd, char *buf, size_t bufsize)
{
	size_t pos = 0;

	while (pos < bufsize - 1) {
		char ch;
		int n = read(sockfd, &ch, 1);
		if (n < 0) {
			perror("read");
			return -1;
		}
		if (n == 0) {
			break;
		}

		buf[pos++] = ch;
		if (ch == '\n') {
			break;
		}
	}

	buf[pos] = '\0';
	return (int)pos;
}

int mpv_command(int sockfd, const char *cmd)
{
	char reply[512] = {0};

	if (mpv_send(sockfd, cmd) < 0) {
		return -1;
	}

	return mpv_recv_reply_line(sockfd, reply, sizeof(reply)) > 0 ? 0 : -1;
}

int mpv_request_property(int sockfd, const char *name, int request_id, cJSON **root)
{
	char cmd[128] = {0};
	char reply[512] = {0};

	snprintf(cmd, sizeof(cmd),
		 "{\"command\":[\"get_property\",\"%s\"],\"request_id\":%d}\n",
		 name, request_id);

	if (mpv_send(sockfd, cmd) < 0) {
		return -1;
	}

	while (mpv_recv_reply_line(sockfd, reply, sizeof(reply)) > 0) {
		*root = cJSON_Parse(reply);
		if (*root == NULL) {
			continue;
		}

		cJSON *id = cJSON_GetObjectItem(*root, "request_id");
		if (cJSON_IsNumber(id) && id->valueint == request_id) {
			return 0;
		}

		cJSON_Delete(*root);
		*root = NULL;
	}

	return -1;
}

int mpv_get_double_property(int sockfd, const char *name, int request_id, double *value)
{
	cJSON *root = NULL;
	if (mpv_request_property(sockfd, name, request_id, &root) < 0) {
		return -1;
	}

	int ok = -1;
	cJSON *data = cJSON_GetObjectItem(root, "data");
	if (cJSON_IsNumber(data)) {
		*value = data->valuedouble;
		ok = 0;
	} else if (cJSON_IsNull(data)) {
		ok = 1;
	}

	cJSON_Delete(root);
	return ok;
}

int mpv_get_bool_property(int sockfd, const char *name, int request_id, int *value)
{
	cJSON *root = NULL;
	if (mpv_request_property(sockfd, name, request_id, &root) < 0) {
		return -1;
	}

	int ok = -1;
	cJSON *data = cJSON_GetObjectItem(root, "data");
	if (cJSON_IsBool(data)) {
		*value = cJSON_IsTrue(data);
		ok = 0;
	}

	cJSON_Delete(root);
	return ok;
}
