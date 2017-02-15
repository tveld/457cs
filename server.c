// Troy Veldhuizen
// Lab 4 - UDP Echo
// server.c

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
	int sockfd = socket(AF_INET, SOCK_DGRAM,0);
	if(sockfd<0){
		printf("There was an error creating the socket'n");
		return 1;
	}
	
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	// set time limit on recv, timeout after 5 seconds
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&timeout, sizeof(timeout));
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
		int len = sizeof(clientaddr);
		char line[5000];
		int err = recvfrom(sockfd, line, 5000, 0, (struct sockaddr*)&clientaddr, &len);
		if(err == -1){
			printf("error recieving, probably timed out.\n");
		}
		printf("\nGot from client %s\n", line);
		sendto(sockfd, line, strlen(line)+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
		bzero(line, sizeof(line));
	}
	
	
}
