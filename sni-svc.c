#include "arg.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TAI_OFFSET 4611686018427387914ULL
#define TIMEOUT    5

static time_t read_change_time(const char *path) {
	char file[512];
	snprintf(file, sizeof(file), "%s/supervise/status", path);
	int fd = open(file, O_RDONLY);
	if (fd == -1)
		return 0;

	unsigned char buf[20];
	if (read(fd, buf, sizeof(buf)) != sizeof(buf)) {
		close(fd);
		return 0;
	}
	close(fd);

	time_t changed = ((uint64_t) buf[0] << 56) | ((uint64_t) buf[1] << 48) | ((uint64_t) buf[2] << 40) |
	                 ((uint64_t) buf[3] << 32) | ((uint64_t) buf[4] << 24) | ((uint64_t) buf[5] << 16) |
	                 ((uint64_t) buf[6] << 8) | ((uint64_t) buf[7]);

	return changed;
}

static int is_down(const char *path) {
	char file[1024];
	snprintf(file, sizeof(file), "%s/supervise/status", path);
	int fd = open(file, O_RDONLY);
	if (fd == -1)
		return 1;

	unsigned char buf[20];
	if (read(fd, buf, sizeof(buf)) != sizeof(buf)) {
		close(fd);
		return 1;
	}
	close(fd);
	return buf[19] == 0;    // runit flag: 0 means down
}

void print_status(const char *path) {
	char file[512];
	snprintf(file, sizeof(file), "%s/supervise/status", path);
	int fd = open(file, O_RDONLY);
	if (fd == -1) {
		printf("%s: no supervise/status\n", path);
		return;
	}

	unsigned char buf[20];
	if (read(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printf("%s: invalid status\n", path);
		close(fd);
		return;
	}
	close(fd);

	time_t changed = ((uint64_t) buf[0] << 56) | ((uint64_t) buf[1] << 48) | ((uint64_t) buf[2] << 40) |
	                 ((uint64_t) buf[3] << 32) | ((uint64_t) buf[4] << 24) | ((uint64_t) buf[5] << 16) |
	                 ((uint64_t) buf[6] << 8) | ((uint64_t) buf[7]);

	uint64_t    since     = time(NULL) - changed + TAI_OFFSET;
	const char *sinceunit = "s";
	if (since >= 60) {
		since /= 60;
		sinceunit = " min";
		if (since >= 60) {
			since /= 60;
			sinceunit = " hours";
			if (since >= 24) {
				since /= 24;
				sinceunit = " days";
			}
		}
	}


	int  pid      = (buf[15] << 24) | (buf[14] << 16) | (buf[13] << 8) | buf[12];
	int  paused   = buf[16];
	char want     = buf[17];
	int  signaled = buf[18];
	int  state    = buf[19];

	const char *name = strrchr(path, '/');
	if (name == NULL)
		name = path;
	else
		name++;

	printf("%s: ", name);
	if (state == 0) {
		printf("down");
		if (want == 'u') {
			printf(", but wants up");
		}
		if (signaled) {
			printf(", terminated");
		}
	} else {
		printf("%s as %d", state == 1 ? "run" : "finish", pid);
		if (want == 'd') {
			printf(", but wants down");
		}
		if (paused) {
			printf(", paused");
		}
	}
	printf(": since %lu%s\n", since, sinceunit);
}

void usage(int code) {
	fprintf(stderr, "sni-svc [-d <svdir>] [-w] [-r] [<service> [<command>]]\n");
	exit(code);
}

int main(int argc, char *argv[]) {
	int         termfirst = 0, waitchange = 0;
	const char *svdir = getenv("SNIDIR");
	if (svdir == NULL)
		svdir = getenv("SVDIR");
	if (svdir == NULL)
		svdir = ".";

	ARGBEGIN
	switch (OPT) {
		case 'h':
			usage(0);
			break;
		case 'w':
			waitchange = 1;
			break;
		case 'r':
			termfirst = 1;
			break;
		case 'd':
			svdir = EARGF(usage(1));
			break;
		default:
			fprintf(stderr, "error: unknown option -%c.\n", OPT);
			usage(1);
	}
	ARGEND

	char path[512];
	if (argc == 0) {
		DIR *d = opendir(svdir);
		if (!d) {
			perror("opendir");
			exit(1);
		}
		struct dirent *ent;
		while ((ent = readdir(d))) {
			if (ent->d_type != DT_DIR || ent->d_name[0] == '.')
				continue;

			snprintf(path, sizeof(path), "%s/%s/supervise", svdir, ent->d_name);
			if (access(path, R_OK) != 0)
				continue;

			snprintf(path, sizeof(path), "%s/%s", svdir, ent->d_name);
			print_status(path);
		}
		closedir(d);
		return 0;
	}

	snprintf(path, sizeof(path), "%s/%s", svdir, argv[0]);

	if (argc == 1) {
		print_status(path);
		return 0;
	}

	char cmd = argv[1][0];

	char control[1024];
	snprintf(control, sizeof(control), "%s/supervise/control", path);

	if (termfirst && !is_down(path)) {
		int ctl = open(control, O_WRONLY);
		if (ctl != -1) {
			char d = 'd';
			write(ctl, &d, 1);
			close(ctl);
		}
		for (int i = 0; i < TIMEOUT * 4; i++) {
			if (is_down(path))
				break;
			usleep(250 * 1000);
		}
	}

	time_t old_time = 0;
	if (waitchange)
		old_time = read_change_time(path);
	else
		print_status(path);

	int fd = open(control, O_WRONLY);
	if (fd == -1) {
		perror("open control");
		exit(1);
	}
	if (write(fd, &cmd, 1) != 1) {
		perror("write control");
		exit(1);
	}
	close(fd);

	if (waitchange) {
		for (int i = 0; i < TIMEOUT * 4; i++) {
			time_t now = read_change_time(path);
			if (now != old_time)
				break;
			usleep(250 * 1000);
		}
		print_status(path);
	}
	return 0;
}
