#include "mt.h"

int main(int argc, char* argv[]) {
    /* TODO: Implementar */
    char* ip;

    if (argc != 2) {
      printf("usage:  mt_client <IP address>\n");
      exit(1);
   } else {
   		ip = argv[1];
    	printf("IP cliente: %s\n",ip);
   }

    int                 socket_fd, n;
    struct sockaddr_in  remote;
    char                buf[MAX_MSG_LENGTH];
    char 				recvline[MAX_MSG_LENGTH];

    /* Crear socket sobre el que se lee: dominio INET, protocolo UDP (DGRAM). */
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("abriendo socket");
        exit(1);
    }

    /* Establecer la dirección a la cual conectarse. */
    bzero(&remote,sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr=inet_addr(ip);
    remote.sin_port = htons(PORT);

    connect(socket_fd, (struct sockaddr *)&remote, sizeof(remote));

    /* Establecer la dirección a la cual conectarse para escuchar. */
    while(printf("> "), fgets(buf, MAX_MSG_LENGTH, stdin), !feof(stdin)) {
        if (sendto(socket_fd, buf, strlen(buf), 
        		0, (struct sockaddr *) &remote, sizeof remote) == -1) {
            perror("enviando");
            exit(1);
        }

        n = recv(socket_fd,recvline,MAX_MSG_LENGTH,0);
      	recvline[n]=0;
      	printf("Output del comando: %s\n", recvline);

        if (strncmp(buf, END_STRING, MAX_MSG_LENGTH) == 0)
            break;
    }

    /* Cerrar el socket. */
    close(socket_fd);


    return 0;
}
