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

int main(int argc, char **argv){
	int sockfd = socket(AF_INET, SOCK_DGRAM,0);
	int port = 0;    // port to listen on
	int e = -1;      // connection return value

	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET;
	
	while(e<0){
       	    while(port < 1024 || port  > 49000){
	      // ask for port number
	      printf("What port do you want to listen to? \n");
	      char input[100];
	      scanf("%s", &input);
	      port = atoi(input);
	
	      // validate input
	      if(port < 1024 || port  > 49000){
	        printf("This is not a valid port number.");
	        printf(" Please pick a port in the range of 1024 to 4900.\n");	
	        }
	      }
	
	serveraddr.sin_port = htons(port); //had to match
	serveraddr.sin_addr.s_addr = INADDR_ANY;

      e = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	
	    //check for error
	    if(e < 0){
		printf("There was an error binding the address\n");
	        printf("Resetting connection process...\n");
	    }
	}
	
	// REMOVED
	//listen(sockfd,10);

	// does not handle multiple clients concurrently
	while(1){
		int clientsocket = 0;
		int len = sizeof(clientaddr);
		
		// REMOVED
		//blocking call: waits for connection
		//clientsocket = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
	
		if(clientsocket >= 0){
			printf("Connected to client\n");
		}

		char fname[5000];
		
		// e is the number of bytes recieved
		int e = recv(clientsocket,fname,5000,0,(struct sockaddr*)&clientaddr, &len);
		if(e < 0){
			printf("Error recieving\n");
			return 1;
		}
	
		// fetch file if it exists
		int success = 1;
		FILE *file;
		
		int fd; //file descriptor
		fd = open(fname, O_RDONLY);	
		
		if(fd == -1){
			success = 0;
		}

		// send file to the client
		
		// get the size of the file
		struct stat stat_buf;
		fstat(fd, &stat_buf);
		

		// use sendfile	
		off_t offset = 0;
		
    		int rc = sendfile (clientsocket, fd, &offset, stat_buf.st_size);
    		if (rc == -1) {
      			printf("Error sending file");
      			exit(1);
    		}	
		
		close(fd);


		// send response to client
		char response[5000];

	        if(success == 1){
			strcpy(response,"File transfer successfull");
		} else {
			strcpy(response,"Could not transfer file");
		}
				
		// send to client response
		//send(clientsocket, response,strlen(response)+1,0);
		close(clientsocket);	
	}	
	
	// one socket to listen to incomeing
	// pass off to one socket for specific client	
}	
