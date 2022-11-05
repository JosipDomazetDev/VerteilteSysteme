#include "client.h"

int main(int argc, char *argv[])
{
    Client client;
    if (argc < 3) // check if program was started correctly
    {
        printf("Usage: ./twmailer-client <ip> <port>\n");
        printf("Not enough arguments...\nSee usage!\n");
        return EXIT_FAILURE;
    }
    else
    {
        client.PORT = atoi(argv[2]); // save port
        client.serverIP = argv[3];   // save ip
        if (!client.start())         // create client socket and bind to ip
        {
            perror("start client");
            return EXIT_FAILURE;
        }

        client.connectServer(); // connect to server

        return EXIT_SUCCESS;
    }
}
