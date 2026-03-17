#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include "mpv_ipc.h"

#define MUSIC_FOLDER_PATH "/home/wanfeng/Music"
#define MAX_MUSIC_COUNT 1024
#define MAX_INPUT_LEN 64
#define MAX_PATH_LEN 4096

static void get_music_list(char *music_list[], int *count);
static void free_music_list(char *music_list[], int count);
static void print_music_list(char *music_list[], int count);
static int play_music(int sockfd, const char *music_file);

int main(void)
{
	char *music_list[MAX_MUSIC_COUNT] = {0};
	int count = 0;
	get_music_list(music_list, &count);
	if (count <= 0) {
		printf("未在 %s 中找到 mp3 文件。\n", MUSIC_FOLDER_PATH);
		return 0;
	}

	int sockfd = mpv_connect();
	if (sockfd < 0) {
		if (mpv_start() < 0) {
			free_music_list(music_list, count);
			return -1;
		}
		sockfd = mpv_connect();
		if (sockfd < 0) {
			perror("connect");
			free_music_list(music_list, count);
			return -1;
		}
	}

	print_music_list(music_list, count);
	printf("请输入歌曲序号: ");
	fflush(stdout);

	char input[MAX_INPUT_LEN] = {0};
	if (fgets(input, sizeof(input), stdin) == NULL) {
		close(sockfd);
		free_music_list(music_list, count);
		return 0;
	}

	char *end = NULL;
	errno = 0;
	long index = strtol(input, &end, 10);
	if (errno != 0 || end == input || index < 1 || index > count) {
		printf("请输入 1 到 %d 之间的序号。\n", count);
		close(sockfd);
		free_music_list(music_list, count);
		return -1;
	}

	if (play_music(sockfd, music_list[index - 1]) == 0) {
		printf("开始播放: %s\n", music_list[index - 1]);
	} else {
		printf("播放失败: %s\n", music_list[index - 1]);
	}

	close(sockfd);
	free_music_list(music_list, count);
	return 0;
}

static int mp3_filter(const struct dirent *entry)
{
	const char *ext = strrchr(entry->d_name, '.');
	return ext && strcasecmp(ext, ".mp3") == 0;
}

static int name_sort(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

static void get_music_list(char *music_list[], int *count)
{
	struct dirent **namelist = NULL;
	*count = scandir(MUSIC_FOLDER_PATH, &namelist, mp3_filter, name_sort);
	if (*count < 0) {
		perror("scandir");
		return;
	}

	for (int i = 0; i < *count; i++) {
		music_list[i] = strdup(namelist[i]->d_name);
		free(namelist[i]);
	}
	free(namelist);
}

static void free_music_list(char *music_list[], int count)
{
	for (int i = 0; i < count; i++) {
		free(music_list[i]);
	}
}

static void print_music_list(char *music_list[], int count)
{
	printf("播放列表:\n");
	for (int i = 0; i < count; i++) {
		printf("%d. %s\n", i + 1, music_list[i]);
	}
}

static int play_music(int sockfd, const char *music_file)
{
	char path[MAX_PATH_LEN] = {0};
	char cmd[MAX_PATH_LEN + 128] = {0};

	snprintf(path, sizeof(path), "%s/%s", MUSIC_FOLDER_PATH, music_file);
	snprintf(cmd, sizeof(cmd),
		 "{\"command\":[\"loadfile\",\"%s\",\"replace\"]}\n", path);

	if (mpv_command(sockfd, cmd) < 0) {
		return -1;
	}

	return 0;
}
