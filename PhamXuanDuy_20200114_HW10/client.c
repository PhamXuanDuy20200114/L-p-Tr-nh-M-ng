#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include<stdlib.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 1024
#define MES "Insert password:"

char buff[BUFF_SIZE];
int bytes_received;


void sendMes(int x, char *mes){
	int bytes_sent = send(x, mes, strlen(mes), 0);
	if(bytes_sent <= 0){
		printf("\nConnection closed!\n");
		return;
	}
}

void sendText(int client_sock){
	int msg_len;
	sendMes(client_sock,"Choice: 1");
	bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
	if(bytes_received <= 0){
		printf("\nError!Cannot receive data from sever!\n");
		return;
	}
		
	buff[bytes_received] = '\0';
	printf("%s", buff);
	fflush(stdin);
	scanf(" %[^\n]s", buff);	
	msg_len = strlen(buff);
	if (msg_len == 0) return;
		
	sendMes(client_sock,buff);

	bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
	if(bytes_received <= 0){
		printf("\nError!Cannot receive data from sever!\n");
		return;
	}
		
	buff[bytes_received] = '\0';
	printf("\nMessage: %s\n", buff);

}

void sendImg(int client_sock){
	sendMes(client_sock,"Choice: 2");
	bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
	if(bytes_received <= 0){
		printf("\nError!Cannot receive data from sever!\n");
		return;
		}
		
	buff[bytes_received] = '\0';
	printf("%s\n", buff);
	char *tenFile;
	scanf("%s",tenFile);
	FILE *file;
	file = fopen(tenFile, "rb");
	if (!file) {
        perror("Lỗi mở file ảnh");
       	return;
    }
	while (!feof(file)) {
       	size_t bytesRead = fread(buff, 1, sizeof(buff), file);
        if (send(client_sock, buff, bytesRead, 0) < 0) {
            perror("Lỗi gửi dữ liệu");
            break;
        }
        bzero(buff, BUFF_SIZE);
	}
	fclose(file);
	printf("END1");
}

int main(int argc, char *argv[]){
	char *check;
	if(argc != 3){
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }

	int client_sock;
	struct sockaddr_in server_addr; /* server's address information */
	
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
	printf("Insert Username: ");
	memset(buff,'\0',(strlen(buff)+1));
	fgets(buff, BUFF_SIZE, stdin);		
	sendMes(client_sock, buff);

	memset(buff,'\0',(strlen(buff)+1));
	bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
	if(bytes_received <= 0){
		printf("\nError!Cannot receive data from sever!\n");
		return 0;
	}
	buff[bytes_received] = '\0';
	check = strdup(buff);
	printf("%s",check);
	while(strcmp(check, "Insert password: ") == 0){
		memset(buff,'\0',(strlen(buff)+1));
		fgets(buff, BUFF_SIZE, stdin);		
		sendMes(client_sock, buff);
		
		memset(buff,'\0',(strlen(buff)+1));
		bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
		if(bytes_received <= 0){
			printf("\nError!Cannot receive data from sever!\n");
			return 0;
		}
		buff[bytes_received] = '\0';
		check = strdup(buff);
		printf("%s\n",check);
	}

	if(strcmp(check, "Login success!") == 0){
		while(1){
			printf("Menu\n------------\n1. Gửi 1 tin nhắn\n2. Gửi 1 file\n3. Đăng xuất\n");
			int choice;
			scanf("%d", &choice);
			getchar();
			printf("%d\n",choice);
			switch(choice){
				case 1:
				sendText(client_sock);
				break;
				case 2:
				sendImg(client_sock);
				break;
				case 3:
				sendMes(client_sock, "logout");
				break;
			}
		}
	}
	//Step 4: Close socket
	close(client_sock);
	return 0;
}
