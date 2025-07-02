#include "arg.h"

#include <ctype.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define TAI_OFFSET       4611686018427387914ULL
#define MAX_DEPENDENCIES 128
#define LINE_MAX         512

#define FOREACH_DEP(i)                                                              \
	for (int i = 0, hits = 0; i < MAX_DEPENDENCIES && hits < dependency_count; i++) \
		if (dependencies[i].pid == -1)                                              \
			continue;                                                               \
		else                                                                        \
			for (int __once = (hits++, 0); __once < 1; __once++)


struct dependency {
	pid_t pid;
	char  name[NAME_MAX];
	int   enlisted; /* is seen */
};

const char       *self;
struct dependency dependencies[MAX_DEPENDENCIES]; /* -1 is unset */
int               dependency_count = 0;
int               terminating      = 0;

pid_t  process       = 0;
time_t status_change = 0;
int    do_restart    = 1;
int    exitstatus    = 0;
int    paused        = 0;

time_t depends_mtime = 0;

void write_status(void) {
	char     buffer[20];
	uint64_t tai = status_change + TAI_OFFSET;

	/* seconds (TAI) since status-change */
	buffer[0] = (tai >> 56) & 0xff;
	buffer[1] = (tai >> 48) & 0xff;
	buffer[2] = (tai >> 40) & 0xff;
	buffer[3] = (tai >> 32) & 0xff;
	buffer[4] = (tai >> 24) & 0xff;
	buffer[5] = (tai >> 16) & 0xff;
	buffer[6] = (tai >> 8) & 0xff;
	buffer[7] = (tai >> 0) & 0xff;

	/* micro-seconds, not that interesting... */
	buffer[8]  = 0;
	buffer[9]  = 0;
	buffer[10] = 0;
	buffer[11] = 0;

	/* PID */
	buffer[15] = (process >> 24) & 0xff;
	buffer[14] = (process >> 16) & 0xff;
	buffer[13] = (process >> 8) & 0xff;
	buffer[12] = (process >> 0) & 0xff;

	/* paused */
	buffer[16] = paused;
	/* wants up/down */
	buffer[17] = do_restart ? 'u' : 'd';
	/* was terminated */
	buffer[18] = WIFSIGNALED(exitstatus) && WTERMSIG(exitstatus) == SIGTERM;
	/* runit-state */
	buffer[19] = process != 0;


	FILE *f = fopen("supervise/status", "w");
	if (!f) {
		perror("fopen status");
		return;
	}
	fwrite(buffer, sizeof(buffer), 1, f);
	fclose(f);


	f = fopen("supervise/stat", "w");
	if (!f) {
		perror("fopen status");
		return;
	}
	if (process == 0) {
		fprintf(f, "down");
	} else {
		fprintf(f, "run");
	}
	fclose(f);

	f = fopen("supervise/pid", "w");
	if (!f) {
		perror("fopen status");
		return;
	}
	fprintf(f, "%d", process);
	fclose(f);
}

int has_lock(const char *path) {
	char lockpath[PATH_MAX];
	if (!path)
		strlcpy(lockpath, "supervise/lock", sizeof(lockpath));
	else
		snprintf(lockpath, sizeof(lockpath), "../%s/supervise/lock", path);

	int lockfd = open(lockpath, O_WRONLY);
	if (lockfd == -1)
		return 0;

	int ret = lockf(lockfd, F_TEST, 0) != 0;
	close(lockfd);
	return ret;
}

pid_t must_fork(void) {
	pid_t pid;
	for (;;) {
		pid = fork();
		if (pid != -1)
			break;
		fprintf(stderr, "unable to fork: %s, waiting...\n", strerror(errno));
		sleep(1);
	}
	return pid;
}

char *strip(char *origin) {
	char *comment = strchr(origin, '#');
	if (comment != NULL) {
		*comment = '\0';
	}

	while (isspace(origin[0]))
		origin++;
	if (origin[0] == '\0')
		return origin;

	for (char *c = strchr(origin, '\0') - 1; isspace(*c) && c >= origin; c--)
		*c = '\0';
	return origin;
}

void reload_dependencies(void) {
	struct stat filestat;
	if (stat("depends", &filestat) == -1)
		return;

	if (filestat.st_mtim.tv_sec <= depends_mtime)
		return;

	FOREACH_DEP(i) {
		dependencies[i].enlisted = 0;
	}

	FILE *file = fopen("depends", "r");
	if (!file)
		return;

	char line[NAME_MAX];
	char path[PATH_MAX];
	while (fgets(line, sizeof(line), file)) {
		line[strcspn(line, "\n")] = '\0';

		char *name = strip(line);
		if (name[0] == '\0')
			continue;

		FOREACH_DEP(i) {
			if (!strcmp(line, dependencies[i].name)) {
				dependencies[i].enlisted = 1;
			}
		}

		if (dependency_count >= MAX_DEPENDENCIES) {
			fprintf(stderr, "too many dependencies, stopping at %s\n", line);
			break;
		}

		if (has_lock(line))
			continue;

		snprintf(path, sizeof(path), "../%s", line);

		if (access(path, R_OK) != 0) {
			continue;
		}

		pid_t pid = must_fork();
		if (pid == 0) {
			chdir(path);
			execlp(self, self, NULL);
			perror("execlp");
			_exit(127);
		}

		for (int i = 0; i < MAX_DEPENDENCIES; i++) {
			if (dependencies[i].pid != -1)
				continue;
			dependencies[i].pid = pid;
			strncpy(dependencies[i].name, line, sizeof(dependencies[i].name));
			dependencies[i].enlisted = 1;
			break;
		}
		dependency_count++;
	}
	fclose(file);

	FOREACH_DEP(i) {
		if (dependencies[i].enlisted)
			continue;
		printf("killing %s (pid %d)\n", dependencies[i].name, dependencies[i].pid);
		kill(dependencies[i].pid, SIGTERM);
	}
}

