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
#include<pthread.h>

#define BUFF_SIZE 1024
#define BACKLOG 5   /* Number of allowed connections */
#define FileName "account.txt"
char recv_data[BUFF_SIZE];
int bytes_received;

pthread_mutex_t accounts_mutex = PTHREAD_MUTEX_INITIALIZER;

struct User{
	char userName[50];
	char password[50];
	int status;
	int loginStatus;
	struct User *next;
};

struct User *head = NULL;
struct User *current = NULL;

void readFile(){
	FILE *f;
	f = fopen(FileName,"r");
	if(f == NULL){
		printf("Loi mo file1\n");
	}

	while(!feof(f)){
		struct User *ptr = (struct User*) malloc(sizeof(struct User));
		fscanf(f,"%s %s %d\n",ptr->userName,ptr->password,&ptr->status); 
		ptr->loginStatus = 0;
		ptr->next = head;
		head = ptr;
	}
	fclose(f);
}

void writeFile(){
	FILE *f;
	f = fopen(FileName,"w");
	struct User *ptr = head;
	while(ptr != NULL){
		if(ptr->next == NULL){
			fprintf(f,"%s %s %d",ptr->userName,ptr->password,ptr->status); 
			break;
		}
		fprintf(f,"%s %s %d\n",ptr->userName,ptr->password,ptr->status); 
		ptr = ptr->next;
	}
	fclose(f);
}

void printList(){
	struct User *ptr = head;
	printf("\n[ ");
	while(ptr != NULL){
		printf("%s %s %d %d|| ", ptr->userName, ptr->password, ptr->status, ptr->loginStatus);
		ptr = ptr->next;
	}
	printf(" ]\n");
}

int searchUserName(char name[]){
	if(head == NULL){
		return 0;
	}
	struct User *link = head;
	while(link != NULL){
		if(strcmp(link->userName,name) == 0){
			current = link;
			return 1;
		}
		link = link->next;
	}
	return 0;
}

void sendMes(int x, char *mes){
	int bytes_sent = send(x, mes, strlen(mes), 0);
	if(bytes_sent <= 0){
		printf("\nConnection closed!\n");
		return;
	}
}

void *handleClient(void  *arg){
	int conn_sock = *(int *)arg;
	memset(recv_data,'\0',(strlen(recv_data)+1));
	bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0);
	if(bytes_received <= 0){
		printf("\nError!Cannot receive data from client!\n");
		return;
	}
	recv_data[bytes_received-1] = '\0';
	char *check = strdup(recv_data);
	
	int search = searchUserName(check);
	if(search == 1 && current->status == 1){
		if(current->loginStatus == 0){
			sendMes(conn_sock, "Insert password: ");
	
			memset(recv_data,'\0',(strlen(recv_data)+1));
			bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0);
			if(bytes_received <= 0){
				printf("\nError!Cannot receive data from client!\n");
				return;
			}
			recv_data[bytes_received-1] = '\0';
			check = strdup(recv_data);
			int count = 1;
			while(count < 3 && strcmp(check, current->password) != 0){
				count ++;
				sendMes(conn_sock, "Insert password: ");
	
				memset(recv_data,'\0',(strlen(recv_data)+1));
				bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0);
				if(bytes_received <= 0){
					printf("\nError!Cannot receive data from client!\n");
					return ;
				}
				recv_data[bytes_received-1] = '\0';
				check = strdup(recv_data);
			}
			if(strcmp(check, current->password) != 0){
				pthread_mutex_lock(&accounts_mutex);
				current->status = 0;
				current->loginStatus = 0;
				pthread_mutex_unlock(&accounts_mutex);
				writeFile();
				sendMes(conn_sock, "Account is Blocked!");
			}else{
				if(current->loginStatus == 1){
					sendMes(conn_sock, "Account is logged in elsewhere");
					return 0;
				}
				pthread_mutex_lock(&accounts_mutex);
				current->loginStatus = 1;
				pthread_mutex_unlock(&accounts_mutex);
				sendMes(conn_sock, "Login success!");
				memset(recv_data,'\0',(strlen(recv_data)+1));
				bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0);
				if(bytes_received <= 0){
					printf("\nError!Cannot receive data from client!\n");
					return;
				}
				recv_data[bytes_received-1] = '\0';
				check = strdup(recv_data);
				if(strcmp(check, "bye") == 0){
					pthread_mutex_lock(&accounts_mutex);
					current->loginStatus = 0;
					pthread_mutex_unlock(&accounts_mutex);
					sendMes(conn_sock, "Logout success!");
				}else{
					sendMes(conn_sock, "Login continue!");
				}
			}
		}else{
			sendMes(conn_sock, "Account is logged in elsewhere");
		}
				
	}else if(search == 0){
		sendMes(conn_sock, "Not found information!");
	}else if(current->status == 0){
		sendMes(conn_sock,"Account is Blocked!");
	}
}

int main(int argc, char *argv[])
{	
	readFile();
	char *check;
	if(argc != 2){
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }

	int listen_sock, conn_sock; /* file descriptors */
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
		
		pthread_t pid;
		pthread_create(&pid, NULL, handleClient, &conn_sock);
	}
	close(listen_sock);
	return 0;
}
