#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#define MAX_CLIENTS 5
#define AES_KEY_SIZE 128
#define AES_BLOCK_SIZE 16

pthread_mutex_t lock;
int client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_lock;

unsigned char aes_key[AES_KEY_SIZE / 8];

void *handle_client(void *args); 

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./server PortNumber\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
	int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;
    RAND_bytes(aes_key, sizeof(aes_key)); // Generate a random AES key
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&client_lock, NULL);
    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    // Set up server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }
    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }
    printf("\nServer is listening on port %d\n", port);
    while (1)
    {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0)
        {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }
        // Create a new thread for the client
        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        pthread_create(&tid, NULL, handle_client, client_socket_ptr);
        pthread_detach(tid);
    }
    close(server_socket);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&client_lock);
    return 0;
}



void *handle_client(void *args)
{
    int client_socket = *((int *)args);
    free(args);
    // Lock the mutex to safely update the client_sockets array
    pthread_mutex_lock(&client_lock);
    if (client_count < MAX_CLIENTS)
    {
        client_sockets[client_count++] = client_socket;
        pthread_mutex_lock(&lock); // Lock the mutex before sending the key
    	send(client_socket, aes_key, sizeof(aes_key), 0); // Send the AES key to the client
    	pthread_mutex_unlock(&lock); // Unlock the mutex after sending the key
    }
    pthread_mutex_unlock(&client_lock);
    while (1)
    {
        char buffer[1024];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            break;
        }
        printf("Received message from client %d: %s\n", client_socket, buffer);
        // Broadcast the received message to all other clients
        pthread_mutex_lock(&client_lock); // Lock the mutex before broadcasting the message
    	for (int i = 0; i < client_count; i++)
    	{
        	if (client_sockets[i] != client_socket)
        	{
        	   send(client_sockets[i], buffer, strlen(buffer), 0);
        	}
    	}
    pthread_mutex_unlock(&client_lock); // Unlock the mutex after broadcasting the message
    }
    close(client_socket);
    return NULL;
}

