#include "arg.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int killsig = 0;

void handle_kill(int signo) {
	killsig = signo;
}

void usage(int code) {
	fprintf(stderr, "sni-waitsignal [-n]");
	exit(code);
}

int main(int argc, char **argv) {
	int printname = 0;

	ARGBEGIN
	switch (OPT) {
		case 'h':
			usage(0);
			break;
		case 'n':
			printname++;
			break;
	}
	ARGEND

	for (int i = 1; i < NSIG; i++)
		signal(i, handle_kill);

	pause();

	if (printname)
		printf("%s\n", strsignal(killsig));
	else
		printf("%d\n", killsig);
}
