#include "client.h"

Client::Client() {}

// create client socket
bool Client::start()
{
    // create socket
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket error");
        return false;
    }

    // IP and port allocation
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    inet_aton("127.0.0.1", &address.sin_addr); // convert to binary

    return true;
}

// connect to server and communicate
void Client::connectServer()
{
    // connect client socket to server socket
    if (connect(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
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
        if (isQuit == true)
        {
            break;
        }
        size = recv(create_socket, buffer, BUF, 0); // receive message from server
        if (size == -1)
        {
            perror("recv error");
            break;
        }
        else if (size == 0)
        {
            printf("Server closed remote socket\n");
            break;
        }
        else
        {
            printf("\nMessage received: \n%s\n", buffer);
        }
    } while (!isQuit);

    // frees the descriptor
    if (create_socket != -1)
    {
        if (shutdown(create_socket, SHUT_RDWR) == -1)
        {
            perror("shutdown create_socket");
        }
        if (close(create_socket) == -1)
        {
            perror("close create_socket");
        }
        create_socket = -1;
    }
}

// handles the mailer functions (SEND, ...)
void Client::mailer()
{
    printf("--------------------\n[Commands]\n SEND\n LIST\n READ\n DEL\n QUIT\n--------------------\n");

    bool input = false;
    while (!input) // while no accepted input by client
    {
        bzero(buffer, BUF);        // clear buffer
        fgets(buffer, BUF, stdin); // get client input
        input = true;

        if (strncmp(buffer, "SEND", 4) == 0)
        {
            if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'SEND' to server
            {
                perror("send error");
            }
            for (int i = 0; i < 4; i++) // send all parts of send
            {
                bzero(buffer, BUF); // clear buffer
                if (i == 0 || i == 1)
                {
                    std::string str;
                    bool valid_user = true;
                    do
                    {
                        valid_user = true;
                        fgets(buffer, BUF, stdin); // only 8 characters
                        str = buffer;
                        if (str.find_first_not_of("abcdefghijklmnopqrstuvwxyz" // username is supposed to be only a-z, 0-9
                                                  "0123456789"
                                                  "\n") != std::string::npos ||
                            strlen(buffer) > 9) // 8 + \n
                        {
                            valid_user = false;
                            printf("Please enter valid username (a-z, 0-9)!\n");
                        }
                    } while (!valid_user);
                }
                else if (i == 2) // subject max 80
                {
                    fgets(buffer, 80, stdin);
                }
                else if (i == 3) // message ends with dot
                {
                    std::cin.getline(buffer, BUF, '.');
                    std::cin.ignore(); // ignore /n after dot
                }
                else
                {
                    fgets(buffer, BUF, stdin);
                }
                if ((send(create_socket, buffer, sizeof(buffer), 0)) == -1) // send to server
                {
                    perror("send error");
                }
            }
            bzero(buffer, BUF);
            return;
        }
        else if (strncmp(buffer, "LIST", 4) == 0)
        {
            if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'LIST' to server
            {
                perror("send error");
            }

            bzero(buffer, BUF);
            std::string str;
            bool valid_user = true;

            do
            {
                valid_user = true;
                fgets(buffer, BUF, stdin);
                str = buffer;
                if (str.find_first_not_of("abcdefghijklmnopqrstuvwxyz" // username is supposed to be only a-z, 0-9
                                          "0123456789"
                                          "\n") != std::string::npos ||
                    strlen(buffer) > 9) // 8 + \n
                {
                    valid_user = false;
                    printf("Please enter valid username (a-z, 0-9)!\n");
                }
            } while (!valid_user);
            // fgets(buffer, BUF, stdin);                                  // get username from client input
            if ((send(create_socket, buffer, sizeof(buffer), 0)) == -1) // send username to server
            {
                perror("send error");
            }

            return;
        }
        else if (strncmp(buffer, "READ", 4) == 0)
        {
            if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'READ' to server
            {
                perror("send error");
            }
            for (int i = 0; i < 2; i++) // send all parts of read
            {
                bzero(buffer, BUF);
                if (i == 0)
                {
                    std::string str;
                    bool valid_user = true;
                    do
                    {
                        valid_user = true;
                        fgets(buffer, BUF, stdin);
                        str = buffer;
                        if (str.find_first_not_of("abcdefghijklmnopqrstuvwxyz" // username is supposed to be only a-z, 0-9
                                                  "0123456789"
                                                  "\n") != std::string::npos ||
                            strlen(buffer) > 9) // 8 + \n
                        {
                            valid_user = false;
                            printf("Please enter valid username (a-z, 0-9)!\n");
                        }
                    } while (!valid_user);
                }
                else
                {
                    fgets(buffer, BUF, stdin);
                }

                if ((send(create_socket, buffer, sizeof(buffer), 0)) == -1)
                {
                    perror("send error");
                }
            }
            return;
        }
        else if (strncmp(buffer, "DEL", 3) == 0)
        {
            if ((send(create_socket, buffer, BUF, 0)) == -1) // send 'DEL' to server
            {
                perror("send error");
            }
            for (int i = 0; i < 2; i++) // send all parts of del
            {
                bzero(buffer, BUF);
                if (i == 0)
                {
                    std::string str;
                    bool valid_user = true;
                    do
                    {
                        valid_user = true;
                        fgets(buffer, BUF, stdin);
                        str = buffer;
                        if (str.find_first_not_of("abcdefghijklmnopqrstuvwxyz" // username is supposed to be only a-z, 0-9
                                                  "0123456789"
                                                  "\n") != std::string::npos ||
                            strlen(buffer) > 9) // 8 + \n
                        {
                            valid_user = false;
                            printf("Please enter valid username (a-z, 0-9)!\n");
                        }
                    } while (!valid_user);
                }
                else
                {
                    fgets(buffer, BUF, stdin);
                }
                if ((send(create_socket, buffer, sizeof(buffer), 0)) == -1)
                {
                    perror("send error");
                }
            }
            return;
        }
        else if (strncmp(buffer, "QUIT", 4) == 0) // no server respond
        {
            isQuit = true;
            return;
        }
        else
        {
            input = false;
        }
    }
    return;
}
