//GVSU CIS 457 - Data Communications
////Lab 2
////Troy Veldhuizen
////Due 1/20/17

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
        int port=0; // port number to send to
	int e = -1; // connection return value
	char ip[5000]; // ip address to send to
	char fname[5000]; // file name you want to fetch
	char nname[5000]; // the new name of the file
	int sockfd = socket(AF_INET, SOCK_STREAM,0);

	struct sockaddr_in serveraddr, clientaddr;


	//if there are too many open files, but not likely
	if(sockfd<0){
		printf("There was an error creating the socket'n");
		return 1;
	}

	//IPV4 Address, maintains info 
	//even if sockaddr is cast to something else
	serveraddr.sin_family=AF_INET;	

	while(port < 1024 || port  > 49000){
	  // ask for port number
	  printf("What port do you want to listen to? \n");
	  char input[100];
	  scanf("%s", &input);
	  port = atoi(input);
	
	  // validate input
	  if(port < 1024 || port  > 49000){
	    printf("This is not a valid port number");
	    printf(" Please pick a port in the range of 1024 to 4900.\n");	
	  }
	}
	
	while(e<0){
	  // get the ip from user input
	  printf("What ip do you want to send to?\n");
	
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
	  e = connect(sockfd,(struct sockaddr*)&serveraddr, sizeof(serveraddr));
	  if(e < 0){
	    printf("There was an error with connecting to server.\n");
	    printf("Resetting to IP input...\n");
	   }
        }

	//send data
	printf("Enter the filename you wish to recieve:  ");
	scanf("%s",fname);

	printf("Enter a name for the new file:  ");
	scanf("%s",nname);

	// added one for the null char at the end
	// strlen is a poor way to check non strings lol
	send(sockfd, fname,strlen(fname)+1,0);
	
	//recieve echo
		int recieved = 0;
		int len = sizeof(serveraddr);
		int clientsocket = accept(sockfd, (struct sockaddr*) &serveraddr, &len);

		char *buff = (char*)malloc(4096);
		char *position = buff;
		FILE *file = fopen(nname, "w");
		int bytes = 0;

		while( (bytes = recv(sockfd,position,4096,0)) > 0){
			fwrite(position, 1, bytes, file);
		}

		free(buff);
		fclose(file);
	close(clientsocket);
}

