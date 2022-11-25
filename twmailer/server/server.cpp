#include "server.h"
#include "ldap.h"
#include "ldap.cpp"

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
    DIR *directory = opendir(spoolDir.c_str());
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

    std::vector<std::thread> threads;

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

        std::thread new_thread(&Server::handle_client_communication, this, &new_socket);
        threads.push_back(std::move(new_thread));
    }


    //join all threads
    auto originalthread = threads.begin();
    while (originalthread != threads.end()) {
        originalthread->join();
        originalthread++;
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
    int current_socket_new = *current_socket;
    *current_socket = -1;
    std::string username;

    do {
        printf("-------------------- \n");
        // RECEIVE from client
        bzero(buffer, BUF); // clear buffer
        size = recv(current_socket_new, buffer, BUF, 0);
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
        handle_command(buffer, current_socket_new, username);
    } while (strcmp(buffer, "quit") != 0 && !abortRequested);

    // closes/frees the descriptor if not already
    if (current_socket_new != -1) {
        if (shutdown(current_socket_new, SHUT_RDWR) == -1) {
            perror("shutdown new_socket");
        }
        if (close(current_socket_new) == -1) {
            perror("close new_socket");
        }
        current_socket_new = -1;
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

std::string gen_random(const int len) {
    static const char alphanum[] =
            "0123456789"
            "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

// handles the handle_command functions (SEND, ...)
void Server::handle_command(char buffer[1024], int parameterSocket, std::string &username) {

    long size = 0;
    std::string directory = spoolDir;
    FILE *fileptr = nullptr;
    bool error = false; // check if error

    printf("%s", "----------------");
    printf("%s", buffer);
    printf("%s", username.c_str());
    printf("%d", strncmp(buffer, "LOGIN", 5) == 0);

    if (strncmp(buffer, "LOGIN", 5) == 0) {
        username = handle_login(buffer, parameterSocket, size, directory, fileptr, error);
    } else {
        if (strncmp(buffer, "SEND", 4) == 0 || strncmp(buffer, "LIST", 4) == 0 || strncmp(buffer, "READ", 4) == 0 ||
            strncmp(buffer, "DEL", 3) == 0) {
            // SEND LIST READ and DEL require authentication now
            if (username.empty()) {
                // Not authenticated
                if (send(parameterSocket, "ERR - NOT AUTHENTICATED", BUF, 0) == -1) {
                    perror("send error");
                }

                return;
            }
        }

        if (strncmp(buffer, "SEND", 4) == 0) {
            m.lock();
            handle_send(buffer, parameterSocket, size, directory, fileptr, username);
            m.unlock();
        } else if (strncmp(buffer, "LIST", 4) == 0) // list all subjects of a user
        {
            handle_list(buffer, parameterSocket, size, directory, fileptr, error, username);
        } else if (strncmp(buffer, "READ", 4) == 0) // read specific message
        {
            handle_read(buffer, parameterSocket, size, directory, fileptr, error, username);
        } else if (strncmp(buffer, "DEL", 3) == 0) // delete specific file (subject)
        {
            m.lock();
            handle_del(buffer, parameterSocket, size, directory, error, username);
            m.unlock();
        } else {
            printf("\n\nUnknown Command %s\n", buffer);
        }
    }
}

std::string
Server::handle_login(char buffer[1024], int current_socket, long size, string &directory, FILE *fptr, bool error) {
    std::string username;
    std::string password;

    if (read_send_line(buffer, current_socket, size)) {
        // 0 -- username
        buffer[strlen(buffer) - 1] = '\0';
        username = buffer;
    } else {
        error = false;
    }

    if (read_send_line(buffer, current_socket, size)) {
        // 1 -- pw
        buffer[strlen(buffer) - 1] = '\0';
        password = buffer;
    } else {
        error = false;
    }

    if (error) {
        if (send(current_socket, "ERR", BUF, 0) == -1) {
            perror("send error");
        }
        return "";
    }

    Ldap loginld;
    loginld.setupLdap();
    loginld.inputUser(username, password);

    // bind credentials and to check if valid
    if (loginld.bindCredentials()) {
        // Login successful

        if (send(current_socket, "OK", BUF, 0) == -1) {
            perror("send error");
        }
        loginld.ldapSearch(); // if valid start ldapSearch
        return username;
    } else {
        // Login failed

        if (send(current_socket, "ERR", BUF, 0) == -1) {
            perror("send error");
        }

        return "";
        /*  if (failed > 1 && !blocked) // if 3 failed attempts block ip
          {
              printf("Client is blocked!\n");
              auto start = chrono::system_clock::now(); // take current time
              time_t starttime = chrono::system_clock::to_time_t(start);
              blocklist << IP << " : " << ctime(&starttime) << endl; // write in blocklist.txt ip and time
              blocked = true;
              return;
          }
          failed++;*/
    }
}

void Server::handle_del(char buffer[1024], int current_socket, long size, std::string &directory, bool error,
                        const std::string& username) {

    bzero(buffer, BUF);
    std::string message_id;
    std::string userDirectoryPath = directory;
    std::string filePath;
    std::string complete_msg;

    bzero(buffer, BUF);

    if (read_send_line(buffer, current_socket, size)) {
        // 0 -- message-id
        buffer[strlen(buffer) - 1] = '\0';
        message_id = buffer;
    } else {
        error = false;
    }

    userDirectoryPath.append("/");
    userDirectoryPath.append(username);
    filePath = userDirectoryPath;
    filePath.append("/");
    filePath.append(message_id);


    if (remove(filePath.c_str()))     // delete file
    {
        perror("Error when deleting file");
        error = true;
    } else {
        printf("\nFile was deleted successfully!\n");
    }

    if (!error) {
        printf("\n\nEverything went well!\n");
        complete_msg = "\nOK\n";
    } else {
        complete_msg = "ERR\n";
        printf("\n\nThere was an error!\n");
    }
    if ((send(current_socket, complete_msg.c_str(), BUF, 0)) == -1) // send message
    {
        perror("del error");
    }
    bzero(buffer, BUF);
}

void
Server::handle_read(char buffer[1024], int current_socket, long size, std::string &directory, FILE *fptr, bool error,
                    const std::string& username) {
    std::string message_id;
    std::string userDirectoryPath = directory;
    std::string filePath;
    std::string complete_msg;

    bzero(buffer, BUF);

    if (read_send_line(buffer, current_socket, size)) {
        // 0 -- message-id
        buffer[strlen(buffer) - 1] = '\0';
        message_id = buffer;
    } else {
        error = false;
    }

    userDirectoryPath.append("/");
    userDirectoryPath.append(username);
    filePath = userDirectoryPath;
    filePath.append("/");
    filePath.append(message_id);

    fptr = fopen(filePath.c_str(), "r"); // open file to read
    if (fptr == nullptr) {
        perror("Unable to open file");
        error = true;
    } else {
        bzero(buffer, BUF);
        printf("\n");
        while (fscanf(fptr, "%s", buffer) != EOF) // scan all lines
        {
            printf("%s\n", buffer);
            complete_msg.append(buffer);
            complete_msg.append("\n");
        }
    }

    if (!error) {
        printf("\n\nEverything went well!\n");
        complete_msg.insert(0, "\nOK\n");
    } else {
        printf("\n\nThere was an error!\n");
        complete_msg = "ERR\n";
    }

    if ((send(current_socket, complete_msg.c_str(), BUF, 0)) == -1) {
        perror("read error");
    }
    bzero(buffer, BUF);
    fclose(fptr);
}

void
Server::handle_list(char buffer[1024], int current_socket, long size, std::string &directory, FILE *fptr, bool error,
                    const std::string& username) {
    std::string userDirectoryPath = directory;
    std::vector<std::string> message_ids;
    std::string subjects;
    std::string complete_msg;
    int count = 0;

    userDirectoryPath.append("/");
    userDirectoryPath.append(username);

    // First get all message_ids (filenames)
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(userDirectoryPath.c_str())) != nullptr) {

        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_name[0] != '.' &&
                ent->d_name[strlen(ent->d_name) - 1] != '~') // get all files except  ., .., hidden files
            {
                message_ids.emplace_back(ent->d_name);
            }
        }
        closedir(dir);
    } else {
        perror("Error when opening directory");
    }

    std::string filePath;

    // After that open the files and get the subjects
    for (const std::string &message_id: message_ids) {
        filePath = userDirectoryPath;
        filePath.append("/");
        filePath.append(message_id);

        fptr = fopen(filePath.c_str(), "r"); // open file to read
        if (fptr == nullptr) {
            perror("Unable to open file");
            error = true;
        } else {
            bzero(buffer, BUF);
            printf("\n");
            int i = 0;
            while (fscanf(fptr, "%s", buffer) != EOF) // scan all lines
            {
                if (i == 2) {
                    // Third line in file is the subject
                    printf("%s\n", buffer);
                    subjects.append(buffer);
                    subjects.append("\n");

                    // count subjects
                    count++;
                }

                i++;
            }
        }
    }


    complete_msg.append(std::to_string(count));
    complete_msg.append("\n");
    complete_msg.append(subjects);

    if (count == 0 || error) {
        complete_msg = "0\n";
    }

    printf("Subjects:\n%s", complete_msg.c_str());

    if (send(current_socket, complete_msg.c_str(), BUF, 0) == -1) // send message
    {
        perror("list error");
    }
}

