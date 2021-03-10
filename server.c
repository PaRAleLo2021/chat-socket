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
#include "server-function.c"

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
void send_message_to_all(char *s, char cmd){
  int size_int;
  char size_str[5];
	pthread_mutex_lock(&clients_mutex);
  switch (cmd) {
    //case message is from server and neds to be constructed
    case 'S':
      for(int i=0; i<MAXCLIENTS; ++i){
        if(clients[i]){
          if(send_mesage(clients[i], s, 'S', "server") < 0){
              printf("error send_message_to_all()");
              break;
          }
        }
      }
      break;
    //case message from client to be fowarded
    default:
      strncpy(size_str,s+1,5);
      size_int=atoi(size_str);
      for(int i=0; i<MAXCLIENTS; ++i){
        if(clients[i]){
          if((send(clients[i], s, 78+size_int , 0)) <=0){
            printf("error send_message_to_all()");
          }
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

    int *client_sockfd_help=(int *)malloc(sizeof(int));
      if(client_sockfd_help==NULL)
      {
          printf("Memory not allocated\n");
          exit(0);
      }
      *client_sockfd_help=client_sockfd;

		/* Add client to the queue and fork thread */
		pthread_create(&tid, NULL, &client_connection, (void*)client_sockfd_help);
	}
}

void *client_connection(void *arg){
  char message[MAXBUF+80];
  char client_username[50];

	no_clients++;
	int *client_sockfd=(int *)arg;

  if(client_authentication(*client_sockfd, client_username)<0){
    printf("Client connection failed\n");
    return NULL;
  }

  //fcntl(*client_sockfd, F_SETFL, O_NONBLOCK);

  queue_add(*client_sockfd);

  /*********Chat Messaging********/

  printf("-%s entered chat-\n",client_username);
  sprintf(message, "%s entered chat\n", client_username);
  send_message_to_all(message, 'S');

	while(1){
		int rcv_len = recv(*client_sockfd, message, MAXBUF, 0);
    if(rcv_len==0){
      printf("-%s exited chat-\n",client_username);
      sprintf(message, "%s exited chat\n", client_username);
      send_message_to_all(message,'S');
      break;
    }
		if (rcv_len > 0){
      //printf("Received msg: %s", message);
      if(strcmp("exit\n", message+78)==0){
        printf("-%s exited chat-\n",client_username);
        sprintf(message, "%s exited chat\n", client_username);
        send_message_to_all(message, 'S');
        break;
      }
		  if(strlen(message) > 0){
			  send_message_to_all(message, 'M');
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
