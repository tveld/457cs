//GVSU CIS 457 - Data Communications
//Lab 2
//Troy Veldhuizen
//Due 1/20/17
//tcpserver.c
 
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <pthread.h>

struct clientinfo{
	struct sockaddr_in addr;
	int socket;
};

void* recieve_message(void *arg){
		int run = 1;
		struct clientinfo ci = *(struct clientinfo *)arg;
		int clientsocket = ci.socket;
		char line[5000];
		// e is the number of bytes recieved
		while(run == 1){
			int e = recv(clientsocket,line,5000,0);
			if(e < 0){
				printf("Error recieving\n");
				return 1;
			}
			printf("\nI'm here\n");
			printf("\nGot from the client: %s\n", line);
			

			// fetch file if it exists
						
			int success = 1;
			FILE *file;
			

			int fd; //file descriptor
			fd = open(line, O_RDONLY);

			

			// send file to the client
			
			// get the size of the file
			struct stat stat_buf;
			fstat(fd, &stat_buf);
			//printf("\nbytes: %d\n", stat_buf.st_size);
			
			// send size
			int size = htonl(stat_buf.st_size);
			send(clientsocket, &size, sizeof(size), 0);

			
			// use sendfile	
			off_t offset = 0;
		  	int rc = sendfile (clientsocket, fd, &offset, stat_buf.st_size);
			close(fd);
		}
	
}


int main(int argc, char **argv){
	int sockfd = socket(AF_INET, SOCK_STREAM,0);


	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET;
	
	// ask for port number
	printf("What port do you want to listen to? \n");
	int port;
	int err = scanf("%d", &port);
	if(err < 0){
		printf("Error getting the port number\n");
	}
	if(port < 1024 || port > 49000){
		printf("This is not a valid port.");
		printf(" Please pick a port in the range of 1024 to 4900.\n");
		return 1;	
	}
	
	serveraddr.sin_port = htons(port); //had to match
	serveraddr.sin_addr.s_addr = INADDR_ANY;

	int e = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	
	//check for error
	if(e < 0){
		printf("There was an error binding the address\n");
		return 1;
	}
	
	listen(sockfd,10);

	// does not handle multiple clients concurrently
	while(1){
		int clientsocket = 0;
		int len = sizeof(clientaddr);
		//blocking call: waits for connection
		clientsocket = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
	
		if(clientsocket >= 0){
			printf("Connected to client\n");
		}
	
		struct clientinfo ci;
		ci.socket = clientsocket;
		ci.addr = clientaddr;
	
		// pthreads share the same memory and global variables.
		// solved by semephores and locking

		pthread_t childrec;
		pthread_create(&childrec, NULL, recieve_message, &ci);
		pthread_detach(childrec); // will clean up thread when function ends
		
		
	
		

	/*
		// send response to client
		char response[5000];

	        if(success == 1){
			strcpy(response,"File transfer successfull");
		} else {
			strcpy(response,"Could not transfer file");
		}
				
		// send to client response
		send(clientsocket, response,strlen(response)+1,0);
		close(clientsocket);
	*/
	}	
	
	// one socket to listen to incomeing
	// pass off to one socket for specific client	
}	
