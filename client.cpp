#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 1000

int main(void)
{
    char msg[MAX];
    ///facem un socket;
    int c_socket;
    c_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(c_socket == -1)
    {
        printf("Eroare la crearea socketului!\n");
        exit(1);
    }

    /// adresa pentru socket;
    struct sockaddr_in adresa_server;
    adresa_server.sin_family = AF_INET;
    adresa_server.sin_port = htons(9002);
    adresa_server.sin_addr.s_addr = INADDR_ANY;

    ///conectare;
    int status_conectare = connect(c_socket, (struct sockaddr *) &adresa_server, sizeof(adresa_server));
    if(status_conectare == -1)
    {
        printf("Eroare conectare\n");
        exit(1);
    }
    else printf("Pentru a folosi messengerul trebuie sa va conectati!\nComenzile posibile: /login, /quit sau /register!\n");

    while(1)
    {
        bzero(msg, sizeof(msg));
        int ct = 0;
        printf("[Client] : ");
        while ((msg[ct++] = getchar()) != '\n')
            ;
        write(c_socket, msg, sizeof(msg));
        bzero(msg, sizeof(msg));
        read(c_socket, msg, sizeof(msg));
        printf("\n[Server] : %s", msg);
         
    }
    close(c_socket);
    return 0;
}