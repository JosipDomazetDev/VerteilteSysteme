#include "server.h"

Server::Server() = default;


bool Server::create_server_socket() {
    int reuseValue = 1;

    // Create socket
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) // if -1 -> error
    {
        perror("Socket error");
        return false;
    }
    // Allow reuse of local address and port
    if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &reuseValue, sizeof(reuseValue)) == -1) {
        perror("Set socket options - reuseAddr");
        return false;
    }
    if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEPORT, &reuseValue, sizeof(reuseValue)) == -1) {
        perror("Set socket options - reusePort");
        return false;
    }


    // IP and port allocation
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);


    // Binding newly created socket to given IP
    if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == -1) {
        perror("bind error");
        return false;
    }

    return true;
}

bool Server::check_dir() const {
    // Creating pointer of type dirent (all directory entries in specific directory)
    DIR *directory = opendir(spoolDir);
    if (directory == nullptr) {
        // Directory could not be opened
        perror("Error when opening directory");
        return false;
    }

    return true;
}

// Create server socket and bind to ip, check dir
bool Server::init() {
    if (!create_server_socket()) {
        return false;
    }

    if (!check_dir()) {
        return false;
    }

    return true;
}

// wait for connections
void Server::start_listening() {
    // 5 queued connections allowed
    if (listen(create_socket, 5) == -1) {
        perror("listen error");
        return;
    }

    while (!abortRequested) {
        printf("Waiting for connections...\n");

        // ACCEPTS CONNECTION SETUP
        addr_length = sizeof(struct sockaddr_in);
        if ((new_socket = accept(create_socket, (struct sockaddr *) &cliaddress, &addr_length)) == -1) {
            perror("accept error");
            break;
        }

        // Start client
        printf("Client connected from %s:%d...\n", inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));

        socketA = new_socket;
        handle_client_communication(&socketA);

        new_socket = -1;
    }

    // frees the descriptor
    if (create_socket != -1) {
        if (shutdown(create_socket, SHUT_RDWR) == -1) {
            perror("shutdown create_server_socket");
        }
        if (close(create_socket) == -1) {
            perror("close create_server_socket");
        }
        create_socket = -1;
    }
}

// communicate with client
void Server::handle_client_communication(int *current_socket) {
    char buffer[BUF];
    long size;

    do {
        printf("-------------------- \n");
        // RECEIVE from client
        bzero(buffer, BUF); // clear buffer
        size = recv(*current_socket, buffer, BUF, 0);
        if (size == -1) {
            if (abortRequested) {
                perror("recv error after aborted");
            } else {
                perror("recv error");
            }
            break;
        } else if (size == 0) {
            printf("Client closed remote socket\n"); // ignore error
            break;
        }
        printf("\nMessage received: %s\n", buffer);
        handle_command(buffer, current_socket);
    } while (strcmp(buffer, "quit") != 0 && !abortRequested);

    // closes/frees the descriptor if not already
    if (*current_socket != -1) {
        if (shutdown(*current_socket, SHUT_RDWR) == -1) {
            perror("shutdown new_socket");
        }
        if (close(*current_socket) == -1) {
            perror("close new_socket");
        }
        *current_socket = -1;
    }
}

// end connection
void Server::abort() {
    abortRequested = true;
    if (new_socket != -1) {
        if (shutdown(new_socket, SHUT_RDWR) == -1) {
            perror("shutdown new_socket");
        }
        if (close(new_socket) == -1) {
            perror("close new_socket");
        }
        new_socket = -1;
    }

    if (create_socket != -1) {
        if (shutdown(create_socket, SHUT_RDWR) == -1) {
            perror("shutdown create_server_socket");
        }
        if (close(create_socket) == -1) {
            perror("close create_server_socket");
        }
        create_socket = -1;
    }
}

// handles the handle_command functions (SEND, ...)
void Server::handle_command(char buffer[BUF], int *current_socket) {

    long size;
    char *directory = spoolDir;
    char filename[BUF];
    FILE *fptr;
    bool error = false; // check if error

    if (strncmp(buffer, "SEND", 4) == 0) {
        handleSend(buffer, current_socket, size, directory, filename, fptr, error);
        return;
    } else if (strncmp(buffer, "LIST", 4) == 0) // list all subjects of a user
    {
        handleList(buffer, current_socket, size, directory, filename, error);
        return;
    } else if (strncmp(buffer, "READ", 4) == 0) // read specific message
    {
        handleRead(buffer, current_socket, directory, filename, fptr, error);
        return;
    } else if (strncmp(buffer, "DEL", 3) == 0) // delete specific file (subject)
    {
        handleDel(buffer, current_socket, directory, filename, error);
        return;
    }
    }

void Server::handleDel(char buffer[1024], const int *current_socket, const char *directory, char *filename,
                       bool error) {
    bzero(buffer, BUF);
    for (int i = 0; i < 2; i++) {
        recv(*current_socket, buffer, BUF, 0);
        printf("Information received: %s", buffer);
        if (i == 0) {
            buffer[strlen(buffer) - 1] = '\0';
            strcpy(filename, directory);
            strcat(filename, "/");
            strcat(filename, buffer); // get path of username directory
        } else if (i == 1) {
            buffer[strlen(buffer) - 1] = '\0';
            strcat(filename, "/");
            strcat(filename, buffer); // get path of file (subject)
            if (remove(filename))     // delete file
            {
                perror("Error when deleting file");
                error = true;
            } else {
                printf("\nFile was deleted successfully!\n");
            }
        }
    }
    bzero(buffer, BUF);
    if (!error) {
        printf("\n\nEverything went well!\n");
        strcat(buffer, "\nOK\n");
    } else {
        printf("\n\nThere was an error!\n");
        strcat(buffer, "ERR\n");
    }
    if ((send(*current_socket, buffer, BUF, 0)) == -1) // send message
    {
        perror("del error");
    }
    bzero(buffer, BUF);
}

