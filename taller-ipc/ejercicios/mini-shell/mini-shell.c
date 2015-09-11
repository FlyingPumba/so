#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>  /* pid_t */
#include <sys/wait.h>   /* waitpid */
#include <unistd.h>     /* exit, fork */

int run(char *program_name[], char **program_argv[], unsigned int count) {

    int child;
    char buf[1024];

    for (int i = 0; i < count; ++i) {
        child = fork();
        if (child == -1)
            perror("fork");

        if(dup2(STDIN_FILENO, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        //fflush(stdout);

        if (child == 0) {
            // estoy en el hijo
            execv(program_name[i], program_argv[i]);
            exit(0);
        } else {
            int status;
            wait(&status);
            // el hijo ya termino de ejecutar
        }
    }

    // imprimo el resultado final
    read(STDOUT_FILENO, buf, 1024);
    printf("%s\n", buf);

    return 0;
}

int main(int argc, char* argv[]) {
    /* Parsing de "ls -al | wc | awk '{ print $2 }'" */
    char *program_name[] = {
        "/bin/ls",
        "/usr/bin/wc",
        "/usr/bin/awk",
    };

    char *ls_argv[] = {"ls", "-al", NULL};
    char *wc_argv[] = {"wc", NULL};
    char *awk_argv[] = {"awk", "{ print $2 }", NULL};

    char **program_argv[] = {
        ls_argv,
        wc_argv,
        awk_argv,
    };

    unsigned int count = 3;

    int status = run(program_name, program_argv, count);

    printf("[+] Status : %d\n", status);

    return 0;
}
