/*UDP Echo Client*/
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>

// #define SERV_PORT 5550
// #define SERV_IP "127.0.0.1"
#define BUFF_SIZE 1024
#define MES3 "Insert password:"

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }

	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent,bytes_received;
	socklen_t sin_size = sizeof(struct sockaddr);
	
	//Step 1: Construct a UDP socket
	if ((client_sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	//Step 2: Define the address of the server
    int portNumber = atoi(argv[2]);
    char *ipAdrr = argv[1];
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portNumber);
	server_addr.sin_addr.s_addr = inet_addr(ipAdrr);
	
	//Step 3: Communicate with server
	while(1){
		printf("\nUser Name:");
		memset(buff,'\0',(strlen(buff)+1));
		fgets(buff, BUFF_SIZE, stdin);
	
		bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
		bytes_received = recvfrom(client_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &server_addr, &sin_size);
		if(bytes_received < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		buff[bytes_received]= '\0';
		char *check = strdup(buff);
		printf("%s ",check);
		while(strcmp(check, MES3) == 0){
			memset(buff,'\0',(strlen(buff)+1));
			fgets(buff, BUFF_SIZE, stdin);
			bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
			bytes_received = recvfrom(client_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &server_addr, &sin_size);
			if(bytes_received < 0){
				perror("Error: ");
				close(client_sock);
				return 0;
			}
			buff[bytes_received]= '\0';
			char *check1 = strdup(buff);
			strcpy(check,check1);
			printf("%s\n",check);
		}
		if(strcmp(check,"Login success!") == 0){
			memset(buff,'\0',(strlen(buff)+1));
			fgets(buff, BUFF_SIZE, stdin);
			bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
			bytes_received = recvfrom(client_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &server_addr, &sin_size);
			if(bytes_received < 0){
				perror("Error: ");
				close(client_sock);
				return 0;
			}
			buff[bytes_received]= '\0';
			char *noti = strdup(buff);
			printf("%s",noti);
		}
	}	
	close(client_sock);
	return 0;
}
