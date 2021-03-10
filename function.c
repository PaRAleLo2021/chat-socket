/* function.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

#define NORMAL  "\x1B[0m"
#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define WHITE  "\x1B[37m"

#include "authentication.h"
#define MAXBUF 2048

//gets message with recv not including cmd
char *get_string(int sockfd, char *message){
  char buf[72];
  //message length
  if(recv(sockfd, buf, 5, 0) < 0){
		printf("error receiving message");
		return NULL;
	}
	int size=atoi(buf);

  if(recv(sockfd, buf, 72, 0) < 0){
		printf("error receiving message");
		return NULL;
	}
  if(recv(sockfd, message, size, 0) < 0){
		printf("error receiving message");
		return NULL;
	}
  return message;
}

int send_mesage(int sockfd, char *str, char cmd, char *sender){
	char buf[MAXBUF+80];
	bzero(buf, MAXBUF+80);
	int size=strlen(str)+1;
	/***********Message Header**********/
	sprintf(buf, "%c%d", cmd, size); //no more than 6

	/***********Message Body************/
	strcpy(buf+6, sender); //no more than 50
	char time[11], date[11];
	get_current_time(time, sizeof(time));
	strcpy(buf+56,time);
	get_current_date(date, sizeof(date));
	strcpy(buf+67, date);
	strncpy(buf+78, str, MAXBUF);
	buf[MAXBUF+78-1]='\0';

	if(size>MAXBUF)
		size=MAXBUF;

	if(send(sockfd, buf, size+78, 0) < 0){
		printf("error sending message");
		return -1;
	}
	return 0;
}

int recv_message(int sockfd){

	/***********Message Header**********/
	char header[8];
	int message_size;
	if(recv(sockfd, header, 6, 0) < 0){
		printf("error receiving message");
		return -1;
	}
	message_size=atoi(header+1);

	/*********Message Body*************/
	char sender[50];
	if(recv(sockfd, sender, 50, 0)< 0){
		printf("error receiving message");
		return -1;
	}

	char time[11], date[11];
	if(recv(sockfd, time, 11, 0)< 0){
		printf("error receiving message");
		return -1;
	}
	if(recv(sockfd, date, 11, 0)< 0){
		printf("error receiving message");
		return -1;
	}

	char message[MAXBUF];
  bzero(message,MAXBUF);
	if(recv(sockfd, message, message_size, 0)< 0){
		printf("error receiving message in recv_message");
		return -1;
	}

	switch(header[0]){
		case 'M':
			printf("%s%s (%s@%s):%s %s", CYAN, sender, date, time, NORMAL, message);
			break;
		case 'S':
			printf("%s%s (%s@%s):%s %s", MAGENTA, sender, date, time, NORMAL, message);
			break;
		case 'Q':
      //printf("answer %s\n",message);
			if(message[0]=='y'||message[0]=='Y')
				return 2;
			return -2;
		default:
			printf("%s %s\n%s", MAGENTA, message, NORMAL);
	}
	return 0;
}

int set_addr(struct sockaddr_in *addr, char *name,
	u_int32_t inaddr, short sin_port){
	struct hostent *h;

	memset((void *)addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	if(name != NULL){
		h=gethostbyname(name);
		if(h == NULL)
			return -1;
		addr->sin_addr.s_addr = *(u_int32_t *) h->h_addr_list[0];
	}
	else
		addr->sin_addr.s_addr=htonl(inaddr);
	addr->sin_port = htons(sin_port);
	return 0;
}
