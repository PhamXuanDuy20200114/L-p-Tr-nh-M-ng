/*UDP Echo Server*/
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <ctype.h>

//#define PORT 5550  /* Port that will be opened */ 
#define BUFF_SIZE 1024
#define MES1 "Account is not active!"
#define MES2 "Account is blocked!"
#define MES3 "Insert password:"
#define MES4 "Login success!"
#define MES6 "Error String!"
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
	f = fopen(tenFile,"w");
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

char *SHA256Hashing(char *password)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    char *output = (char *)malloc(65);
    int i = 0;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0';
    return output;
}
// return digit in string
char *digitInString(char *str)
{
    int i = 0;
    char *digit = (char *)malloc(BUFF_SIZE);
    int j = 0;
    while (i < strlen(str))
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            digit[j] = str[i];
            j++;
        }
        i++;
    }
    digit[j] = '\0';
    return digit;
}
// return character in string
char *charInString(char *str)
{
    int i = 0;
    char *character = (char *)malloc(BUFF_SIZE);
    int j = 0;
    while (i < strlen(str))
    {
        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
        {
            character[j] = str[i];
            j++;
        }
        i++;
    }
    character[j] = '\0';
    return character;
}

int main(int argc, char *argv[])
{	
    if(argc != 2){
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }
	readFile();
	printList();
	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	socklen_t sin_size = sizeof(struct sockaddr_in);

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
		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &client, &sin_size);
		if (bytes_received < 0){
			perror("\nError: ");
			close(server_sock);
			return 0;
		}
		buff[bytes_received-1] = '\0';
		char *userName = strdup(buff);
		int check = searchUserName(userName);
		if(check == 1){
			if(current->status == 0){
				bytes_sent = sendto(server_sock, MES2, strlen(MES2), 0, (struct sockaddr *) &client, sin_size);
			}
			else{
				bytes_sent = sendto(server_sock, MES3, strlen(MES3), 0, (struct sockaddr *) &client, sin_size);
				bytes_received = recvfrom(server_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &client, &sin_size);
				if (bytes_received < 0){
					perror("\nError: ");
					close(server_sock);
					return 0;
				}
				buff[bytes_received-1] = '\0';
				printf("%s",buff);
				char *password = strdup(buff);
				int count = 1;
				if(strcmp(current->password,password) != 0){
					while(strcmp(current->password,password) != 0 && count < 3 ){
						bytes_sent = sendto(server_sock, MES3, strlen(MES3), 0, (struct sockaddr *) &client, sin_size);
						bytes_received = recvfrom(server_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &client, &sin_size);
						if (bytes_received < 0){
							perror("\nError: ");
							close(server_sock);
							return 0;
						}
						buff[bytes_received-1] = '\0';
						char *x = strdup(buff);
						strcpy(password,x);
						count++;
					}
					if( strcmp(current->password,password) != 0 && count == 3){
						bytes_sent = sendto(server_sock, MES2, strlen(MES2), 0, (struct sockaddr *) &client, sin_size);
						current->status = 0;
						int i = remove(tenFile);
						writeFile();
					}
				}
				if(strcmp(current->password,password) == 0){
					bytes_sent = sendto(server_sock, MES4, strlen(MES4), 0, (struct sockaddr *) &client, sin_size);
					bytes_received = recvfrom(server_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &client, &sin_size);
					if (bytes_received < 0){
						perror("\nError: ");
						close(server_sock);
						return 0;
					}
					buff[bytes_received-1] = '\0';
					char *checkPassword = strdup(buff);
					if(strcmp(checkPassword,"bye") == 0){
						char noti[] = "Goodbye ";
						strcat(noti, current->userName);
						bytes_sent = sendto(server_sock, noti, strlen(noti), 0, (struct sockaddr *) &client, sin_size);	
					}
					else{
						int flag = 0;
						for(int i = 0; checkPassword[i] != '\0'; i++) {
        					if (!(isalnum(checkPassword[i]) || checkPassword[i] == ' ')) {
								flag = 1;
            					break;
        					}
   						}
						if(flag){
							bytes_sent = sendto(server_sock, MES6, strlen(MES6), 0, (struct sockaddr *) &client, sin_size);
						}
						else{
							strcpy(current->password, checkPassword);
							int i= remove(tenFile);
							writeFile();
							char* hashString = SHA256Hashing(checkPassword);
							char *digit = digitInString(hashString);
							char *charString = charInString(hashString);
							printf("%s",digit);
							printf("%s",charString);
							bytes_sent = sendto(server_sock, hashString, strlen(hashString), 0, (struct sockaddr *) &client, sin_size);	
						}
					}
				}
			}
		}else{
			bytes_sent = sendto(server_sock, MES1, strlen(MES1), 0, (struct sockaddr *) &client, sin_size);
		}
	}					
	close(server_sock);
	return 0;
}
