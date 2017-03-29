#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <string.h>
#include <netinet/in.h>
#include <cstdlib> 
#include <linux/ip.h>


struct arpheader {
	unsigned short int   hardware_type;
	unsigned short int   protocol_type;
	unsigned char        hardware_addr_length;
	unsigned char        protocol_addr_length;
	unsigned short int   op;
	unsigned char        sha[6];
	unsigned char        spa[4];
	unsigned char        dha[6];
	unsigned char        dpa[4];
};

int main(){  
	int packet_socket;
	unsigned char* ifeth1addr;
	//get list of interfaces (actually addresses)
	struct ifaddrs *ifaddr, *tmp;


	if(getifaddrs(&ifaddr)==-1){
		perror("getifaddrs");
		return 1;
	}

	//have the list, loop over the list
	for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
		//Check if this is a packet address, there will be one per
		//interface.  There are IPv4 and IPv6 as well, but we don't care
		//about those for the purpose of enumerating interfaces. We can
		//use the AF_INET addresses in this list for example to get a list
		//of our own IP addresses
		if(tmp->ifa_addr->sa_family==AF_PACKET){
			//create a packet socket on interface r?-eth1
			if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
				printf("Creating Socket on interface %s\n",tmp->ifa_name);
				//create a packet socket
				//AF_PACKET makes it a packet socket
				//SOCK_RAW makes it so we get the entire packet
				//could also use SOCK_DGRAM to cut off link layer header
				//ETH_P_ALL indicates we want all (upper layer) protocols
				//we could specify just a specific one
				packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
				if(packet_socket<0){
					perror("socket");
					return 2;
				}
				//Bind the socket to the address, so we only get packets
				//recieved on this specific interface. For packet sockets, the
				//address structure is a struct sockaddr_ll (see the man page
				//for "packet"), but of course bind takes a struct sockaddr.
				//Here, we can use the sockaddr we got from getifaddrs (which
				//we could convert to sockaddr_ll if we needed to)
				if(bind(packet_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
					perror("bind");
				}

				struct sockaddr_ll *eth1  = (struct sockaddr_ll *) tmp->ifa_addr;
				ifeth1addr = (unsigned char *) eth1->sll_addr; 
			}
		}
	}
	//free the interface list when we don't need it anymore
	//freeifaddrs(ifaddr);

	//loop and recieve packets. We are only looking at one interface,
	//for the project you will probably want to look at more (to do so,
	//a good way is to have one socket per interface and use select to
	//see which ones have data)
	printf("Ready to recieve now\n");

	// recv loop
	while(1){
		char buf[1500];
		struct sockaddr_ll recvaddr;
		socklen_t recvaddrlen=sizeof(struct sockaddr_ll);
		//we can use recv, since the addresses are in the packet, but we
		//use recvfrom because it gives us an easy way to determine if
		//this packet is incoming or outgoing (when using ETH_P_ALL, we
		//see packets in both directions. Only outgoing can be seen when
		//using a packet socket with some specific protocol)
		int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
		//ignore outgoing packets (we can't disable some from being sent
		//by the OS automatically, for example ICMP port unreachable
		//messages, so we will just ignore them here)
		if(recvaddr.sll_pkttype==PACKET_OUTGOING)
			continue;

		//start processing all others
		printf("Got a %d byte packet\n", n);

		// extract Ethernet information
		char* tempEth;
		unsigned short tempType;
		struct ether_header *eth = (struct ether_header*)buf;


		//
		if(ntohs(eth->ether_type)==0x0806){ 
			/*
			tempEth=ether_ntoa((struct ether_addr*) &eth->ether_dhost);
			printf("Destination address: %s\n", tempEth);
			tempEth=ether_ntoa((struct ether_addr*) &eth->ether_shost);
			printf("Source address: %s\n", tempEth);
			printf("Type: %04hx\n", ntohs(eth->ether_type));
			int size = sizeof(eth->ether_dhost)+sizeof(eth->ether_shost)+sizeof(eth->ether_type);
			printf("Size of Eth header: %d\n", size);	
			*/
		
			struct arpheader arp;

			memcpy(&arp, &buf[sizeof(struct ether_header)], sizeof(struct arpheader));

			printf("SENDER MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
				arp.sha[0],
				arp.sha[1],
				arp.sha[2],
				arp.sha[3],
				arp.sha[4],
				arp.sha[5]
			);

			printf("SENDER IP address: %02d:%02d:%02d:%02d\n",
				arp.spa[0],
				arp.spa[1],
				arp.spa[2],
				arp.spa[3]
			); 

			unsigned char routerMac[6];


			printf("Router MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
				ifeth1addr[0],
				ifeth1addr[1],
				ifeth1addr[2],
				ifeth1addr[3],
				ifeth1addr[4],
				ifeth1addr[5]
			);
			// setup responce packet

			//arp
			arp.op=(htons(2));
			memcpy(arp.dha, arp.sha, 6);
			memcpy(arp.sha, ifeth1addr, 6);

			// protocal addrs
			unsigned char tmpaddr[4];
			memcpy(tmpaddr, arp.dpa, 4);
			memcpy(arp.dpa, arp.spa, 4);
			memcpy(arp.spa, tmpaddr, 4);

			// ether

			//memcpy(eth->ether_dhost, recvaddr.sll_addr, 6);

			memcpy(eth->ether_dhost, eth->ether_shost, 6);

			memcpy(eth->ether_shost, ifeth1addr, 6);
			// add to buffer

			printf("ether saddr: %02X:%02X:%02X:%02X:%02X:%02X\n",
			eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2],
			eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5]);


			printf("ether daddr: %02X:%02X:%02X:%02X:%02X:%02X\n",
			eth->ether_dhost[0], eth->ether_dhost[1], eth->ether_dhost[2],
			eth->ether_dhost[3], eth->ether_dhost[4], eth->ether_dhost[5]);


			char responce[42];
			memcpy(responce, eth, 14);
			memcpy(&responce[14], &arp, 28);

			printf("Size of eth + arp is: %d\n", (int) sizeof(responce));

			int b = send(packet_socket, responce, 42, 0);

			printf("%d bytes sent back\n==========================\n", b);
			
		} else {
			printf("I've got an ICMP packet");
			
			struct ether_header *eth_1;
			struct iphdr *ip_1;
			struct icmphdr *icmp_1;

			memcpy(&eth_1, &buf, sizeof(struct ether_header*));

			memcpy(&ip_1, &buf[sizeof(eth_1)], sizeof(struct iphdr*));

			memcpy(&icmp_1, &buf[sizeof(ip_1)], sizeof(struct icmphdr*));

			memcpy(eth_1->ether_dhost, eth_1->ether_shost, 6);
			memcpy(eth_1->ether_shost, ifeth1addr, 6);

			memcpy(ip_1->daddr,ip_1->saddr, 4);

			
			
		}
		//what else to do is up to you, you can send packets with send,
		//just like we used for TCP sockets (or you can use sendto, but it
		//is not necessary, since the headers, including all addresses,
		//need to be in the buffer you are sending)

	} // end of while(1)
	//exit
	return 0;
}
