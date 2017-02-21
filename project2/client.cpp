#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
using namespace std;

int expSeq;
int maxSeq;
int totalPackets = 0;
int packetCounter = 0;

int totalBytes = 798916;
int bytesWritten = 0;

FILE* fd;

bool receiving = true;

struct packet {
    int seqNum; //the sequence number of the packet
    char data[1000]; // the data portion of the packet
};

void sendAck(int seqNum, int sockfd, sockaddr_in serveraddr){

    printf("Sent Packet Ack: %d\n", seqNum);
    int sendSeq = htonl(seqNum);
    sendto(sockfd, &sendSeq, sizeof(sendSeq), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
}

void recvFile(int sockfd, sockaddr_in serveraddr){
    receiving = true;
    while(receiving){
        packet tempPacket;

        int len = sizeof(serveraddr);
        int err = recvfrom(sockfd, &tempPacket, sizeof(tempPacket), 0, (struct sockaddr*)&serveraddr, (socklen_t *) &len);

        if(err != -1) {
            int write;
            if((totalBytes - bytesWritten) >= 1000){
                write = 1000;
            } else {
                write = totalBytes - bytesWritten;
            }
            printf("Packet recieved %d\n", tempPacket.seqNum);
            fwrite(tempPacket.data, 1, write, fd);

            bytesWritten += write;
            sendAck(tempPacket.seqNum, sockfd, serveraddr);
            ++packetCounter;

            //printf("Packet Counter: %d\n", packetCounter);

            if(packetCounter == 799){
                fclose(fd);
                printf("Exiting program\n");
                receiving = false;
            }
        }
    }

}

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM,0);
    if(sockfd<0){
        printf("There was an error creating the socket'n");
        return 1;
    }

    // Get port from user input
    printf("\nWhat port do you want to send to?  ");

    int port;
    int errPort = scanf("%d", &port);
    if(errPort < 0){
        printf("\nError getting the port number");
    }

    // validate port number
    if(port < 1024 || port > 49000){
        printf("This is not a valid port.");
        printf("  Please pick a port in the range of 1024 to 4900.\n");
        return 1;
    }

    // get the ip from user input
    printf("\nWhat ip do you want to send to?  ");

    char ip[5000];
    int errIp = scanf("%s", &ip);

    if(errIp < 0){
        printf("Error reading ip.\n");
    }

    struct sockaddr_in serveraddr;
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(port);
    serveraddr.sin_addr.s_addr=inet_addr(ip);


    //send data
    char fname[5000];
    printf("\nEnter the filename you wish to recieve:  ");
    scanf("%s",fname);

    char nname[5000];
    printf("\nEnter a name for the new file:  ");
    scanf("%s",nname);

    // send filename
    sendto(sockfd, fname, strlen(fname), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));


    // make new file
    fd = fopen(nname, "w");

    recvFile(sockfd, serveraddr);
    printf("Bytes written: %d\n", bytesWritten);
    return 0;
}