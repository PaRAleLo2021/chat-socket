#include "function.c"

#define MAXBUF 2048


int create_user(int sockfd){
	char cmd[1], username[50], password[50], date[11];
	send_mesage(sockfd, "User does not exist. Create a new user.\nusername: ", 'S', "server");

  if(recv(sockfd, cmd, 1, 0) < 0){
    printf("error receiving message");
    return -1;
  }
  if(cmd[0]!='U'){
    printf("error user not received3");
    return -1;
  }
	if(get_string(sockfd, username)==NULL){
    printf("error user not received4");
    return -1;
  }


  send_mesage(sockfd, "password: ", 'S', "server");
  if(recv(sockfd, cmd, 1, 0) < 0){
    printf("error receiving message");
    return -1;
  }
  if(cmd[0]!='P'){
    printf("error password not received");
    return -1;
  }
	if(get_string(sockfd, password)==NULL){
    printf("error password not received");
    return -1;
  }
  get_current_date(date, sizeof(date));

  add_user(username, password, date, date);
  send_mesage(sockfd, "User added.\n", 'S', "server");


  return 0;
}

int client_authentication(int sockfd,char *client_username){
  char cmd[1], username[50], password[50];
  int ok;


  do{
    ok=-1;
    //username
    send_mesage(sockfd, "Welcome! What should people call you?\nusername: ", 'S', "server");
    //send(sockfd, message, MAXBUF, 0);
    if(recv(sockfd, cmd, 1, 0) < 0){
      printf("error receiving message");
      return -2;
    }
    if(cmd[0]!='U'){
      printf("cmd %c\n", cmd[0]);
      printf("error user not received1");
      return -2;
    }
  	if(get_string(sockfd, username)==NULL){
      printf("error user not received2");
      return -1;
    }

    if(search_user(username)==NULL){
      send_mesage(sockfd, "n", 'Q', "server");
      create_user(sockfd);
    }
    else{
      send_mesage(sockfd, "y", 'Q', "server");
      do{
        send_mesage(sockfd, "password: ", 'S', "server");

        if(recv(sockfd, cmd, 1, 0) < 0){
          printf("error receiving message");
          return -2;
        }
        //printf("received the password: -%c-\n",cmd[0]);
        if(cmd[0]!='P'){
          printf("error password not received");
          return -2;
        }
      	if(get_string(sockfd, password)==NULL){
          printf("error password not received");
          return -1;
        }

        ok=check_password(username, password); // 1- OK | 0 - NOT OK
        if(ok==1)
          send_mesage(sockfd, "y", 'Q', "server");
        else
          send_mesage(sockfd, "n", 'Q', "server");
      }while(ok<=0);
    }

  }while(ok<=0);
	strcpy(client_username,username);
  return 1;
}
