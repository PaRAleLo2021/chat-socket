#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "function.c"

#define SERVER_ADDRESS "localhost"
#define SERVER_PORT 5000
#define MAXBUF 2048

int client_sockfd;
char username[50];

int create_user(int sockfd);
int client_authentication(int sockfd);

void client_io(int sockfd);
void* send_msg(void *arg);
void* recv_msg(void *arg);

int main ( void) {
	int sockfd;
	struct sockaddr_in local_addr, remote_addr;
	if(-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0))){
		printf("Eroare la socket()\n");
		exit(1);
	}
	set_addr(&local_addr, NULL, INADDR_ANY, 0);
	if(-1 ==
		bind(sockfd, (struct sockaddr *)&local_addr,
			sizeof(local_addr))){
		printf("Eroare la bind()\n");
	exit(1);
	}
	set_addr(& remote_addr, SERVER_ADDRESS, 0,
		SERVER_PORT);
	if(-1 ==
		connect(sockfd, (struct sockaddr *)&remote_addr,
			sizeof(remote_addr))){
		printf("Eroare la connect()\n");
	exit(1);
	}

	client_io(sockfd);

	close(sockfd);
	exit(0);
}

void client_io(int sockfd){
	client_sockfd = sockfd;

	/*******Authentication******/
	if(client_authentication(sockfd)<0){
    printf("Client connection failed\n");
    return;
  }


	pthread_t recv_t;
  if(pthread_create(&recv_t, NULL, (void *) recv_msg, NULL)!=0){
		printf("error pthread");
    exit(1);
	}

	pthread_t send_t;
  if(pthread_create(&send_t, NULL, (void *) send_msg, NULL)!=0){
		printf("error pthread");
		exit(1);
	}


	pthread_join(recv_t, NULL);
  pthread_join(send_t, NULL);
}


void *send_msg(void *arg){
	char message[MAXBUF];
	while(1){
		fflush(stdout);
	  fgets(message, MAXBUF, stdin);
		if((send_mesage(client_sockfd, message, 'M', username)) <0)
			return NULL;
		if(strcmp(message, "exit\n")==0){
			printf("\nExited chat\n");
			exit(1);
		}
		bzero(message, MAXBUF);
	}
}

void *recv_msg(void *arg){
	while(1){
		recv_message(client_sockfd);
	}
}

int create_user(int sockfd){
	char password[50];

	//username
	recv_message(sockfd);
	fflush(stdin);
	if(fgets(username, 50, stdin)==NULL){
		printf("fgets error\n");
		return -1;
	}
	char *p;
	p = strchr(username, '\n');
	if (p) {
		*p = '\0';
	}
	send_mesage(sockfd, username, 'U', "user");

	//password
	recv_message(sockfd);
	fflush(stdin);
	if(fgets(password, 50, stdin)==NULL){
		printf("fgets error\n");
		return -1;
	}
	p = strchr(password, '\n');
	if (p) {
		*p = '\0';
	}
	send_mesage(sockfd, password, 'P', "user");

	recv_message(sockfd);


  return 0;
}

int client_authentication(int sockfd){
  char password[50];
  int ok;
  do{
    ok=-1;
    //username
		recv_message(sockfd); //welcome message
		fflush(stdin);
		if(fgets(username, 50, stdin)==NULL){ // username
			printf("fgets error\n");
			return -1;
		}
		char *p;
		p = strchr(username, '\n');
    if (p) {
      *p = '\0';
    }
		//printf("this is the username -%s-", username);
		send_mesage(sockfd, username, 'U', "user");

    if(recv_message(sockfd)<0){
      create_user(sockfd);
    }else{
      do{
				//password
				recv_message(sockfd);
				fflush(stdin);
				if(fgets(password, 50, stdin)==NULL){ // password
					printf("fgets error\n");
					return -1;
				}
				char *p;
				p = strchr(password, '\n');
		    if (p) {
		      *p = '\0';
		    }
				send_mesage(sockfd, password, 'P', "user");

        ok=recv_message(sockfd);

      }while(ok<0);
    }

  }while(ok<0);
  return 1;
}
