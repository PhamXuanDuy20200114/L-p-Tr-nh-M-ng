#include<stdio.h>
#include<string.h>
#include<stdlib.h>

char tenFile[100] = "./account.txt";
int status;
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
			status = link->status;
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
	if(strcmp(current->password,password) == 0){
		return 1;
	}
	return 0;
}

void insertUser(char name[], char pass[]){
	struct User *link = (struct User*) malloc(sizeof(struct User));
		strcpy(link->userName,name);
		strcpy(link->password,pass);
		link->status = 1;
		link->next = head;
		head = link;
		printf("Successful registration!\n");
}

void main(){
	readFile();
	printf("\nUSER MANAGEMENT PROGRAM\n");
	printf("------------------------------\n");
	printf("1. Register \n2. Sign in \n3. Search \n4.Sign out\n");
	printf("Your choice (1-4, other to quit)\n");
	int choice;
	scanf("%d", &choice);
	while(choice <= 4 && choice >=1){
		switch(choice){
			case 1:{
				char uName[50], pass[50];
				printf("User Name: ");
				scanf("%s",uName);
				if(searchUserName(uName) != 0){
					printf("Account Existed!\n");
					break;
				}
				printf("Password: ");
				scanf("%s",pass);
				insertUser(uName,pass);
				writeFile();
				break;
			}
			case 2:{
				char uName[50], pass[50];
				printf("User Name: ");
				scanf("%s",uName);
				int i = searchUserName(uName);
				if(i == 0){
					printf("Cannot find account!\n");
					break;
				}
				if(status == 0 && i == 1){
					printf("Account is blocked!\n");
					break;
				}
				printf("Password: ");
				scanf("%s",pass);
				int count = 1;
				while(count < 3 && checkPassword(pass) != 1){
					printf("Password is incorrect!\n");
					printf("Password: ");
					scanf("%s",pass);
					count++;
				}
				if(checkPassword(pass) != 1){
					current->status = 0;
					int i = remove(tenFile);
					writeFile();
					printf("Account is blocked!\n");
				}else{
					strcpy(login,uName);
					printf("Login Successful!\n");
				}
				break;
			}
			case 3:{
				char uName[50];
				printf("User Name: ");
				scanf("%s",uName);
				if(searchUserName(uName) == 1){
					if(status == 1) printf("Account is active!\n");
					else printf("Account is blocked!\n");
				}
				else printf("Cannot find account!\n");
				break;
			}
			case 4:{
				char uName[50];
				printf("User Name: ");
				scanf("%s",uName);
				if(searchUserName(uName) == 1){
					if(strcmp(uName,login) == 0){
						printf("Good bye %s!\n",uName);
						strcpy(login,"");
					}else printf("Account is not sign in!\n");
				}
				else printf("Cannot find coount!\n");
				break;
			}
		}
		scanf("%d",&choice);
	}
	
}