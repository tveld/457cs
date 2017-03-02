// UDP Reliability Project
// Troy Veldhuizen
// Matt Noblett
// Matt Pairitz
// server.cpp

#include <iostream>
#include <queue>
#include <set>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>

using namespace std;

#define ENDOFFILE -2
#define TIMEOUT -1
#define REG 0

int totalPackets = 0;   // total packets we need to successfully send
int packetsInMem = 0;   // total packets  sent
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
set<int> outOfOrder;

int nextSeq(){
    seqNum = (seqNum % 10) + 1;
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

void reSend(){
	packet sendPack = windowQueue.front();
	printf("Re-Send Packet: %d\n", sendPack.seqNum);
	sendPacket(sendPack);
}

void checkOutOfOrder(){
    // check if front element is in the set
    if(outOfOrder.count(windowQueue.front().seqNum) == 1){
        printf("Acknowledged Packet: %d\n", windowQueue.front().seqNum);
        outOfOrder.erase(windowQueue.front().seqNum);
        windowQueue.pop();
        ++ackCounter;
		printf("Number of Packets: %d\n", ackCounter);
        checkOutOfOrder();
    }
}

int recvAck(){
    int seq = 0;
    int len = sizeof(clientaddr);
    bool recvingAck = true;

    while(recvingAck) {

	//printf("Before timeout\n");
    int err = recvfrom(sockfd, &seq, sizeof(seq), 0, (struct sockaddr *) &clientaddr, (socklen_t *) &len);
	printf("Return status: %d\n", err);

        // Check for timeout case
        if(err != -1) {
            int seqNum = ntohl(seq);
            //printf("Recieved Packet: %d\n", seqNum);
            // Check if we can move window
            if (seqNum == windowQueue.front().seqNum) {
                printf("Acknowledged Packet: %d\n", seqNum);
                windowQueue.pop();
                ++ackCounter;
				printf("Number of Packets: %d\n", ackCounter);
                checkOutOfOrder();

                // Check if we have acknowledged all packets
                if(ackCounter == totalPackets){
                    // we have acknowledge all packets
                    return -2;//-2 is end of file
                }

                // regular return from function
                return 0;
            } else {
                // store in queue for out of order
                outOfOrder.insert(seq);
				return 0;
            }
        } else {
            //timeout
	    	printf("Timeout.\n");
           	return -1;
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
    timeout.tv_sec = time;
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

    // validate port number
    if(port < 1024 || port > 49000){
        printf("This is not a valid port.");
        printf("  Please pick a port in the range of 1024 to 4900.\n");
        return 1;
    }

    // setup server port
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(port);
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

    int size = htonl(st.st_size);

    // send over initial size

    sendto(sockfd, &size, sizeof(size), 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
	
    //send file
    while (receiving) {

        fillWindow();
        int rtrn = recvAck();

        if (rtrn == -2) {
            receiving = false;
        }

		if (rtrn == -1) {
			printf("Calling resend window\n");
            reSend();
        }
    }

    printf("File transfer complete\n");

}
