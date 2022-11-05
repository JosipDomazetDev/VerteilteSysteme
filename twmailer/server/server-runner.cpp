#include <iostream>
#include <csignal>
#include "server.h"

void signalHandler(int);

void print_usage();

Server server;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        // Check if enough arguments were provided
        print_usage();
        return EXIT_FAILURE;
    }
    else
    {
        // Set the port of the server
        server.port = std::stoi(argv[1]);
        // Set the directory name
        server.spoolDir = argv[2];

        // Sets handler for signal, SIGINT = external interrupt (often by user)
        if (signal(SIGINT, signalHandler) == SIG_ERR)
        {
            perror("Signal can not be registered");
            return EXIT_FAILURE;
        }

        // Create server socket and bind to ip
        if (!server.init())
        {
            perror("Starting server");
            return EXIT_FAILURE;
        }

        // Wait for connections
        server.start_listening();

        return 0;
    }
}

void print_usage() {
    printf("Usage: ./twmailer-server <port> <mail-spool-directoryname>\n");
}

void signalHandler(int sig)
{
    if (sig == SIGINT)
    {
        printf("Cancelling... ");
        // end connection
        server.abort();
    }
    else
    {
        exit(sig);
    }
}