void Server::handleRead(char buffer[1024], const int *current_socket, const char *directory, char *filename, FILE *fptr,
                        bool error) {
    char message[BUF];
    bzero(buffer, BUF);
    bzero(filename, BUF);
    for (int i = 0; i < 2; i++) {
        recv(*current_socket, buffer, BUF, 0);
        printf("Information received: %s", buffer);
        if (i == 0) {
            buffer[strlen(buffer) - 1] = '\0';
            strcpy(filename, directory);
            strcat(filename, "/");
            strcat(filename, buffer); // get path of user directory
        } else if (i == 1) {
            buffer[strlen(buffer) - 1] = '\0';
            strcat(filename, "/");
            strcat(filename, buffer); // get path of specifc file (subject)
            // printf("%s\n", filename);
            fptr = fopen(filename, "r"); // open file to read
            if (fptr == NULL) {
                perror("Unable to open file");
                error = true;
            } else {
                bzero(buffer, BUF);
                printf("\n");
                int j = 0;
                while (fscanf(fptr, "%s", buffer) != EOF) // scan all lines
                {
                    printf("%s\n", buffer);
                    if (j != 0) // save all lines except first one (=receiver)
                    {
                        strcat(message, buffer);
                        strcat(message, "\n");
                    }
                    j = 1;
                }
            }
        }
    }
    if (!error) {
        printf("\n\nEverything went well!\n");
        strcat(message, "\nOK\n");
    } else {
        printf("\n\nThere was an error!\n");
        bzero(message, BUF);
        strcat(message, "ERR\n");
    }
    if ((send(*current_socket, message, BUF, 0)) == -1) {
        perror("read error");
    }
    bzero(buffer, BUF);
    bzero(message, BUF);
    bzero(filename, BUF);
    fclose(fptr);
}

void Server::handleList(char buffer[1024], const int *current_socket, long size, const char *directory, char *filename,
                        bool error) {
    char subjects[BUF];
    bzero(subjects, BUF);
    char c[BUF];
    bzero(c, BUF);
    int count = 0;
    bzero(buffer, BUF);
    size = recv(*current_socket, buffer, BUF, 0); // receive username
    if (size <= 0) {
        perror("list error");
    }
    printf("Username received: %s\n", buffer);

    buffer[strlen(buffer) - 1] = '\0';
    strcpy(filename, directory);
    strcat(filename, "/");
    strcat(filename, buffer); // get path of directory of user

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(filename)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] != '.' &&
                ent->d_name[strlen(ent->d_name) - 1] != '~') // get all files except  ., .., hidden files
            {
                count++;                       // count files
                strcat(subjects, ent->d_name); // save them all into subjects
                strcat(subjects, "\n");
            }
        }
        sprintf(c, "%d", count); // convert count int into char array
        strcat(c, "\n");
        strcat(c, subjects); // concenate count and subjects
        bzero(subjects, BUF);
        closedir(dir);
    } else {
        perror("Error when opening directory");
        error = true;
    }
    printf("Subjects:\n%s", c);

    if (count == 0 || error) {
        bzero(c, BUF);
        strcat(c, "0\n"); // when no user or files -> 0
    }

    if (send(*current_socket, c, BUF, 0) == -1) // send message
    {
        perror("list error");
    }
    bzero(filename, BUF);
    bzero(c, BUF);
}

void Server::handleSend(char buffer[1024], const int *current_socket, long size, const char *directory, char *filename,
                        FILE *fptr, bool error) {
    char receiver[BUF]; // username of receiver
    bzero(buffer, BUF);

    for (int i = 0; i < 4; i++) // receive all parts of send (sender, receiver, subject, message)
    {
        size = recv(*current_socket, buffer, BUF, 0);
        if (size <= 0) {
            perror("recv error");
            break;
        }
        printf("Information received: %s", buffer);

        if (i == 0) {
            buffer[strlen(buffer) - 1] = '\0';
            strcpy(filename, directory); // get path of directory for user
            strcat(filename, "/");
            strcat(filename, buffer);
            // printf("filename: %s\n", filename);
            if (mkdir(filename, 0777) == -1) // make directory
            {
                if (errno == EEXIST) // ignore error if directory already exists
                {
                } else {
                    error = true; // all other errors set bool on true
                }
            }
        } else if (i == 1 && error == false) {
            strcpy(receiver, buffer); // save username of receiver
            // printf("receiver: %s\n", receiver);
        } else if (i == 2 && error == false) {
            buffer[strlen(buffer) - 1] = '\0';
            strcat(filename, "/");
            strcat(filename, buffer); // get path for file (named after subject)
            // printf("filename: %s\n", filename);
            fptr = fopen(filename, "w"); // create file
            if (fptr == NULL) {
                printf("Unable to create file.\n");
                error = true;
            } else {
                fputs(receiver, fptr); // print receiver into file
            }
        } else if (i == 3 && error == false) {
            fputs(buffer, fptr); // print message into file
            fclose(fptr);
        }
    }

    bzero(filename, BUF);
    // printf("filename1: %s\n", filename);
    bzero(receiver, BUF);
    bzero(buffer, BUF);
    if (!error) {
        printf("\n\nEverything went well!\n");
        buffer = (char *) "OK\n";
    } else {
        printf("\n\nThere was an error!\n");
        buffer = (char *) "ERR\n";
    }
    if (send(*current_socket, buffer, BUF, 0) == -1) // send message to client
    {
        perror("send error");
    }
}