void Server::handle_send(char buffer[1024], int current_socket, long size, std::string &directory, FILE *fptr,
                         std::string username) {
    std::string receiverUsername;
    std::string subject;
    std::string msg;

    bzero(buffer, BUF);

    std::string userDirectoryPath = directory;
    std::string filePath;

    bool isSuccessful = persist_message_from_send(buffer, current_socket, size, fptr, username, receiverUsername,
                                                  subject, msg,
                                                  userDirectoryPath,
                                                  filePath);


    bzero(buffer, BUF);

    if (isSuccessful) {
        printf("\n\nEverything went well!\n");
        buffer = (char *) "OK\n";
    } else {
        printf("\n\nThere was an error!\n");
        buffer = (char *) "ERR\n";
    }

    if (send(current_socket, buffer, BUF, 0) == -1) // send message to client
    {
        perror("send error");
    }
}

bool
Server::persist_message_from_send(char buffer[1024], int current_socket, long size, FILE *fptr,
                                  std::string &username,
                                  std::string &receiverUsername, std::string &subject, std::string &msg,
                                  std::string &userDirectoryPath, std::string &filePath) {
    // Read the lines first
    if (read_send_lines(buffer, current_socket, size, username, receiverUsername, subject, msg)) {

        userDirectoryPath.append("/");
        userDirectoryPath.append(username);

        // make directory
        if (mkdir(userDirectoryPath.c_str(), 0777) == -1) {
            if (errno == EEXIST) // ignore error if directory already exists
            {
            } else {
                return false;
            }
        }


        filePath = userDirectoryPath;
        filePath.append("/");
        std::string msg_id = gen_random(MSG_ID_LENGTH);
        filePath.append(msg_id);

        // create file
        fptr = fopen(filePath.c_str(), "w");
        if (fptr == nullptr) {
            printf("Unable to create file.\n");
            return false;
        } else {
            username.append("\n");
            subject.append("\n");

            fputs(username.c_str(), fptr);
            fputs(receiverUsername.c_str(), fptr);
            fputs(subject.c_str(), fptr);
        }

        // write message into file
        fputs(msg.c_str(), fptr);
        fclose(fptr);
        return true;
    } else {
        return false;
    }
}

bool
Server::read_send_lines(char buffer[1024], int current_socket, long size, std::string &username,
                        std::string &receiverUsername, std::string &subject, std::string &msg) {



    if (read_send_line(buffer, current_socket, size)) {
        // 0 -- receiverUsername
        receiverUsername = buffer;
    } else {
        return false;
    }


    if (read_send_line(buffer, current_socket, size)) {
        // 1 -- subject
        buffer[strlen(buffer) - 1] = '\0';
        subject = buffer;
    } else {
        return false;
    }

    if (read_send_line(buffer, current_socket, size)) {
        // 2 -- msg
        msg = buffer;
    } else {
        return false;
    }


    return true;
}

bool Server::read_send_line(char buffer[1024], int current_socket, long size) {
    size = recv(current_socket, buffer, BUF, 0);
    if (size <= 0) {
        perror("recv error");
        return false;
    }
    return true;
}