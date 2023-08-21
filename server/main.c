#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

struct ClientNode {
    int socket;
    struct ClientNode* next;
};

typedef struct ClientNode ClientNode;

ClientNode* addClient(ClientNode* head, int socket) {
    ClientNode* newNode = (ClientNode*)malloc(sizeof(ClientNode));
    if (newNode == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    newNode->socket = socket;
    newNode->next = head;
    return newNode;
}

ClientNode* removeClient(ClientNode* head, int socket) {
    ClientNode* current = head;
    ClientNode* prev = NULL;

    while (current != NULL) {
        if (current->socket == socket) {
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }

    return head;
}

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    ClientNode* clients = NULL;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(3357);

    printf("\nInitializing server...\n");
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("\nListening for connections...\n");

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        char* client_ip = inet_ntoa(address.sin_addr);
        printf("Client connected from IP address: %s\n", client_ip);

        clients = addClient(clients, new_socket);

        char buffer[1024] = {0};
        while (1) {
            valread = recv(new_socket, buffer, sizeof(buffer), 0);
            if (valread <= 0) {
                clients = removeClient(clients, new_socket);
                close(new_socket);
                break;
            }

            printf("Received: %s\n", buffer);

            ClientNode* current = clients;
            while (current != NULL) {
                if (current->socket != new_socket) {
                    send(current->socket, buffer, strlen(buffer), 0);
                }
                current = current->next;
            }
        }
    }


    ClientNode* current = clients;
    while (current != NULL) {
        ClientNode* temp = current;
        current = current->next;
        close(temp->socket);
        free(temp);
    }

    return 0;
}