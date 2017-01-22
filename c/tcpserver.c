//GVSU CIS 457 - Data Communications
//Lab 2
//Troy Veldhuizen
//Due 1/20/17
//tcpserver.c
 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
	int sockfd = socket(AF_INET, SOCK_STREAM,0);

	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET;
	// ask for port number
	printf("What port do you want to listen to? \n");
	int port;
	int err = scanf("%d", &port);
	if(err < 0){
		printf("error getting port\n");
	}
	serveraddr.sin_port = htons(port); //had to match
	serveraddr.sin_addr.s_addr = INADDR_ANY;

	int e = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	
	//check for error
	if(e < 0){
		printf("There was an error binding the address\n");
	}
	
	listen(sockfd,10);

	// does not handle multiple clients concurrently
	while(1){
		int len = sizeof(clientaddr);
		//blocking call: waits for connection
		int clientsocket = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
		
		char line[5000];
		
		// e is the number of bytes recieved
		int e = recv(clientsocket,line,5000,0);
		if(e < 0){
			printf("Error recieving\n");
			return 1;
		}
		printf("Got from the client: %s\n", line);
		

		// send to client
		send(clientsocket, line,strlen(line)+1,0);
		close(clientsocket);	
	}	
	
	// one socket to listen to incomeing
	// pass off to one socket for specific client	
}	
