#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <vector>
#include <cstddef>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <thread>
#include <mutex>
#include <iterator>
#include <map>

#define BUF 1024
#define MSG_ID_LENGTH 12
#define MSGSIZE 11

class Server {
private:
    int create_socket = -1;                 // socket descriptor (server)
    bool abortRequested = false;            // to check if request has been aborted
    socklen_t addr_length{};                      // size of addr structur
    struct sockaddr_in address{}, cliaddress{}; // socket addr structure
    int new_socket = -1;                    // socket descriptor (client)
    std::mutex m;

public:
    Server();

    bool init();                                        // create server socket and bind to ip
    void start_listening();                             // wait for connections
    void abort();                                        // end connection
    void handle_client_communication(int *current_socket_new, const std::string& client_ip);      // communicate with client
    void
    handle_command(char buffer[1024], int parameterSocket, std::string &username, const std::string &client_ip,
                   int &failedLoginAttempts, bool &blocked); // handles the handle_command functions (SEND, ...)
    int port{};
    std::string spoolDir{}; // spool directory of handle_command
    [[nodiscard]] bool check_dir() const;

    bool create_server_socket();

    void
    handle_send(char buffer[1024], int current_socket, long size, std::string &directory, FILE *fptr,
                std::string string);
    static std::string
    handle_login(char buffer[1024], int current_socket, long size, std::string &directory, FILE *fptr, bool error,
                 const std::string client_ip, int &failedLoginAttempts, bool &blocked);


    static bool read_send_line(char *buffer, int current_socket, long size);


    static bool read_send_lines(char *buffer, int current_socket, long size, std::string &username,
                                std::string &receiverUsername, std::string &subject, std::string &msg);

    static bool persist_message_from_send(char buffer[1024], int current_socket, long size, FILE *fptr,
                                          std::string &username,
                                          std::string &receiverUsername, std::string &subject, std::string &msg,
                                          std::string &userDirectoryPath, std::string &filePath);


    static void
    handle_list(char buffer[1024], int current_socket, long size, std::string &directory, FILE *fptr, bool error,
                const std::string& string);

    static void
    handle_read(char buffer[1024], int current_socket, long size, std::string &directory, FILE *fptr, bool error,
                const std::string& string);

    void handle_del(char buffer[1024], int current_socket, long size, std::string &directory, bool error,
                    const std::string& string);
};

#endif
