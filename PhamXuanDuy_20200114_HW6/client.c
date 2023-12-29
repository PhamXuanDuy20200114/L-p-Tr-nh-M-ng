#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include<stdlib.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 8192

void sendMes(int x, char *mes){
	int bytes_sent = send(x, mes, strlen(mes), 0);
	if(bytes_sent <= 0){
		printf("\nConnection closed!\n");
		return;
	}
}

int main(int argc, char *argv[]){
	if(argc != 3){
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }

	int client_sock, bytes_received;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
	
	//Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server			
	while(1){
		while(1){
			printf("Menu\n------------\n1. Gửi 1 tin nhắn\n2. Gửi 1 file\n");
			int choice;
			scanf("%d", &choice);
			if(choice == 1){
				sendMes(client_sock,"Choice: 1");
				bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
				if(bytes_received <= 0){
					printf("\nError!Cannot receive data from sever!\n");
					break;
				}
		
				buff[bytes_received] = '\0';
				printf("%s", buff);
				fflush(stdin);
				scanf(" %[^\n]s", buff);	
				msg_len = strlen(buff);
				if (msg_len == 0) break;
		
				sendMes(client_sock,buff);

				bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
				if(bytes_received <= 0){
					printf("\nError!Cannot receive data from sever!\n");
					break;
				}
		
				buff[bytes_received] = '\0';
				printf("\nDigit: %s\n", buff);

				sendMes(client_sock,"x");

				bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
				if(bytes_received <= 0){
					printf("\nError!Cannot receive data from sever!\n");
					break;
				}

				buff[bytes_received] = '\0';
				printf("Char: %s\n", buff);

				// bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
				// if(bytes_received <= 0){
				// 	printf("\nError!Cannot receive data from sever!\n");
				// 	break;
				// }
		
				// buff[bytes_received] = '\0';
				// printf("Char: %s", buff);
			}
			else if(choice == 2){
				sendMes(client_sock,"Choice: 2");
				bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
				if(bytes_received <= 0){
					printf("\nError!Cannot receive data from sever!\n");
					break;
				}
		
				buff[bytes_received] = '\0';
				printf("%s\n", buff);
				char tenFile[BUFF_SIZE];
				scanf("%s", tenFile);
				FILE *file;
				file = fopen(tenFile, "rb");
				if (!file) {
        			perror("Lỗi mở file ảnh");
       				return -1;
    			}
				while (!feof(file)) {
       				size_t bytesRead = fread(buff, 1, sizeof(buff), file);
        			if (send(client_sock, buff, bytesRead, 0) < 0) {
            			perror("Lỗi gửi dữ liệu");
            			return -1;
        			}
        			bzero(buff, BUFF_SIZE);
				}
				fclose(file);
			}else break;
		}
	}
	//Step 4: Close socket
	close(client_sock);
	return 0;
}
