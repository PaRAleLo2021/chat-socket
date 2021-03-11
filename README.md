# chat-socket
## Table of contents
* [Project Specifications](#project-specifications)
* [Protocol definition](#protocol-definition)
* [Compiling and running]()

## Project Specifications
A program that offers a chat room for multiple clients.
written in C, using BSD sockets on Linux or UNIX

![client server connection](https://github.com/alexandra-doris/Images/blob/main/client-server.png)

Components:
* concurrent server - serves multiple clients at the same time, using a thread for each
* client - has two threads, one for getting input from the user and sending it to the server and one for receiving messages from the server

### Server program(single):
Manages a chat room by accepting multiple simultaneous connections from
clients who want to talk. 
Authentication: Clients have to authenticate with a name and a password.The name must be unique.

### Client program(multiple):
Connects to the server and uses a text interface through which he can send messages to the chat room. A message from a client will instantly appear in the interfaces of all clients, including the sender’s interface.


## Protocol definition

### Message Structure
CMD | SIZE | SENDER    | DATE  | TIME  | MESSAGE
----| ---- | ----------| ------| ------| ---------------
1   | 5    | 50        | 11    | 11    | SIZE <= MAXBUF


**CMD**     | **Data**
------------|------------------------
‘M’/’S’ | Message from Client/Server
‘U’/’P’ | Username/Password hashed
‘Q’     | “y”/”n”

### Communication rules

![communication-rules](https://github.com/alexandra-doris/Images/blob/main/communication-reules.png)

## Compiling and running
> gcc -Wall -o server -lpthread server.c && ./server

> gcc -Wall -o client -lpthread client.c && ./client
