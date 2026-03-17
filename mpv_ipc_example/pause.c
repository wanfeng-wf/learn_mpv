#include <unistd.h>
#include <stdio.h>
#include "mpv_ipc.h"

int main(void)
{
	int sockfd = mpv_connect();
	if (sockfd < 0) {
		perror("connect");
		return -1;
	}

	const char *cmd = "{\"command\":[\"cycle\",\"pause\"]}\n";
	if (mpv_command(sockfd, cmd) < 0) {
		close(sockfd);
		return -1;
	}

	close(sockfd);
	return 0;
}
