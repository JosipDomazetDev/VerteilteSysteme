#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <iostream>
#include <cstring>

#define MAXLINE 1500
#define BUF 1024
//#define port 6543

class Client {
private:
    int create_socket{}; // socket descriptor (client)
    char buffer[BUF]{};
    struct sockaddr_in address{}; // socket addr structure
    long size{};
    bool isQuit = false; // check if client chose quit option

public:
    Client();

    bool start();         // create client socket
    void connectServer(); // connect to server and communicate
    void mailer();        // handles the mailer functions (SEND, ...)
    int port{};
    char *serverIP{};

    void handle_read();

    void handle_send();

    void handle_list();

    void handle_del();

    void extractUsername();

    void sendBuffer() const;
};

#endif
