#include "mt.h"

int main(int argc, char* argv[]) {
    int                 sock, conn_fd;
    struct sockaddr_in  name, cliaddr;
    socklen_t 			clilen;
    char                buf[MAX_MSG_LENGTH];
    //pid_t     			childpid;

    /* Crear socket sobre el que se lee: dominio INET, protocolo UDP (DGRAM). */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("abriendo socket");
        exit(1);
    }

    /* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede enviar aquí. */
    bzero(&name,sizeof(name));
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *) &name, sizeof(name))) {
        perror("binding datagram socket");
        exit(1);
    }

    listen(sock,1024);

	clilen = sizeof(cliaddr);
	conn_fd = accept(sock,(struct sockaddr *)&cliaddr, &clilen);
  	if (conn_fd == -1) {
  		perror("accept");
    	exit(1);
  	}

  	/* Duplico el stdout */
    if(dup2(conn_fd, STDOUT_FILENO) == -1) {
        perror( "dup2" );
        exit(1);
    }

	/* Recibimos mensajes hasta que alguno sea el que marca el final. */
	for (;;) {
      	// recibo el mensaje del cliente
        memset(buf, 0, MAX_MSG_LENGTH);

        read(conn_fd, buf, MAX_MSG_LENGTH);
        //recvfrom(conn_fd, buf, MAX_MSG_LENGTH, 0, (struct sockaddr *)&cliaddr, &clilen);

        if (strncmp(buf, END_STRING, MAX_MSG_LENGTH) == 0)
            break;
        //printf("Comando: %s\n", buf);
        //fflush(stdout);

        // ejecutamos el programa
        system(buf);
        //printf("0\n");

        // flusheamos por si quedo algo
        fflush(stdout);
    }

    /* Cerrar socket de recepción. */
    close(sock);
    close(conn_fd);

    return 0;
}
