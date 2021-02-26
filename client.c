#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//#include ” netio.h”
#define SERVER_ADDRESS "localhost"
#define SERVER_PORT 5000

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

int main ( void) {
	int sockfd;
	struct sockaddr_in local_addr, remote_addr;
	if(-1 == (sockfd = socket(PF_INET, SOCK_DGRAM, 0))){
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
	write(sockfd, "abcd", strlen ("abcd"));
	write(sockfd, "ab", strlen("ab"));
	write(sockfd, "cd", strlen("cd"));
	exit(0);
}
