#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include<stdlib.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <ctype.h>

#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 1024
#define FILENAME "./anhcopy.jpg"

char *md5Hashing(char *str)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, str, strlen(str));
    MD5_Final(digest, &md5);
    char *md5Hash = (char *)malloc(33);
    for (int i = 0; i < 16; i++)
        sprintf(&md5Hash[i * 2], "%02x", (unsigned int)digest[i]);
    return md5Hash;
}
char* digitInMD5(char* str){
    char* digit = (char*)malloc(BUFF_SIZE);
    int j = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            digit[j] = str[i];
            j++;
        }
    }
    digit[j] = '\0';
    return digit;
}

char* letterInMD5(char* str){
    char* letter = (char*)malloc(BUFF_SIZE);
    int j = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')){
            letter[j] = str[i];
            j++;
        }
    }
    letter[j] = '\0';
    return letter;
}

void sendMes(int x, char *mes){
	int bytes_sent = send(x, mes, strlen(mes), 0);
	if(bytes_sent <= 0){
		printf("\nConnection closed!\n");
		return;
	}
}

int main(int argc, char *argv[])
{
	if(argc != 2){
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }

	int listen_sock, conn_sock; /* file descriptors */
	char recv_data[BUFF_SIZE];
	int bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	
	//Step 1: Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("Error: \n");
		return 0;
	}
	
	//Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(atoi(argv[1]));   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ /* calls bind() */
		perror("Error: \n");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("Error: \n");
		return 0;
	}
	
	//Step 4: Communicate with client
	while(1){
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
			perror("\nError: ");
  
		printf("You got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
		
		//start conversation
		while(1){
			//receives message from client
			bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
			if (bytes_received <= 0){
				printf("Connection closed\n");
				break;
			}
			recv_data[bytes_received] = '\0';
			printf("%s \n", recv_data);
			char *check = strdup(recv_data);
			if(strcmp(check, "Choice: 1") == 0){
				sendMes(conn_sock, "Insert string: ");
				bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
				if (bytes_received <= 0){
					printf("Connection closed1\n");
					break;
				}
				recv_data[bytes_received] = '\0';
				printf("%s \n", recv_data);
				char *message = md5Hashing(strdup(recv_data));
				char *digit = digitInMD5(message);
				char *letter = letterInMD5(message);

				sendMes(conn_sock, digit);
				bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
				if (bytes_received <= 0){
					printf("Connection closed1\n");
					break;
				}
				recv_data[bytes_received] = '\0';
				sendMes(conn_sock, letter);
			}else{
				sendMes(conn_sock, "Nhập đường dẫn đến file: ");
				FILE *file;
				file = fopen(FILENAME, "wb");
				while ((bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0)) > 0) {
       				fwrite(recv_data, 1, bytes_received, file);
    			}
				fclose(file);
			}
		}
		close(conn_sock);	
	}
	
	close(listen_sock);
	return 0;
}
