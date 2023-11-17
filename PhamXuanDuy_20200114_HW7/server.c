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

#define BUFF_SIZE 1024
#define BACKLOG 5   /* Number of allowed connections */
#define MES1 "Account is not active!"
#define MES2 "Account is blocked!"
#define MES3 "Insert password:"
#define MES4 "Login success!"
#define MES6 "Not found information!"
#define FileName "./account.txt"

struct User{
	char userName[50];
	char password[50];
	int status;
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
		fprintf(f,"%s %s %d\n",ptr->userName,ptr->password,ptr->status); 
		ptr = ptr->next;
	}
	fclose(f);
}

void printList(){
	struct User *ptr = head;
	printf("\n[ ");
	while(ptr != NULL){
		printf("%s %s %d || ", ptr->userName, ptr->password, ptr->status);
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

int main(int argc, char *argv[])
{
	readFile();

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
		
		pid_t pid = fork();

        if (pid == 0) {  // Child process
            close(listen_sock); 
			memset(recv_data,'\0',(strlen(recv_data)+1));
			bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0);
			if(bytes_received <= 0){
				printf("\nError!Cannot receive data from client!\n");
				break;
			}
			char *check = strdup(recv_data);
			printf("%s", check);
			if(searchUserName(check) == 0){
				sendMes(conn_sock, "OK");
			}
			else{
				sendMes(conn_sock, MES6);
			}
			
            exit(EXIT_SUCCESS);
        } else if (pid > 0) { 
            close(conn_sock);  
        } else {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
	}
	close(listen_sock);
	return 0;
}
