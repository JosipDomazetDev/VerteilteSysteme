#include "client.h"

Client::Client() = default;

// create client socket
bool Client::start() {
    // create socket
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket error");
        return false;
    }

    // IP and port allocation
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    inet_aton("127.0.0.1", &address.sin_addr); // convert to binary

    return true;
}

// connect to server and communicate
void Client::connectServer() {
    // connect client socket to server socket
    if (connect(create_socket, (struct sockaddr *) &address, sizeof(address)) == -1) {
        perror("Connect error - no server available");
        exit(EXIT_FAILURE);
    }

    printf("Connection with server (%s) established\n",
           inet_ntoa(address.sin_addr));

    do // do until client quits
    {
        mailer(); // mailer functions
        printf("-------------------- \n");
        bzero(buffer, BUF);
        if (isQuit) {
            break;
        }
        size = recv(create_socket, buffer, BUF, 0); // receive message from server
        if (size == -1) {
            perror("recv error");
            break;
        } else if (size == 0) {
            printf("Server closed remote socket\n");
            break;
        } else {
            printf("\nMessage received: \n%s\n", buffer);
        }
    } while (true);

    // frees the descriptor
    if (create_socket != -1) {
        if (shutdown(create_socket, SHUT_RDWR) == -1) {
            perror("shutdown create_socket");
        }
        if (close(create_socket) == -1) {
            perror("close create_socket");
        }
        create_socket = -1;
    }
}

// handles the mailer functions (SEND, ...)
void Client::mailer() {
    printf("--------------------\n[Commands]\n LOGIN\n SEND\n LIST\n READ\n DEL\n QUIT\n--------------------\n");

    while (true) // while no accepted input by client
    {
        bzero(buffer, BUF);        // clear buffer
        fgets(buffer, BUF, stdin); // get client input

        if (strncmp(buffer, "LOGIN", 5) == 0) {
            handle_login();
            return;
        }
        if (strncmp(buffer, "SEND", 4) == 0) {
            handle_send();
            return;
        } else if (strncmp(buffer, "LIST", 4) == 0) {
            handle_list();
            return;
        } else if (strncmp(buffer, "READ", 4) == 0) {
            handle_read();
            return;
        } else if (strncmp(buffer, "DEL", 3) == 0) {
            handle_del();
            return;
        } else if (strncmp(buffer, "QUIT", 4) == 0) // no server respond
        {
            isQuit = true;
            return;
        }
    }
}


bool isValidUsername(const std::string &str) {
    std::string criteria("abcdefghijklmnopqrstuvwxyz0123456789\n");
    // only 8 characters
    return (std::string::npos == str.find_first_not_of(criteria)) && str.length() <= 9;
}

void Client::extractUsername() {
    std::string username;
    bool is_valid_username;

    do {
        fgets(buffer, BUF, stdin);
        username = buffer;
        is_valid_username = isValidUsername(username);

        if (!is_valid_username) {
            printf("Please enter valid username (a-z, 0-9) (<=9 characters)!\n");
        }
    } while (!is_valid_username);
}

void Client::sendBuffer() const {
    if ((send(create_socket, buffer, sizeof(buffer), 0)) == -1) {
        perror("send error");
    }
}


void Client::handle_read() {
    if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'READ' to server
    {
        perror("send error");
    }
    for (int i = 0; i < 2; i++) // send all parts of read
    {
        bzero(buffer, BUF);
        if (i == 0) {
            extractUsername();
        } else {
            // message-id
            fgets(buffer, BUF, stdin);
        }

        sendBuffer();
    }
}


void Client::handle_del() {
    if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'DEL' to server
    {
        perror("send error");
    }
    for (int i = 0; i < 2; i++) // send all parts of del
    {
        bzero(buffer, BUF);
        if (i == 0) {
            extractUsername();
        } else {
            // message-id
            fgets(buffer, BUF, stdin);
        }

        sendBuffer();
    }
}

void Client::handle_list() {
    if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'LIST' to server
    {
        perror("send error");
    }
    bzero(buffer, BUF);

    extractUsername();
    sendBuffer();
}

void Client::handle_login() {
    if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'LOGIN' to server
    {
        perror("send error");
    }
    for (int i = 0; i < 2; i++) // send all parts of login
    {
        bzero(buffer, BUF); // clear buffer
        if (i == 0) {
            // username & recipient
            extractUsername();
        } else {
            // password
            fgets(buffer, BUF, stdin);
        }

        sendBuffer();
    }
    bzero(buffer, BUF);
}

void Client::handle_send() {
    if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'SEND' to server
    {
        perror("send error");
    }
    for (int i = 0; i < 4; i++) // send all parts of send
    {
        bzero(buffer, BUF); // clear buffer
        if (i == 0 || i == 1) {
            // username & recipient
            extractUsername();
        } else if (i == 2) {
            // subject max 80
            fgets(buffer, 80, stdin);
        } else if (i == 3) {
            // message ends with dot
            std::cin.getline(buffer, BUF, '.');
            std::cin.ignore(); // ignore /n after dot
        }


        sendBuffer();
    }
    bzero(buffer, BUF);
}
