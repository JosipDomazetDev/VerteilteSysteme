#include "client.h"

void print_usage();

Client client;

int main(int argc, char *argv[]) {

    if (argc < 3) // check if program was started correctly
    {
        print_usage();
        return EXIT_FAILURE;
    } else {
        // Save port
        client.port = std::stoi(argv[2]);
        // Save ip
        client.serverIP = argv[3];

        // Create client socket and bind to ip
        if (!client.start())
        {
            perror("start client");
            return EXIT_FAILURE;
        }

        // Connect to server
        client.connectServer();

        return EXIT_SUCCESS;
    }
}

void print_usage() {
    printf("Usage: ./twmailer-client <ip> <port>\n");
}

