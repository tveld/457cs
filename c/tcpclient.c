//GVSU CIS 457 - Data Communications
////Lab 2
////Troy Veldhuizen
////Due 1/20/17

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
    
	int sockfd = socket(AF_INET, SOCK_STREAM,0);
	
	//if there are too many open files, but not likely
	if(sockfd<0){
		printf("There was an error creating the socket'n");
		return 1;
	}

	struct sockaddr_in serveraddr, clientaddr;

	//IPV4 Address, maintains info 
	//even if sockaddr is cast to something else
	serveraddr.sin_family=AF_INET;	
	
	// Get port from user input
	printf("What port do you want to send to?\n");
	int port;
	int err = scanf("%d", &port);
	if(err < 0){
		printf("Error getting the port number\n");
	}

	// get the ip from user input
	printf("What ip do you want to send to?\n");
	char ip[5000];
	int error = scanf("%s", &ip);
	if(error < 0){
		printf("Error reading ip.\n");
	}
	//sets what port this app is listening to (1024-49000)
	serveraddr.sin_port=htons(port);  

	// 127.0.0.1 is a loopback address
	// address of client
	serveraddr.sin_addr.s_addr=inet_addr(ip);
	
	// takes socket and address and binds them together
	// makes sure someone is listening at that address
	int e = connect(sockfd,(struct sockaddr*)&serveraddr, sizeof(serveraddr));

	if(e < 0){
		printf("There was an error with connecting\n");
		return 1;
	}

	//send data

	printf("Enter a message from client:  ");
	char line[5000];
	scanf("%s",line);

	// added one for the null char at the end
	// strlen is a poor way to check non strings lol
	send(sockfd, line,strlen(line)+1,0);
	
	//recieve echo
		int len = sizeof(serveraddr);
		int clientsocket = accept(sockfd, (struct sockaddr*) &serveraddr, &len);
		
		char ln[5000];

		recv(sockfd,ln,5000,0);
		printf("Echoed back from server: %s\n",ln);
	
		close(clientsocket);
}

