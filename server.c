#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "function.c"
#include "authentication.h"

#define SERVER_PORT 5000
#define MAXBUF 2048
#define MAXCLIENTS 8

int clients[MAXCLIENTS];
pthread_mutex_t clients_mutex;
int no_clients = 0;

void concurent_server();

void *client_connection(void *arg);

/**Add client in array**/
void queue_add(int sockfd){
  pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAXCLIENTS; ++i){
		if(!clients[i]){
			clients[i] = sockfd;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

/* Remove client from array */
void queue_remove(int sockfd){
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAXCLIENTS; ++i){
		if(clients[i]){
			if(clients[i] == sockfd){
				clients[i] = 0;
				break;
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

/**Send message to all clients**/
void send_message(char *s, int sockfd){
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i<MAXCLIENTS; ++i){
		if(clients[i]){
				if(send(clients[i], s, MAXBUF, 0) < 0){
					printf("error send_message()");
					break;
				}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

int main(int argc, char* argv[]){
  concurent_server();
  return 0;
}


/***Concurent server***/
void concurent_server(){
  int sockfd, client_sockfd;
  pthread_t tid;
  pthread_mutex_init(&clients_mutex, NULL);

  //create a new socket
  if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
     printf ("error creating a new socket"); exit(1);
  }

  //reuse address
  int yes=1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    printf("error setsockopt()");
    exit(1);
  }

  //bind a IP and port to the socket
  struct sockaddr_in server_addr;
  set_addr(&server_addr, NULL, INADDR_ANY, SERVER_PORT);
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
     printf ("error bind()"); exit(1);
  }

  //mark the socket as passive
  //will accept incoming connections
  if (listen(sockfd, MAXCLIENTS) < 0){
     printf ("error listen()"); exit(1);
  }

  printf("*** Chat Open ***\n");

	while(1){
    socklen_t len = sizeof(server_addr);
    client_sockfd=accept(sockfd, (struct sockaddr *)&server_addr, &len);

		/* Check if max clients is reached */
		if((no_clients + 1) == MAXCLIENTS){
			printf("Max clients reached. Rejected.");
			close(client_sockfd);
			continue;
		}

		/* Add client to the queue and fork thread */
		queue_add(client_sockfd);
		pthread_create(&tid, NULL, &client_connection, (void*)&client_sockfd);
	}
}

void *client_connection(void *arg){
  char message[MAXBUF];
    //char welcome_string[MAXBUF];
    
	no_clients++;
	int *client_sockfd=(int *)malloc(sizeof(int));
    if(client_sockfd==NULL)
    {
        printf("Memory not allocated\n");
        exit(0);
    }    
    *client_sockfd=*(int *)arg;
  fcntl(*client_sockfd, F_SETFL, O_NONBLOCK);
/*
  bzero(welcome_string, MAXBUF);
  strncpy(welcome_string, "Welcome!\nWhat should people call you?\n", MAXBUF);
  
  send(*client_sockfd, welcome_string, MAXBUF, 0);
  
  printf("***\n");
  
  
  char username[50];
  char password[50];
  recv(*client_sockfd, username, 50, 0);
  
  
  strncpy(welcome_string, "Password?\n", MAXBUF);
  send(*client_sockfd, welcome_string, MAXBUF, 0);
  
  recv(*client_sockfd, password, 50, 0);
  
  
  int size=11;
  char s[size];
  strcpy(s, get_current_date(s, size));
  struct user *this_user=search_user(username);
  if(this_user==NULL)
  {
      add_user(username, password, s, s);
  }
  
  int verify=check_password(username, password);
  if(verify==0)
  {
      printf("Access denied\n");
      return NULL;
  }
      
  queue_add(*client_sockfd);*/
  printf("Someone entered\n");
  send_message("Someone entered\n", *client_sockfd);

	while(1){
		int rcv_len = recv(*client_sockfd, message, MAXBUF, 0);
        if(rcv_len==0)
        {
            printf("Someone exited\n");
            send_message("Someone exited\n", *client_sockfd);
            break;
        }
		if (rcv_len > 0){
            printf("Received msg: %s", message);
            if(strcmp("exit\n", message)==0){
                printf("Someone exited\n");
                send_message("Someone exited\n", *client_sockfd);
                break;
            }
			if(strlen(message) > 0){
				send_message(message, *client_sockfd);
			}
		}
		bzero(message, MAXBUF);
	}

  /* Delete client from queue and yield thread */
	close(*client_sockfd);
  queue_remove(*client_sockfd);
  no_clients--;
  pthread_detach(pthread_self());
    free(client_sockfd);
	return NULL;
}
