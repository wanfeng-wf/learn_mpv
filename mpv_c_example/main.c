#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <mpv/client.h>

#define MUSIC_FOLDER_PATH "/home/wanfeng/Music"
#define MAX_MUSIC_COUNT 1024
#define MAX_INPUT_LEN 128
#define MAX_PATH_LEN 4096

static void get_music_list(char *music_list[], int *count);
static void free_music_list(char *music_list[], int count);
static void print_music_list(char *music_list[], int count);
static int play_music(mpv_handle *ctx, const char *music_file);
static int toggle_pause(mpv_handle *ctx);
static int show_time(mpv_handle *ctx);

int main(void)
{
	char *music_list[MAX_MUSIC_COUNT] = {0};
	int count = 0;
	get_music_list(music_list, &count);
	if (count <= 0) {
		printf("未在 %s 中找到 mp3 文件。\n", MUSIC_FOLDER_PATH);
		return 0;
	}

	mpv_handle *ctx = mpv_create();
	if (ctx == NULL) {
		fprintf(stderr, "mpv_create failed\n");
		free_music_list(music_list, count);
		return -1;
	}

	if (mpv_initialize(ctx) < 0) {
		fprintf(stderr, "mpv_initialize failed\n");
		mpv_terminate_destroy(ctx);
		free_music_list(music_list, count);
		return -1;
	}

	print_music_list(music_list, count);
	printf("命令: play 序号 | pause | time | list | quit\n");

	char input[MAX_INPUT_LEN] = {0};
	while (1) {
		printf("> ");
		fflush(stdout);

		if (fgets(input, sizeof(input), stdin) == NULL) {
			break;
		}

		if (strncmp(input, "quit", 4) == 0) {
			break;
		}

		if (strncmp(input, "list", 4) == 0) {
			print_music_list(music_list, count);
			continue;
		}

		if (strncmp(input, "pause", 5) == 0) {
			if (toggle_pause(ctx) < 0) {
				printf("暂停/继续失败。\n");
			}
			continue;
		}

		if (strncmp(input, "time", 4) == 0) {
			if (show_time(ctx) < 0) {
				printf("当前没有正在播放的歌曲。\n");
			}
			continue;
		}

		if (strncmp(input, "play", 4) == 0) {
			char *arg = input + 4;
			while (*arg == ' ' || *arg == '\t') {
				arg++;
			}

			char *end = NULL;
			errno = 0;
			long index = strtol(arg, &end, 10);
			if (errno != 0 || end == arg || index < 1 || index > count) {
				printf("请输入正确的序号，例如: play 1\n");
				continue;
			}

			if (play_music(ctx, music_list[index - 1]) == 0) {
				printf("开始播放: %s\n", music_list[index - 1]);
			} else {
				printf("播放失败: %s\n", music_list[index - 1]);
			}
			continue;
		}

		printf("未知命令。可用命令: play 序号 | pause | time | list | quit\n");
	}

	mpv_terminate_destroy(ctx);
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

static int play_music(mpv_handle *ctx, const char *music_file)
{
	char path[MAX_PATH_LEN] = {0};
	snprintf(path, sizeof(path), "%s/%s", MUSIC_FOLDER_PATH, music_file);

	const char *cmd[] = {"loadfile", path, "replace", NULL};
	return mpv_command(ctx, cmd);
}

static int toggle_pause(mpv_handle *ctx)
{
	int pause = 0;
	if (mpv_get_property(ctx, "pause", MPV_FORMAT_FLAG, &pause) < 0) {
		return -1;
	}

	pause = !pause;
	return mpv_set_property(ctx, "pause", MPV_FORMAT_FLAG, &pause);
}

static void format_time(double value, char *buf, size_t size)
{
	int total = (int)value;
	snprintf(buf, size, "%02d:%02d", total / 60, total % 60);
}

static int show_time(mpv_handle *ctx)
{
	double time_pos = 0.0;
	double duration = 0.0;

	if (mpv_get_property(ctx, "time-pos", MPV_FORMAT_DOUBLE, &time_pos) < 0 ||
	    mpv_get_property(ctx, "duration", MPV_FORMAT_DOUBLE, &duration) < 0) {
		return -1;
	}

	char time_pos_str[10] = {0};
	char duration_str[10] = {0};
	format_time(time_pos, time_pos_str, sizeof(time_pos_str));
	format_time(duration, duration_str, sizeof(duration_str));
	printf("%s / %s\n", time_pos_str, duration_str);
	return 0;
}
