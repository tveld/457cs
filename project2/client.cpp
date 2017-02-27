// UDP Reliability Project
// Troy Veldhuizen
// Matt Noblett
// Matt Pairitz
// client.cpp

#include <iostream>
#include <queue>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;

int seqNum = 0;

int totalPackets = 0;
int packetCounter = 0;

int totalBytes = 0;
int bytesWritten = 0;

FILE* fd;

bool receiving = true;

int sockfd;
struct sockaddr_in serveraddr;

struct packet {
    int seqNum; //the sequence number of the packet
    char data[1000]; // the data portion of the packet
};

queue<int> windowQueue;
map<int, packet> outOfOrder;

int nextSeq(){
    seqNum = (seqNum % 9) + 1;
    return seqNum;
}

void fillWindow(){
    while(windowQueue.size() < 5){
        windowQueue.push(nextSeq());
    }
}

void sendAck(int seqNum){

    printf("Sent Packet Ack: %d\n", seqNum);
    int sendSeq = htonl(seqNum);
    sendto(sockfd, &sendSeq, sizeof(sendSeq), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
}

void checkOutOfOrder() {
    // check if front element is key in map
    if (outOfOrder.count(windowQueue.front()) == 1) {
        int write;
        if ((totalBytes - bytesWritten) >= 1000) {
            write = 1000;
        } else {
            write = totalBytes - bytesWritten;
        }

        bytesWritten += write;
        ++packetCounter;

        printf("Out of order packet recieved %d\n", outOfOrder.at( windowQueue.front()).seqNum);
        fwrite(outOfOrder.at(windowQueue.front()).data, 1, write, fd);

        sendAck(windowQueue.front());
        outOfOrder.erase(windowQueue.front());
        windowQueue.pop();

        fillWindow();

        checkOutOfOrder();
    }
}

void recvFile(){



    receiving = true;
    while(receiving){

        packet tempPacket;

        int len = sizeof(serveraddr);
        int err = recvfrom(sockfd, &tempPacket, sizeof(tempPacket), 0, (struct sockaddr*)&serveraddr, (socklen_t *) &len);

        if(err != -1) {

            // check if in order
            if (tempPacket.seqNum == windowQueue.front()) {
                int write;
                if ((totalBytes - bytesWritten) >= 1000) {
                    write = 1000;
                } else {
                    write = totalBytes - bytesWritten;
                }
                printf("Packet recieved %d\n", tempPacket.seqNum);
                fwrite(tempPacket.data, 1, write, fd);


                bytesWritten += write;
                sendAck(tempPacket.seqNum);
                ++packetCounter;

                windowQueue.pop();
                fillWindow();

                // check if any in out of order queue is next seq
                checkOutOfOrder();

                if (packetCounter == totalPackets) {
                    fclose(fd);
                    printf("Exiting program\n");
                    receiving = false;
                }
            } else {
                printf("Out of order packet: %d\n", tempPacket.seqNum);
                // store in map
                outOfOrder[tempPacket.seqNum] = tempPacket;
            }
        }
    }

}

int main() {
    sockfd = socket(AF_INET, SOCK_DGRAM,0);
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

    // recieve file size
    int size = 0;
    int len = sizeof(serveraddr);
    recvfrom(sockfd, &size, sizeof(size), 0, (struct sockaddr*)&serveraddr, (socklen_t *) &len);
    totalBytes = ntohl(size);

    //calculate total number of packets we need recieved
    totalPackets = totalBytes / 1000;

    if ((totalBytes % 1000) != 0) {
        ++totalPackets;
    }

    // make new file
    fd = fopen(nname, "w");

    // pre fill window with expected packet
    fillWindow();

    // recieve the file
    recvFile();

    printf("Bytes written: %d\n", bytesWritten);
    return 0;
}
