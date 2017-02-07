//GVSU CIS 457 - Data Communications
//TCP Server
//Troy Veldhuizen
//Matt Noblett
//Matt Pairitz
//Due 02/06/17
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
#include <dirent.h>

struct clientinfo{
	struct sockaddr_in addr;
	int socket;
};

void* recieve_message(void *arg){
		int run = 1;
		struct clientinfo ci = *(struct clientinfo *)arg;
		int clientsocket = ci.socket;
		char line[5000];
    char* filename;
		// e is the number of bytes recieved
		//while(run == 1){
      while(1){
            
			int e = recv(clientsocket,line,5000,0);
           // printf("Contents of Line: %s", line);
			if(e < 0){
				printf("Error recieving\n");
				return 1;
			}

                if(strcmp(line, "Quit") == 0){
                    printf("Client is disconnecting...\n");
                    close(clientsocket);
                }
                if(strcmp(line, "List") ==0){
                    DIR *d;
                    struct dirent *dir;
                    d = opendir(".");
                    int count=0;
                    if(d){
                        while((dir = readdir(d)) != NULL)
                        {
                            count++;
                        }
                    }
                    int filecount = htonl(count);
                    send(clientsocket, &filecount, sizeof(filecount), 0); 
                    rewinddir(d);
                    if(d){
                        while((dir = readdir(d)) != NULL)
                        {
                            // send byte buffer of files
                            filename = dir->d_name;
                            printf("sending file %s\n", filename);
                            send(clientsocket, filename, strlen(filename)+1, 0);
														usleep(20 * 1000);
                        }
                    }
                }
                else{
                    printf("\nGot from the client %d: %s\n", clientsocket, line);
            	

                // fetch file if it exists
						
                    int success = 1;
                    FILE *file;
			

                    int fd; //file descriptor
                    fd = open(line, O_RDONLY);
                    printf("Got file open\n");
			

                    // send file to the client
                
                    // get the size of the file
                    struct stat stat_buf;
                    fstat(fd, &stat_buf);
                    //printf("\nbytes: %d\n", stat_buf.st_size);
                
                    // send size
                    int size = htonl(stat_buf.st_size);
                    int senderr = send(clientsocket, &size, sizeof(size), 0);
										if (senderr == -1) {
												printf("Error sending file");
												exit(1);
										} else {
                    	printf("Sent size back\n");
										}
                    //printf("Send error : %d", senderr);
			
                    // use sendfile	
                    off_t offset = 0;
                    int rc = sendfile (clientsocket, fd, &offset, stat_buf.st_size);
                    //printf("Send file err: %d", rc);
                    printf("Sent File Back\n");
                    close(fd);
            }
        }
            
	
}


int main(int argc, char **argv){
	int sockfd = socket(AF_INET, SOCK_STREAM,0);
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
	
	listen(sockfd,10);

	// does not handle multiple clients concurrently
	while(1){

		int newsock = socket(AF_INET, SOCK_STREAM,0);

		int clientsocket = 0;
		int len = sizeof(clientaddr);
		//blocking call: waits for connection
		clientsocket = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
	
		if(clientsocket >= 0){
			printf("Connected to client %d\n", clientsocket);
		}
	
		struct clientinfo ci;
		ci.socket = clientsocket;
		ci.addr = clientaddr;
	
		// pthreads share the same memory and global variables.
		// solved by semephores and locking

		pthread_t childrec;
		pthread_create(&childrec, NULL, recieve_message, &ci);
		pthread_detach(childrec); // will clean up thread when function ends
}

}	
