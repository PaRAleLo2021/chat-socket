#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

//#include ”netio.h”

#define BUFFSZ 1500

/*seturile de descriptori urmariti*/
fd_set rd_set , wr_set;
int sfd;

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

void sigterm_handler(int s){
	syslog(LOG_INFO, "ex5_server stopped");
	exit(0);
}

void daemonize(void){
	int i, maxfd;
	int fd;

	maxfd = getdtablesize();

	for(i=0; i<maxfd; i++)
		close(i);
	chdir("/");
	switch(fork()){
		case -1:
			syslog(LOG_ERR, "Eroare la fork()\n");
		case 0: /*fiu*/
			break;
		default: /*parinte*/
			exit(0);
	}
	setpgid( 0, 0);
	fd = open ("/dev/tty", O_RDWR);
	if(fd >= 0) {
		ioctl(fd, TIOCNOTTY);
		close(fd);
		syslog(LOG_INFO, "ex5_server started");
	}
}

void main_loop(struct sockaddr_in remote){
	int ret;
	char buf[BUFFSZ];
	char *rpos = buf;
	char *wpos = buf;
	char *last = buf+BUFFSZ;
	int free = BUFFSZ;
	int avail = 0;

	for (;;){
		FD_ZERO(&rd_set);
		FD_ZERO(&wr_set);
		if(free)
			FD_SET(sfd, &rd_set);
		if(avail)
			FD_SET(sfd, &wr_set);
		ret = select(sfd+1, &rd_set, &wr_set, NULL, NULL);
		if(ret == -1) {
			if(errno == EINTR)
				continue;
			syslog(LOG_ERR, "error at select()");
			exit(1);
		}
		if(FD_ISSET(sfd, &wr_set)) {
			ret = sendto(sfd, wpos, avail, 0,
				(void *)&remote, sizeof(remote));
			if(-1 == ret) {
				syslog(LOG_ERR, "error write()ing");
			}
			avail -= ret;
			wpos += ret;
			if(avail== 0 && wpos == last) {
				wpos = rpos = buf;
				free = BUFFSZ;
			}
		}
		if (FD_ISSET(sfd, &rd_set)) {
			ret = read(sfd, rpos, free);
			if(-1 == ret) {
				syslog(LOG_ERR, "error read()ing");
				exit(1);
			}
			free -= ret;
			avail += ret;
			rpos += ret;
		}
	}
}

int main(int argc, char* argv[]) {
	int port_l, port_r;
	struct sockaddr_in local;
	struct sockaddr_in remote;

	if(argc != 4) {
		printf( "Utilizare: %s port_l adresa portr\n",
			argv[0]);
		exit(1);
	}

	port_l = atoi(argv[1]);
	port_r = atoi(argv[3]);
	if(port_l < 0 || port_r < 0) {
		printf("Port incorect");
		exit(1);
	}

	daemonize();

	signal(SIGTERM, sigterm_handler);
	signal(SIGPIPE, SIG_IGN);

	openlog("ex5", 0, LOG_WARNING);
	sfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sfd == -1) {
		syslog(LOG_ERR, "Could not create socket");
		exit(1);
	}

	set_addr(&local, NULL, INADDR_ANY, port_l);

	if(-1 ==
		bind(sfd, (struct sockaddr *)&local,
			sizeof(local))) {
		syslog(LOG_ERR, "Could not bind to local");
		exit(1);
	}

	if(-1 == set_addr(&remote, argv[2], 0, port_r)) {
		syslog(LOG_ERR, "Wrong address");
		exit(1);
	}
	main_loop(remote);
	exit(0);
}