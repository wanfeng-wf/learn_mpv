#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "mpv_ipc.h"

int main(void)
{
	int sockfd = mpv_connect();
	if (sockfd >= 0) {
		const char *cmd = "{\"command\":[\"quit\"]}\n";
		if (mpv_command(sockfd, cmd) < 0) {
			close(sockfd);
			return -1;
		}

		close(sockfd);
	}

	if (unlink(MPV_SOCKET_PATH) < 0 && errno != ENOENT) {
		perror("unlink");
		return -1;
	}

	return 0;
}
