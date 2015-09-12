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

    //char buf[1024];
    int comm[count][2];
    int status;
    /*int save_in = dup(STDIN_FILENO);
    int save_out = dup(STDOUT_FILENO);*/

    // creo los count pipes
    for (int i = 0; i < count; ++i) {
        pipe(comm[i]);
    }

    // creo los count-1 procesos (ya que el proceso padre es el cero)
    int i;
    int parent = getpid();
    int child[count-1];
    for (i = 0; i < count-1; ++i) {
        child[i] = fork();
        if (child[i] == 0) {
            break;
        }
    }

    // cierro todos los pipes que no le sirven a este proceso
    for (int j = 0; j < count; ++j) {
        if (j == i) {
            close(comm[j][1]);
        } else if (j == (i+1)%count) {
            close(comm[j][0]);
        } else {
            close(comm[j][0]);
            close(comm[j][1]);
        }
    }

    if (i == count-1) {
        // el padre ejecuta el primer programa sin input y pasa el output al segundo programa (i = 0)
        close(comm[count-1][0]);
        if (dup2(comm[0][1], STDOUT_FILENO) == -1) {
            fprintf(stderr, "dup2: %s %d\n", strerror(errno), i);
        }
        execv(program_name[0], program_argv[0]);
        if (fflush(stdout) == -1) {
            fprintf(stderr, "fflush: %s %d\n", strerror(errno), i);
        }
        close(comm[0][1]);
        exit(0);
    } else if (i == 0) {
        // el segundo programa espera el output del primer programa (padre) y lo pasa al tercer programa (i = 1)
        waitpid(parent, &status, 0);
        if (dup2(comm[i][0], STDIN_FILENO) == -1) {
            fprintf(stderr, "dup2: %s %d\n", strerror(errno), i);
        }
        if (dup2(comm[(i+1)%(count-1)][1], STDOUT_FILENO) == -1) {
            fprintf(stderr, "dup2: %s %d\n", strerror(errno), i);
        }
        execv(program_name[i+1], program_argv[i+1]);
        if (fflush(stdout) == -1) {
            fprintf(stderr, "fflush: %s %d\n", strerror(errno), i);
        }
        close(comm[i][0]);
        close(comm[(i+1)%(count-1)][1]);
        exit(0);
    } else if (i == count-2) {
        waitpid(child[count-3], &status, 0);
        // el ultimo programa espera el output del anteultimo e imprime en pantalla
        if (dup2(comm[i][0], STDIN_FILENO) == -1) {
            fprintf(stderr, "dup2: %s %d\n", strerror(errno), i);
        }
        execv(program_name[i+1], program_argv[i+1]);
        if (fflush(stdout) == -1) {
            fprintf(stderr, "fflush: %s %d\n", strerror(errno), i);
        }
        close(comm[i][0]);
        close(comm[(i+1)%(count-1)][1]);
    } else {
        // el programa i espera el input del programa i-1, y pasa su output al programa i+1
        waitpid(child[i-1], &status, 0);
        if (dup2(comm[i][0], STDIN_FILENO) == -1) {
            fprintf(stderr, "dup2: %s %d\n", strerror(errno), i);
        }
        if (dup2(comm[(i+1)%(count-1)][1], STDOUT_FILENO) == -1) {
            fprintf(stderr, "dup2: %s %d\n", strerror(errno), i);
        }
        execv(program_name[i+1], program_argv[i+1]);
        if (fflush(stdout) == -1) {
            fprintf(stderr, "fflush: %s %d\n", strerror(errno), i);
        }
        close(comm[i][0]);
        close(comm[(i+1)%(count-1)][1]);
    }

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
