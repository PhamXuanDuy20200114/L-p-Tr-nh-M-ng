/*UDP Echo Server*/
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>

//#define PORT 5550  /* Port that will be opened */ 
#define BUFF_SIZE 1024
char tenFile[100] = "./account.txt";
int status;
char *password;
char login[50];

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
	f = fopen(tenFile,"r");
	if(f == NULL){
		printf("Loi mo file\n");
	}

	while(!feof(f)){
		struct User *ptr = (struct User*) malloc(sizeof(struct User));
		fscanf(f,"%s %s %d\n",ptr->userName,ptr->password,&ptr->status); 
		ptr->next = head;
		head = ptr;
	}
	fclose(f);
}

int searchUserName(char name[]){
	if(head == NULL){
		return 0;
	}
	struct User *link = head;
	while(link != NULL){
		if(strcmp(link->userName,name) == 0){
			status = link->status;
			password = link->password;
			current = link;
			return 1;
		}
		link= link->next;
	}
	return 0;
}

int checkPassword(char password[]){
	if(head == NULL){
		return 0;
	}
	struct User *link = head;
	if(strcmp(current->password,password) == 0){
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }
	readFile();

	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	socklen_t sin_size=sizeof(struct sockaddr_in);

	//Step 1: Construct a UDP socket
	if ((server_sock=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}
	
	//Step 2: Bind address to socket
	server.sin_family = AF_INET;         
	server.sin_port = htons(atoi(argv[1]));   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */   
	bzero(&(server.sin_zero),8); /* zero the rest of the structure */

  
	if(bind(server_sock,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1){ /* calls bind() */
		perror("\nError: ");
		exit(0);
	}     
	
	//Step 3: Communicate with clients
	while(1){
		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client, &sin_size);
		if (bytes_received < 0)
			perror("\nError: ");
		else{
			buff[bytes_received] = '\0';
			char *userName = buff;
			if(searchUserName(userName) == 1){
				int count = 1;
				bytes_sent = sendto(server_sock, "Insert password: ", strlen("Insert password: ") , 0, (struct sockaddr *) &client, sin_size ); /* send to the client welcome message */
				bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client, &sin_size);
				if (bytes_received < 0)
					perror("\nError: ");
				else if (bytes_sent < 0)
					perror("\nError: ");
				while(count < 3 && strcmp()){

				}
				
				
			}
		}					
	}
	
	close(server_sock);
	return 0;
}
