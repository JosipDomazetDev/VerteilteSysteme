
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>

#define DATA_SIZE 1000


int main(int argc, char *argv[]) {
    // Josip Domazet & Reynaud Cade

    char buffer[1024];
    char username[1024];
    char title[1024];
    char message[1024];

    int socket_server, newsockfd, port, n;
    struct sockaddr_in serv_addr, client_addr;

    socklen_t len;

    //check for appropriate no. of command line inputs

    if (argc < 2) {
        printf("provide required commend-line inputs as c> <server_ip> <port>");
        exit(1);
    }

    port = atoi(argv[1]);
    socket_server = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_server < 0) {
        printf("Error in opening socket");
        exit(1);
    }

    //build the structure for server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    //Bind the socket to the address and port number specified in structure

    if (bind(socket_server, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR in binding socket.");
    }

    //Listen to any incoming client
    listen(socket_server, 5);

    len = sizeof(client_addr);

    newsockfd = accept(socket_server, (struct sockaddr *) &client_addr, &len);

    //check if the client socket is created properly

    if (newsockfd < 0) {
        printf("ERROR in accepting request");
        exit(1);
    }

    //-----------------------------------------------Server and Client Connected --- Chat Started--------------------------------------------

    while (1) {
        bzero(buffer, 1024); //clear buffer
        n = read(newsockfd, buffer, 1024);
        if (n < 0) {
            printf("ERROR in reading");
            exit(1);
        }

        //print client request data..
        printf("\nClient =>%s", buffer);
        //-------------------------------------------------SEND (CREATE) FUNCTION-----------------------------------------
        if (!strncmp("SEND", buffer, 4)) {
            for (int i = 0; i < 4; i++) {
                if (i == 0) {
                    printf("You are in READ mode 1 step. Wait until client give the nessecary infos.\n");
                    bzero(buffer, 1024); //clear buffer again
                    printf("[SERVER]:");
                    strcpy(buffer, "Who are you?");
                    n = write(newsockfd, buffer, 1024);
                }
                if (i == 1) {
                    printf("You are in send mode 2 step.\n");
                    bzero(buffer, 1024); //clear buffer
                    n = read(newsockfd, buffer, 1024);
                    printf("\nClient =>%s", buffer);
                    strcpy(username, buffer);
                    username[strlen(username) - 1] = '\0';
                    printf("Username: %s", username);
                    strcpy(buffer, "What is the title?");
                    n = write(newsockfd, buffer, 1024);
                }
                if (i == 2) {
                    printf("You are in send mode 3 step.\n");
                    bzero(buffer, 1024); //clear buffer
                    n = read(newsockfd, buffer, 1024);
                    printf("\nClient =>%s", buffer);
                    strcpy(title, buffer);
                    printf("Title: %s", title);
                    strcpy(buffer, "What is the message ?");
                    n = write(newsockfd, buffer, 1024);
                    printf("You are in send mode 3 step.\n");
                    bzero(buffer, 1024); //clear buffer
                    n = read(newsockfd, buffer, 1024);
                    printf("\nClient =>%s", buffer);
                    strcpy(message, buffer);
                    printf("Message: %s", message);
                }
                if (i == 3) {
                    const char *name = username;
                    errno = 0;
                    int ret = mkdir(name, S_IRWXU);
                    if (ret == -1) {
                        switch (errno) {
                            case EACCES:
                                printf("the parent directory does not allow write\n	");
                                break;
                            case EEXIST:
                                printf("pathname already exists\n");
                                break;
                            case ENAMETOOLONG:
                                printf("pathname is too long\n");
                                break;
                            default:
                                perror("mkdir");
                                break;
                        }
                    }
                    char *filebuffer;
                    char command[512];
                    char data[DATA_SIZE];
                    filebuffer = malloc(strlen(title) + 50);
                    strcpy(filebuffer, username);
                    strcat(filebuffer, "/");
                    strcat(filebuffer, title);
                    filebuffer[strlen(filebuffer) - 1] = '\0';
                    printf("Filebuffer: %s", filebuffer);
                    FILE *fPtr;
                    fPtr = fopen(filebuffer, "w");
                    if (fPtr == NULL) {
                        printf("Unable to create file.\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("Enter contents to store in file : \n");
                    strcpy(data, message);
                    fputs(data, fPtr);
                    n = write(newsockfd, buffer, 1024);
                    fclose(fPtr);
                    printf("File created and saved successfully. :) \n");
                }
            }
        }
        //-------------------------------------------------READ FUNCTION-----------------------------------------
        if (!strncmp("READ", buffer, 4)) {
            for (int i = 0; i < 4; i++) {
                if (i == 0) {
                    printf("You are in READ mode 1 step. Wait until client give the nessecary infos.\n");
                    bzero(buffer, 1024); //clear buffer again
                    printf("[SERVER]:");
                    strcpy(buffer, "Who are you?");
                    n = write(newsockfd, buffer, 1024);
                }
                if (i == 1) {
                    printf("You are in send mode 2 step.\n");
                    bzero(buffer, 1024); //clear buffer
                    n = read(newsockfd, buffer, 1024);
                    printf("\nClient =>%s", buffer);
                    strcpy(username, buffer);
                    printf("Username: %s", username);
                    strcpy(buffer, "What is the title?");
                    n = write(newsockfd, buffer, 1024);
                }
                if (i == 2) {
                    printf("You are in send mode 3 step.\n");
                    bzero(buffer, 1024); //clear buffer
                    n = read(newsockfd, buffer, 1024);
                    printf("\nClient =>%s", buffer);
                    strcpy(title, buffer);
                    printf("Title: %s", title);
                }

                if (i == 3) {
                    char *filebuffer;
                    char command[512];
                    char data[DATA_SIZE];
                    char *filename = username;
                    filebuffer = malloc(strlen(title) + 50);
                    strcpy(filebuffer, username);
                    filebuffer[strlen(filebuffer) - 1] = '\0';
                    strcat(filebuffer, "/");
                    strcat(filebuffer, title);
                    filebuffer[strlen(filebuffer) - 1] = '\0';
                    printf("Filebuffer: %s", filebuffer);
                    FILE *fPtr;
                    fPtr = fopen(filebuffer, "r");
                    char messageContent[2000];
                    if (fPtr == NULL)
                        printf("Error on finding the file.\n");
                    else {
                        while (fscanf(fPtr, "%s", messageContent) != EOF) {
                            printf("%s\n", messageContent);
                            strcat(buffer, messageContent);
                            strcat(buffer, " ");
                        }
                        n = write(newsockfd, buffer, 1024);
                        fclose(fPtr);
                    }
                }
            }
        }

        //-------------------------------------------------LIST FUNCTION-----------------------------------------
        if (!strncmp("LIST", buffer, 4)) {

            printf("You are in LIST mode.\n");

            //Get username
            bzero(buffer, 1024); //clear buffer again
            printf("[SERVER]:");
            strcpy(buffer, "Who are you?");
            n = write(newsockfd, buffer, 1024);

            bzero(buffer, 1024); //clear buffer
            n = read(newsockfd, buffer, 1024);
            printf("\nClient =>%s", buffer);
            buffer[strcspn(buffer, "\r\n")] = 0; // \r\n von buffer entfernen
            strcpy(username, buffer);
            printf("Username: %s", username);

            //Open directory and get files
            DIR *d;
            struct dirent *dir;
            d = opendir(username);
            int count = 0;
            char erg[1024];
            if (d) {
                sprintf(buffer, "0 Messages sent\n");
                while ((dir = readdir(d)) != NULL) {
                    if (dir->d_type == DT_REG) {
                        sprintf(buffer + strlen(buffer), "\n%s", dir->d_name);
                        count++;
                        printf("%s", dir->d_name);
                    }
                }

                n = write(newsockfd, buffer, 1024);
                closedir(d);
            } else //Error message, if 0 messages or user not found
            {
                bzero(buffer, 1024); //clear buffer again
                strcpy(buffer, "0 Messages sent or user unknown.\n");
                n = write(newsockfd, buffer, 1024);
            }
        }
        //-------------------------------------------------DEL FUNCTION-----------------------------------------
        if (!strncmp("DEL", buffer, 3)) {
            printf("You are in DEL mode.\n");

            //Get username
            bzero(buffer, 1024); //clear buffer again
            printf("[SERVER]:");
            strcpy(buffer, "Who are you?");
            n = write(newsockfd, buffer, 1024);
            bzero(buffer, 1024); //clear buffer
            n = read(newsockfd, buffer, 1024);
            printf("\nClient =>%s", buffer);
            buffer[strcspn(buffer, "\r\n")] = 0; // \r\n von buffer entfernen
            strcpy(username, buffer);

            //Get message which should be deleted
            bzero(buffer, 1024); //clear buffer again
            printf("[SERVER]:");
            strcpy(buffer, "Please enter the subject of the message you want to delete.");
            n = write(newsockfd, buffer, 1024);
            bzero(buffer, 1024); //clear buffer
            n = read(newsockfd, buffer, 1024);
            printf("\nClient =>%s", buffer);

            //Open dir and get files
            DIR *d;
            struct dirent *dir;
            d = opendir(username);
            char erg[1024];
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                    if (!strncmp(dir->d_name, buffer, dir->d_name)) {
                        sprintf(erg, "%s/%s", username, dir->d_name);
                        printf("FILE: %s\n", erg);
                        remove(erg);
                    }
                }
                closedir(d);
            } else {
                bzero(buffer, 1024); //clear buffer again
                strcpy(buffer, "Message couldn't be found.");
                n = write(newsockfd, buffer, 1024);
            }
        }

        //-------------------------------------------------QUIT FUNCTION-----------------------------------------
        if (!strncmp("QUIT", buffer, 4)) {
            bzero(buffer, 1024); //clear buffer again
            printf("[SERVER]:");
            strcpy(buffer, "QUIT");
            n = write(newsockfd, buffer, 1024);
            break;
        }
    }

    close(newsockfd);
    close(socket_server);

    return 0;
}
