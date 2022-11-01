#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc, char const *argv[]) {
    // Josip Domazet & Reynaud Cade

    struct sockaddr_in serv_address;
    int client_socket, port, n;
    char buffer[1024];

    if (argc < 3) {
        printf("provide required commend-line inputs as c> <server_ip> <port>");
        exit(1);
    }

    port = atoi(argv[2]);
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0) {
        printf("Error in opening socket");
        exit(1);
    }

    serv_address.sin_family = AF_INET;
    serv_address.sin_addr.s_addr = inet_addr(argv[1]);
    serv_address.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr *) &serv_address, sizeof(serv_address)) < 0) {
        printf("Connection failed :(");
        exit(1);
    }
    printf("Option keys:\nSEND\nREAD\nLIST\nDELETE\nQUIT");
    while (1) {
        bzero(buffer, 1024); //clear buffer
        printf("\n[Client]:");
        fgets(buffer, 1024, stdin);
        n = write(client_socket, buffer, strlen(buffer));
        if (n < 0) {
            printf("ERROR in writing");
            exit(1);
        }

        //now read back from the server
        bzero(buffer, 1024); //clear buffer
        n = read(client_socket, buffer, 1024);

        if (n < 0) {

            printf("ERROR in reading");
            exit(1);
        }

        //print server's response...
        printf("\nServer --> %s", buffer);


        //detected end of conversation
        if (!strncmp("QUIT", buffer, 4)) {
            break;
        }
    }
    close(client_socket);
    return 0;
}

