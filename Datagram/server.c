#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
//#include ” netio.h”

#define SERVER_PORT 5678
#define MAXBUF 2048

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

int main(void){
	int sockfd;
	char buf[MAXBUF];
	struct sockaddr_in local_addr;
	int nread;
	if(-1 == (sockfd = socket(PF_INET, SOCK_DGRAM, 0))){
		printf("Eroare la socket()\n");
		exit(1);
	}
	set_addr(&local_addr, NULL, INADDR_ANY,
		SERVER_PORT);
	if(-1 ==
		bind(sockfd, (struct sockaddr *)&local_addr,
			sizeof(local_addr))){
		printf("Eroare la bind()\n");
	exit(1);
	}
	while(0 < (nread = read(sockfd, &buf, MAXBUF))) {
		printf("%.*s\n", nread, buf);
	}
	exit(0);
}