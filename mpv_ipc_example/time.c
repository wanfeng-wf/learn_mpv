#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "mpv_ipc.h"

static void format_time(double value, char *buf, size_t size);

int main(void)
{
	signal(SIGPIPE, SIG_IGN);

	int sockfd = mpv_connect();
	if (sockfd < 0) {
		perror("connect");
		return -1;
	}

	double last_time = 0;
	double last_duration = 0;

	while (1) {
		double time_pos = -1;
		double duration = -1;
		int idle = 0;
		int paused = 0;

		if (mpv_get_bool_property(sockfd, "idle-active", 1, &idle) < 0) {
			break;
		}
		if (idle) {
			break;
		}

		if (mpv_get_bool_property(sockfd, "pause", 2, &paused) < 0) {
			break;
		}

		int time_rc = mpv_get_double_property(sockfd, "time-pos", 3, &time_pos);
		int duration_rc = mpv_get_double_property(sockfd, "duration", 4, &duration);
		if (time_rc < 0 || duration_rc < 0) {
			break;
		}

		if (duration_rc == 1) {
			if (last_duration <= 0) {
				break;
			}
			duration = last_duration;
		}

		if (time_rc == 1) {
			if (!paused) {
				break;
			}
			time_pos = last_time;
		}

		if (time_pos < 0 || duration <= 0) {
			break;
		}

		last_time = time_pos;
		last_duration = duration;

		char current[16] = {0};
		char total[16] = {0};
		format_time(time_pos, current, sizeof(current));
		format_time(duration, total, sizeof(total));
		printf("\r%s / %s%s", current, total, paused ? " [pause]" : "        ");
		fflush(stdout);

		usleep(200000);
	}

	printf("\n");
	close(sockfd);
	return 0;
}

static void format_time(double value, char *buf, size_t size)
{
	int total = (int)value;
	snprintf(buf, size, "%02d:%02d", total / 60, total % 60);
}
