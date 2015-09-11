#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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
		if (ptrace(PTRACE_TRACEME, 0, NULL, NULL)) {
			perror("ERROR child ptrace(PTRACE_TRACEME, ...)");
			exit(1);
		}

		execvp(argv[1], argv+1);
		/* Si vuelve de exec() hubo un error */
		perror("ERROR child exec(...)"); exit(1);
	} else {
		/* S'olo se ejecuta en el Padre */
		while(1) {
			if (wait(&status) < 0) {
				perror("waitpid"); break;
			}
			if (WIFEXITED(status)) break; /* Proceso terminado */

			// el child está detenido
			int sysno = ptrace(PTRACE_PEEKUSER, child, 8*ORIG_RAX, NULL);

			if (sysno == 62) {
				//kill(child, SIGKILL);
				//ptrace(PTRACE_CONT, child, NULL, SIGKILL);/*Liberamos al hijo*/
				ptrace(PTRACE_KILL, child, NULL, NULL);/*Liberamos al hijo*/
				printf("Se ha hecho justicia!\n");
				break;
			}

			ptrace(PTRACE_SYSCALL, child, NULL, NULL); /* continua */
		}
		ptrace(PTRACE_DETACH, child, NULL, NULL);/*Liberamos al hijo*/
	}
	return 0;
}