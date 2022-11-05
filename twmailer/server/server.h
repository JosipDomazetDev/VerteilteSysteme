#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <cstddef>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>

#define BUF 1024
#define MSGSIZE 11

class Server
{
private:
    int create_socket = -1;                 // socket descriptor (server)
    bool abortRequested = false;            // to check if request has been aborted
    socklen_t addr_length{};                      // size of addr structur
    struct sockaddr_in address{}, cliaddress{}; // socket addr structure
    int new_socket = -1;                    // socket descriptor (client)
    int socketA{};                            // socket descriptor (client)

public:
    Server();
    bool init();                                        // create server socket and bind to ip
    void start_listening();                             // wait for connections
    void abort();                                        // end connection
    void handle_client_communication(int *parameterSocket);      // communicate with client
    void handle_command(char buffer[BUF], int *parameterSocket); // handles the handle_command functions (SEND, ...)
    int port{};
    char *spoolDir{}; // spool directory of handle_command
    bool check_dir() const;

    bool create_server_socket() const;

    bool create_server_socket();

    static void
    handleSend(char *buffer, const int *current_socket, long size, const char *directory, char *filename, FILE *fptr,
               bool error) ;

    static void handleList(char *buffer, const int *current_socket, long size, const char *directory, char *filename,
                    bool error) ;

    static void handleRead(char *buffer, const int *current_socket, const char *directory, char *filename, FILE *fptr,
                    bool error) ;

    static void handleDel(char *buffer, const int *current_socket, const char *directory, char *filename, bool error) ;
};

#endif
