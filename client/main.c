#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3357
#define MAX_MESSAGE_SIZE 1024

void *receive(void *arg) {
    int sock = *((int *)arg);
    char buffer[MAX_MESSAGE_SIZE];
    
    while (1) {
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            perror("Error receiving data");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
    }
    
    return NULL;
}

int main() {

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Socket creation failed");
        return 1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    if (connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        return 1;
    }
    
    char username[5];
    printf("\nEnter a username of four letters: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';
    
    while (strlen(username) != 4) {
        printf("Please enter a valid username of four letters: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0';
    }
    
    send(s, username, strlen(username), 0);
    
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive, &s) != 0) {
        perror("Thread creation failed");
        return 1;
    }
    
    while (1) {
        char message[MAX_MESSAGE_SIZE];
        printf("You: ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';
        
        char full_message[MAX_MESSAGE_SIZE + 20];
        snprintf(full_message, sizeof(full_message), "%s said: %s", username, message);
        
        send(s, full_message, strlen(full_message), 0);
    }
    
    close(s);
    
    return 0;
}