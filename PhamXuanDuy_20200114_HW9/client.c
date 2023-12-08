#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/aes.h>

#define MAX_MESSAGE_SIZE 1024
#define AES_KEY_SIZE 128
#define AES_BLOCK_SIZE 16

unsigned char aes_key[AES_KEY_SIZE / 8];

// Encrypt a message using AES-128
char *encrypt_message(const char *input);
// Decrypt a message using AES-128
void decrypt_message(const char *encrypted_input, char **decrypted_output);
// Thread function for receiving messages from the server
void *receive_messages(void *arg);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./client ServerIP PortNumber\n");
        exit(EXIT_FAILURE);
    }
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_socket;
    struct sockaddr_in server_addr;
    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    // Set up server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    char buffer[MAX_MESSAGE_SIZE];
    int read_size = recv(client_socket, buffer, sizeof(buffer), 0);
    if (read_size > 0)
    {
        buffer[read_size] = '\0'; // Null-terminate the received data
        memcpy(aes_key, buffer, AES_KEY_SIZE / 8);
        // printf("AES key received from server: %s\n", aes_key);
    }
    // Create a thread to receive messages from the server
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, receive_messages, (void *)&client_socket);
    char message[MAX_MESSAGE_SIZE];
    while (1)
    {
        // Send the message to the server
        fgets(message, sizeof(message), stdin);
        char *encrypted_message = encrypt_message(message);
        send(client_socket, encrypted_message, strlen(encrypted_message), 0);
        free(encrypted_message);
    }
    close(client_socket);
    return 0;
}

char *encrypt_message(const char *input)
{
    AES_KEY aes;
    AES_set_encrypt_key(aes_key, AES_KEY_SIZE, &aes);
    size_t input_length = strlen(input);
    int blocks = (input_length + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    size_t output_length = blocks * AES_BLOCK_SIZE;
    unsigned char *encrypted_buffer = (unsigned char *)malloc(output_length);
    if (!encrypted_buffer)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    // Pad the input if necessary
    size_t padded_length = blocks * AES_BLOCK_SIZE;
    unsigned char *padded_input = (unsigned char *)malloc(padded_length);
    if (!padded_input)
    {
        free(encrypted_buffer);
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    memcpy(padded_input, input, input_length);
    memset(padded_input + input_length, AES_BLOCK_SIZE - input_length % AES_BLOCK_SIZE, AES_BLOCK_SIZE - input_length % AES_BLOCK_SIZE);
    // Encrypt each block
    for (int i = 0; i < blocks; i++)
    {
        AES_encrypt(padded_input + i * AES_BLOCK_SIZE, encrypted_buffer + i * AES_BLOCK_SIZE, &aes);
    }
    free(padded_input);
    return (char *)encrypted_buffer;
}

void decrypt_message(const char *encrypted_input, char **decrypted_output)
{
    AES_KEY aes;
    AES_set_decrypt_key(aes_key, AES_KEY_SIZE, &aes);
    size_t input_length = strlen(encrypted_input);
    int blocks = input_length / AES_BLOCK_SIZE;
    size_t output_length = blocks * AES_BLOCK_SIZE;

    unsigned char *decrypted_buffer = (unsigned char *)malloc(output_length);
    if (!decrypted_buffer)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    // Giải mã từng khối dữ liệu
    for (int i = 0; i < blocks; i++)
    {
        AES_decrypt((const unsigned char *)(encrypted_input + i * AES_BLOCK_SIZE),
                    decrypted_buffer + i * AES_BLOCK_SIZE, &aes);
    }
    // Loại bỏ phần padding
    size_t unpadded_length = output_length - decrypted_buffer[output_length - 1];
    *decrypted_output = (char *)malloc(unpadded_length + 1);
    if (!(*decrypted_output))
    {
        free(decrypted_buffer);
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    memcpy(*decrypted_output, decrypted_buffer, unpadded_length);
    (*decrypted_output)[unpadded_length] = '\0';
    free(decrypted_buffer);
}

void *receive_messages(void *arg)
{
    int client_socket = *((int *)arg);
    char buffer[MAX_MESSAGE_SIZE];
    while (1)
    {
        // Receive messages from the server
        int read_size = recv(client_socket, buffer, sizeof(buffer), 0);
        if (read_size > 0)
        {
            buffer[read_size] = '\0'; // Null-terminate the received data
            char *decrypted_message;
            decrypt_message(buffer, &decrypted_message);
            printf("--> %s\n", decrypted_message);
            free(decrypted_message);
        }
        else if (read_size == 0)
        {
            // Server disconnected
            printf("Server disconnected\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("Error receiving message");
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}
