// Troy Veldhuizen
// Lab 4 - UDP Echo
// server.c

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
#include <dirent.h>

int main(int argc, char** argv){
	int sockfd = socket(AF_INET, SOCK_DGRAM,0);
	if(sockfd<0){
		printf("There was an error creating the socket'n");
		return 1;
	}
	
	int port;
	int error;

	printf("What port do you want to listen to? \n");
  char input[100];
  fgets(input, 100, stdin);
  port = atoi(input);

  // validate input
  if(port < 1024 || port  > 49000){
    printf("This is not a valid port number");
    printf(" Please pick a port in the range of 1024 to 4900.\n");	
  }


	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(port);
	serveraddr.sin_addr.s_addr= INADDR_ANY;
	
	bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	while(1){
		// Recieve FName
		char fname[5000];
		int len = sizeof(clientaddr);
		recvfrom(sockfd, fname, 5000, 0, (struct sockaddr*)&clientaddr, &len);
		
		printf("Length of string: %d\n", strlen(fname));

		// fetch the file if it exists
		int fd; //file descriptor
		fd = open(fname, O_RDONLY);


		// send file to the client
		
		// get the size of the file
		struct stat stat_buf;
		fstat(fd, &stat_buf);
		printf("\nbytes: %d\n", stat_buf.st_size);
		
		int size = htonl(stat_buf.st_size);

		

		// send over file size
		sendto(sockfd, &size, sizeof(size), 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
		// setup packet
		//char packet[1024];

		// Send File
		//sendto(sockfd, fname, strlen(fname)+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
		//bzero(fname, sizeof(fname));
		
	}
	
	
}