void start_process(void) {
	if (process != 0)
		return;

	if (access("run", X_OK) != 0) {
		return;
	}

	pid_t pid = must_fork();
	if (pid == 0) {
		execl("./run", "./run", NULL);
		perror("execlp");
		_exit(127);
	}

	process       = pid;
	status_change = time(NULL);
	write_status();
}

void stop_process(int sig) {
	if (process == 0)
		return;

	kill(process, sig);
}

void usage(int code) {
	fprintf(stderr, "supervise [-C dir]");
	exit(code);
}

void on_sigchild(int signo) {
	if (terminating)
		return;

	int   status;
	pid_t killed;
	(void) signo;

	while ((killed = waitpid(-1, &status, WNOHANG)) > 0) {
		if (killed == process) {
			exitstatus    = status;
			process       = 0;
			paused        = 0;
			status_change = time(NULL);
			write_status();
			if (do_restart) {
				sleep(1);
				start_process();
			}
			continue;
		}

		// Was it a dependency?
		int is_dependency = 0;
		FOREACH_DEP(i) {
			if (dependencies[i].pid == killed) {
				dependencies[i].pid = -1;
				dependency_count--;
				if (dependencies[i].enlisted)
					is_dependency = 1;
				break;
			}
		}
		if (is_dependency) {
			fprintf(stderr, "dependency exited, reloading depends...\n");
			reload_dependencies();
		}
	}
}

void on_sigusr1(int signo) {
	(void) signo;
	reload_dependencies();
}

void on_sigalarm(int signo) {
	if (terminating)
		return;
	(void) signo;
	reload_dependencies();
	alarm(1);
}

void shutdown_supervisor(void) {
	do_restart = 0;

	FOREACH_DEP(i) {
		if (dependencies[i].pid != -1) {
			kill(dependencies[i].pid, SIGTERM);
			waitpid(dependencies[i].pid, NULL, 0);
			dependencies[i].pid = -1;
		}
	}

	if (process != 0) {
		kill(process, SIGTERM);
		waitpid(process, NULL, 0);
		process       = 0;
		status_change = time(NULL);
		write_status();
	}
}

void handle_command(char chr) {
	fprintf(stderr, "command %c\n", chr);
	switch (chr) {
		case 'u':
			do_restart = 1;
			start_process();
			break;
		case 'd':
			do_restart = 0;
			stop_process(SIGTERM);
			break;
		case 'o':
			do_restart = 0;
			start_process();
			break;
		case 't':
			stop_process(SIGTERM);
			break;
		case 'p':
			paused = 1;
			stop_process(SIGSTOP);
			break;
		case 'c':
			paused = 0;
			stop_process(SIGCONT);
			break;
		case 'a':
			stop_process(SIGALRM);
			break;
		case 'h':
			stop_process(SIGHUP);
			break;
		case 'i':
			stop_process(SIGINT);
			break;
		case 'q':
			stop_process(SIGQUIT);
			break;
		case '1':
			stop_process(SIGUSR1);
			break;
		case '2':
			stop_process(SIGUSR2);
			break;
		case 'x':
			shutdown_supervisor();
			break;
	}
}

void read_control_loop(void) {
	int fd;
	for (;;) {
		/* if this directory does not exist anymore, exit. */
		if (access(".", R_OK) != 0) {
			return;
		}

		if ((fd = open("supervise/control", O_RDONLY)) == -1) {
			perror("open control");
			sleep(1);
			continue;
		}

		int c, n;
		while ((n = read(fd, &c, 1)) != 0) {
			if (n == -1 && errno != EINTR) {
				break;
			}
			handle_command(c);
		}

		close(fd);
	}
}

void on_sigterm(int signo) {
	(void) signo;
	fprintf(stderr, "received termination signal, shutting down...\n");
	terminating = 1;
	shutdown_supervisor();
	exit(0);
}

int main(int argc, char **argv) {
	self          = argv[0];
	status_change = time(NULL);

	for (int i = 0; i < MAX_DEPENDENCIES; i++) {
		dependencies[i].pid      = -1;
		dependencies[i].enlisted = 0;
	}

	signal(SIGCHLD, on_sigchild);
	signal(SIGUSR1, on_sigusr1);
	signal(SIGALRM, on_sigalarm);
	signal(SIGTERM, on_sigterm);
	signal(SIGINT, on_sigterm);
	signal(SIGQUIT, on_sigterm);

	const char *dir;
	ARGBEGIN
	switch (OPT) {
		case 'h':
			usage(0);
			break;
		case 'C':
			dir = EARGF(usage(1));
			if (chdir(dir) == -1) {
				perror("chdir");
				exit(1);
			}
			break;
	}
	ARGEND

	if (mkdir("supervise", 0755) < 0 && errno != EEXIST) {
		perror("mkdir");
		return 1;
	}

	if (has_lock(NULL)) {
		return 0;
	}

	mkfifo("supervise/ok", 0600);
	mkfifo("supervise/control", 0600);

	int lockfd = open("supervise/lock", O_WRONLY | O_CREAT, 0600);
	if (lockfd == -1) {
		perror("open lockfile");
		return 1;
	}

	if (lockf(lockfd, F_LOCK, 0) < 0) {
		perror("flock");
		close(lockfd);
		return 1;
	}

	int controlfd = open("supervise/ok", O_RDONLY | O_NONBLOCK);
	if (controlfd == -1) {
		perror("open ok");
		close(lockfd);
		return 1;
	}

	reload_dependencies();
	start_process();

	alarm(1);
	read_control_loop();

	// on exit
	close(lockfd);
	close(controlfd);

	shutdown_supervisor();
}
