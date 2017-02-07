//GVSU CIS 457 - Data Communications
//TCP Server
//Troy Veldhuizen
//Matt Noblett
//Matt Pairitz
//Due 02/06/17
//tcpclient.c

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
  int port; // port number to send to
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
	
	// Get port from user input
	printf("What port do you want to send to?\n");
	
	int err = scanf("%d", &port);
	if(err < 0){
		printf("Error getting the port number\n");
	}

	// validate port number
	if(port < 1024 || port > 49000){
		printf("This is not a valid port.");
		printf("  Please pick a port in the range of 1024 to 4900.\n");
		return 1;
	}

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
	int e = connect(sockfd,(struct sockaddr*)&serveraddr, sizeof(serveraddr));

	if(e < 0){
		printf("There was an error with connecting to server.\n");
		return 1;
	}
    while(1){

        
	//send data
	printf("Enter the filename you wish to recieve, or 'Quit' to exit:  ");
	scanf("%s",fname);

    if(strcmp(fname, "Quit") == 0){
        send(sockfd, fname, strlen(fname)+1, 0);
        printf("Disconnecting from Server...");
        close(sockfd);
        exit(0);
    }

    if(strcmp(fname, "List") == 0 ){
        
        send(sockfd, fname, strlen(fname)+1, 0);
        int filecount = 0;
        recv(sockfd,  &filecount, sizeof(filecount), 0);
        int count = ntohl(filecount);
        while(count>0){
            recv(sockfd, fname, 5000, 0);
            printf("Files available for transfer  %s\n", fname);
            count--;
        }
    }


    else{

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
		
		// recieve size

		int size = 0;
		recv(sockfd, &size, sizeof(size), 0);
		
		
		int total = 0;	
		bytes = 0;
		
		while( total < ntohl(size)){
			//printf("bytes before: %d", bytes);

			bytes = recv(sockfd,position,4096,0);

			//printf("bytes after: %d", bytes);

			total += bytes;
			//printf("\ntotal bytes: %d\n", total);
			
			if(bytes > 0){
				fwrite(position, 1, bytes, file);
			} 
		}
		free(buff);
		fclose(file);
    }
}
}

