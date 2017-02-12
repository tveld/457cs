// Troy Veldhuizen
// Lab 4 - UDP Echo
// client.c

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv){
	int sockfd = socket(AF_INET, SOCK_DGRAM,0);
	if(sockfd<0){
		printf("There was an error creating the socket'n");
		return 1;
	}

	int port;

	printf("What port do you want to listen to? \n");
	char input[100];
	fgets(input, 100, stdin);
	port = atoi(input);

	// validate input
	if(port < 1024 || port  > 49000){
	printf("This is not a valid port number");
	printf(" Please pick a port in the range of 1024 to 4900.\n");	
	}

	printf("What ip do you want to send to? \n");
	char ip[100];
	fgets(ip, 100, stdin);

	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(port);
	serveraddr.sin_addr.s_addr=inet_addr(ip);
	
	while(1) {

		printf("\nEnter the File Name you wish to recieve:  ");

		char fname[5000];
		fgets(fname, 5000, stdin);
		
		// send filename
		sendto(sockfd, fname, strlen(fname) - 1, 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		
		int len = sizeof(serveraddr);
		char responce[5000];

		// recieve file size
		int size = 0;
		recvfrom(sockfd, size, sizeof(size), 0, (struct sockaddr*)&serveraddr, &len);
		
		printf("\nBytes %d\n", ntohl(size));
 
		// recieve file
		/*
		recvfrom(sockfd, responce, 5000, 0, (struct sockaddr*)&serveraddr, &len);
		printf("\nGot from server %s\n", responce);
		
		bzero(fname, sizeof(fname));
		bzero(responce, sizeof(responce));
		*/
	}
	close(sockfd);
	return 0;
}
