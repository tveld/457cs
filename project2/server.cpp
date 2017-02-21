#include <iostream>
#include <queue>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>

using namespace std;

#define ENDOFFILE -1
#define TIMEOUT 1
#define REG 0

int totalPackets = 0;   // total packets we need to successfully send
int packetsInMem = 0;  // total packets  sent
int ackCounter = 0;     // total packets acknowledged
int seqNum = 0;         // current sequence number to send
bool receiving = true;  // true if currently recieving
int sockfd;
struct sockaddr_in serveraddr;
struct  sockaddr_in clientaddr;
FILE* fd;               // file we read from

struct packet {
    int seqNum;         //the sequence number of the packet
    char data[1000];    // the data portion of the packet
};

queue<packet> windowQueue;

int nextSeq(){
    seqNum = (seqNum % 9) + 1;
    return seqNum;
}

packet buildPacket(){
    packet pkt;
    pkt.seqNum = nextSeq();
    fread(pkt.data, 1, 1000, fd);
    return pkt;
}

void sendPacket(packet sendPack){
    printf("Send Packet: %d\n", sendPack.seqNum);
    int bytes = sendto(sockfd, &sendPack, sizeof(sendPack), 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
    if( bytes < 0){
        printf("Error while attempting to send packet.");
    }
}

void fillWindow(){
    while(windowQueue.size() < 5){
        if(packetsInMem < totalPackets) {
            packet temp = buildPacket();
            windowQueue.push(temp);
            sendPacket(temp);
            ++packetsInMem;
        } else {
            return;
        }
    }
    return;
}

void reSendWindow(){
    int size = windowQueue.size();
    while(size >0){
        packet sendPack = windowQueue.front();
        printf("Re-Send Packet: %d\n", sendPack.seqNum);
        sendto(sockfd, &sendPack, sizeof(sendPack), 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
        windowQueue.push((packet &&) windowQueue.front());
        windowQueue.pop();
    }
}

int recvAck(){
    int seq = 0;
    int len = sizeof(clientaddr);
    bool recvingAck = true;

    while(recvingAck) {
        int err = recvfrom(sockfd, &seq, sizeof(seq), 0, (struct sockaddr *) &clientaddr, (socklen_t *) &len);

        // Check for timeout case
        if(err != -1) {
            int seqNum = ntohl(seq);
            //printf("Recieved Packet: %d\n", seqNum);
            // Check if we can move window
            if (seqNum == windowQueue.front().seqNum) {
                printf("Acknowledged Packet: %d\n", seqNum);
                windowQueue.pop();
                ++ackCounter;
                // Check if we have acknowledged all packets
                if(ackCounter == totalPackets){
                    // we have acknowledge all packets
                    return ENDOFFILE;
                }
                // regular return from function
                return REG;
            } else {
                // store in bucket for out of order
            }
        } else {
            //timeout
            recvingAck = TIMEOUT;
        }
    }
}

int main() {
    sockfd = socket(AF_INET, SOCK_DGRAM,0);
    if(sockfd<0){
        printf("There was an error creating the socket'n");
        return 1;
    }

    // Get timeout period from user input
    printf("\nHow many seconds before timeout?  ");
    int time;
    int errTime = scanf("%d", &time);
    if(errTime < 0){
        printf("Error getting the timeout information\n");
    }

    // set timeout period
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    // set socket to timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&timeout, sizeof(timeout));

    // Get port from user input
    printf("\nWhat port do you want to send to?  ");
    int port;
    int errPort = scanf("%d", &port);
    if(errPort < 0){
        printf("Error getting the port number\n");
    }

    // setup server port
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(9333);
    serveraddr.sin_addr.s_addr= INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));


    // Recieve FName
    char fname[5000];
    int len = sizeof(clientaddr);

    int err;
    do {
        err = recvfrom(sockfd, fname, 5000, 0, (struct sockaddr *) &clientaddr, (socklen_t *) &len);
    } while (err == -1);

    if (fname == "Exit") {
        // Exit program
        return 0;
    }
    printf("Filename %s\n", fname);

    // open file stream
    // fetch the file if it exists
    fd = fopen(fname, "r");

    struct stat st;
    stat(fname, &st);

    //calculate total number of packets we need sent and acknowledged
    totalPackets = st.st_size / 1000;

    if ((st.st_size % 1000) != 0) {
        ++totalPackets;
    }

    printf("File size: %d\n", st.st_size);
    printf("Total packets: %d\n", totalPackets);

    receiving = true;

    // send over initial responce



    //send file
    while (receiving) {

        fillWindow();
        int rtrn = recvAck();

        if (rtrn == ENDOFFILE) {
            receiving = false;
        } else if (rtrn == TIMEOUT) {
            //reSendWindow(sockfd, clientaddr);
        }
    }

    printf("File transfer complete\n");

}