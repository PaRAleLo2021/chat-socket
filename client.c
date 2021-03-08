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
#include "authentication.h"

#define SERVER_ADDRESS "localhost"
#define SERVER_PORT 5000
#define MAXBUF 2048

int client_sockfd;

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

void reading_fct()
{
    char buf[MAXBUF];
    recv(client_sockfd, buf, MAXBUF, 0);
    printf("%s", buf);
    fgets(buf, MAXBUF, stdin);
    send(client_sockfd, buf, MAXBUF, 0);
}

void client_io(int sockfd){
	client_sockfd = sockfd;
    
    /*reading_fct();
    reading_fct();*/
    
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
		printf(">> ");
		fflush(stdout);
	  fgets(message, MAXBUF, stdin);
		int sl=0;
		if((sl=send(client_sockfd, message, MAXBUF, 0)) <=0)
			return NULL;
		printf("msg sent %d\n", sl);
		if(strcmp(message, "exit\n")==0){
			printf("\nExited chat\n");
			exit(1);
		}
		bzero(message, MAXBUF);
	}
}

void *recv_msg(void *arg){
	char message[MAXBUF];
	while(1){
		if(recv(client_sockfd, message, MAXBUF, 0) >0){
			printf("%s", message);
			printf(">> ");
			fflush(stdout);
		}

		bzero(message, MAXBUF);
	}
}
