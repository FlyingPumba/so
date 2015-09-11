#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int child_count = 0;

void sigurg_child(int n) {
	++child_count;
	printf("ya va\n");
}

int main(int argc, char* argv[]) {
	int status;
	pid_t child;

	if (argc <= 1) {
		fprintf(stderr, "Uso: %s commando [argumentos ...]\n", argv[0]);
		exit(1);
	}

	/* Fork en dos procesos */
	child = fork();
	if (child == -1) { perror("ERROR fork"); return 1; }
	if (child == 0) {
		/* S'olo se ejecuta en el Hijo */
		signal(SIGURG, sigurg_child);

		while (child_count<5) {}

		execvp(argv[1], argv+1);
		/* Si vuelve de exec() hubo un error */
		perror("ERROR child exec(...)"); exit(1);
	} else {
		/* S'olo se ejecuta en el Padre */
		int i;
		for(i = 0; i < 5; ++i) {
			sleep(1);
			printf("sup!\n");
			kill(child, SIGURG);
		}
		wait(&status);
	}
	return 0;
}