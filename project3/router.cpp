#include <netpacket/packet.h> 
#include <inttypes.h>
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
#include <linux/icmp.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <arpa/inet.h>

using namespace std;

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



			uint16_t checksum(char* resp, int size_of_packet){
				
				uint32_t acc = 0xffff;
	
				for(size_t i=34; i+1<size_of_packet; i+=2){
					uint16_t word;
					memcpy(&word, resp+i, 2);
					acc+=ntohs(word);
					if(acc>0xffff){
						acc -=0xffff;
					}
				}

				if(size_of_packet&1) {
					uint16_t word=0;
					memcpy(&word, resp+size_of_packet-1, 1);
					acc+=ntohs(word);
					if (acc>0xffff){
						acc-=0xffff;
					}
				}

			return htons(~acc);
		 }






			// socket number
			// eth source address
			// ip source address
			// ip destination address
void send_arp_request(
	int packet_socket, 
	unsigned char *ethsaddr, 
	unsigned char *ipsaddr, 
	unsigned char *ipdaddr){
	

	struct ether_header eth;
	struct arpheader arp;
	unsigned char ethdaddr[6];
	// setup broadcast mac
	ethdaddr[0] = 255;
	ethdaddr[1] = 255;
	ethdaddr[2] = 255;
	ethdaddr[3] = 255;
	ethdaddr[4] = 255;
	ethdaddr[5] = 255;
	// build ether header
	memcpy(eth.ether_dhost, ethdaddr, 6);
	memcpy(eth.ether_shost, ethsaddr, 6);
		
	eth.ether_type = htons(0x0806); //arp
	
	// build arp header
	arp.hardware_type = htons(1); // ether
	arp.protocol_type = htons(2048); // IP
	arp.hardware_addr_length = 6; // mac addr
	arp.protocol_addr_length = 4; // ip addr
	arp.op = htons(1); //arp request
	
	//ether addrs
	memcpy(arp.dha, ethdaddr, 6);
	memcpy(arp.sha, ethsaddr, 6);

	// ip addrs
	memcpy(arp.dpa, ipdaddr, 4);
	memcpy(arp.spa, ipsaddr, 4);

	// add headers into the buffer
		
	char responce[42];
	memcpy(responce, &eth, 14);
	memcpy(&responce[14], &arp, 28);

	printf("Size of arp request is: %d\n", (int) sizeof(responce));


	int b = send(packet_socket, responce, 42, 0);

	printf("%d bytes sent back\n==========================\n", b);


}



void send_arp_reply(
	unsigned char* ifethaddr, 
	struct ether_header *eth, 
	struct arpheader arp, 
	int packet_socket, 
	char *buf){
	
	// setup responce packet

	//arp
	arp.op=(htons(2));
	memcpy(arp.dha, arp.sha, 6);
	memcpy(arp.sha, ifethaddr, 6);

	// protocal addrs
	unsigned char tmpaddr[4];
	memcpy(tmpaddr, arp.dpa, 4);
	memcpy(arp.dpa, arp.spa, 4);
	memcpy(arp.spa, tmpaddr, 4);

	// ether

	//memcpy(eth->ether_dhost, recvaddr.sll_addr, 6);

	memcpy(eth->ether_dhost, eth->ether_shost, 6);

	memcpy(eth->ether_shost, ifethaddr, 6);
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

}

int main(){
	int router_socket = 0;  
	int packet_socket = 0;
	struct sockaddr_ll *eth;
	unsigned char* ifethaddr;
	unsigned char* ifipaddr;
	string iname, router_iname;

	// get list of interfaces (actually addresses)
	struct ifaddrs *ifaddr, *tmp;
	// hash map to hold table info
	// key: prefix   first: next hop   second: interface name
	unordered_map<uint32_t, pair<string, string>> rmap;
	
	// hash map to hold interface info
	// key: interface name first:socket second: mac third: ip
	unordered_map<string, tuple<int, unsigned char *, unsigned char *>> imap;

	// hash map to hold packets waiting to be send
	// WIP

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

		// get interface name
		iname = tmp->ifa_name;
		
		// create entry in hash map
		imap[iname];
		
		// get IP addresses
		if(tmp->ifa_addr->sa_family==AF_INET){
				
				struct sockaddr_in *ip = (struct sockaddr_in*) tmp->ifa_addr;
				ifipaddr = (unsigned char *) &(ip->sin_addr.s_addr);

					// add into interface map
					get<2>(imap[iname]) = ifipaddr;
	
					printf("\n\n\tInterface: %s\n", iname.c_str());
					printf("Socket: %d\n", get<0>(imap[iname]));
					printf("MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
					get<1>(imap[iname])[0],
					get<1>(imap[iname])[1],
					get<1>(imap[iname])[2],
					get<1>(imap[iname])[3],
					get<1>(imap[iname])[4],
					get<1>(imap[iname])[5]
					);

					printf("IP address: %02d:%02d:%02d:%02d\n",
					get<2>(imap[iname])[0],
					get<2>(imap[iname])[1],
					get<2>(imap[iname])[2],
					get<2>(imap[iname])[3]
					);		
		}

		if(tmp->ifa_addr->sa_family==AF_PACKET){
			//create a packet socket on interface r?-eth1
			//if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
				//printf("Creating Socket on interface %s\n",tmp->ifa_name);
				//create a packet socket
				//AF_PACKET makes it a packet socket
				//SOCK_RAW makes it so we get the entire packet
				//could also use SOCK_DGRAM to cut off link layer header
				//ETH_P_ALL indicates we want all (upper layer) protocols
				//we could specify just a specific one
				if(!strncmp(&(tmp->ifa_name[3]), "eth0",4)){
					packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
					get<0>(imap[iname]) = packet_socket;
					if(bind(packet_socket, tmp->ifa_addr, sizeof(struct sockaddr_ll))== -1){ 
						perror("bind");
					}
				}
				
				if(!strncmp(&(tmp->ifa_name[3]), "eth2", 4)){
					packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
					get<0>(imap[iname]) = packet_socket;
				
					if(bind(packet_socket, tmp->ifa_addr, sizeof(struct sockaddr_ll)) == -1){
						perror("bind");
					}
				}

				if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
					packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
					if(packet_socket<0){
						perror("socket");
						return 2;
					}
				
					router_socket = packet_socket;
				//Bind the socket to the address, so we only get packets
				//recieved on this specific interface. For packet sockets, the
				//address structure is a struct sockaddr_ll (see the man page
				//for "packet"), but of course bind takes a struct sockaddr.
				//Here, we can use the sockaddr we got from getifaddrs (which
				//we could convert to sockaddr_ll if we needed to)
			

					if(bind(packet_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
						perror("bind");
					}

					// set router iname
					router_iname = iname;
			
					// store in imap
					get<0>(imap[iname]) = packet_socket;
	
					// read in router
					if(!strncmp(&(tmp->ifa_name[0]),"r1", 2)){
						printf("\tRouter Table 1\n");
						ifstream rtable("r1-table.txt");
						string line;
						string src,prestr, dest, iface;
						unsigned char pre[3];
						while(getline(rtable, line)){
							stringstream ss(line);
							
							while(ss >> src){
								ss >> dest;
								ss >> iface;
								prestr = src.substr(0,8).c_str();	
								printf("Source addr in string form: %s\n", prestr.c_str());
								uint32_t addr = inet_addr(prestr.c_str());
								rmap[addr].first = dest;
								rmap[addr].second = iface;
								printf("Prefix: %d\n",addr);
								printf("Dest: %s\n", rmap[addr].first.c_str());
								printf("Iface: %s\n", rmap[addr].second.c_str());
							}	
						//printf("%s \n", line.c_str());
						}
				} else if(!strncmp(&(tmp->ifa_name[0]), "r2", 2)){
					printf("On router 2\n");
				} else {
					printf("You are not on a mininet router! Could not open routing table.\n");
				}
		
			}// close if(r1-eth1)
		
				
	
					// get mac address	
					eth  = (struct sockaddr_ll *) tmp->ifa_addr;
					ifethaddr = (unsigned char *) eth->sll_addr; 
					
					get<1>(imap[iname]) = ifethaddr;			
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
		int n = recvfrom(router_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
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


		//if arp
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

	
			printf("Destination IP address: %02d:%02d:%02d:%02d\n",
				arp.dpa[0],
				arp.dpa[1],
				arp.dpa[2],
				arp.dpa[3]
			); 

			unsigned char routerMac[6];


			
			printf("Router MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
				get<1>(imap[router_iname])[0],
				get<1>(imap[router_iname])[1],
				get<1>(imap[router_iname])[2],
				get<1>(imap[router_iname])[3],
				get<1>(imap[router_iname])[4],
				get<1>(imap[router_iname])[5]
			);


			printf("Router IP address: %02d:%02d:%02d:%02d\n",
				get<2>(imap[router_iname])[0],
				get<2>(imap[router_iname])[1],
				get<2>(imap[router_iname])[2],
				get<2>(imap[router_iname])[3]
			);
	
			// if arp reply
			if(ntohs(arp.op) == 2){
				printf("I've got an ARP reply\n");


				// arp request
			} else if(ntohs(arp.op) == 1){
				printf("I've got an ARP request\n");
				
				// arp request is for me
				if(arp.dpa[0] == get<2>(imap[router_iname])[0] &&
					 arp.dpa[1] == get<2>(imap[router_iname])[1] &&
					 arp.dpa[2] == get<2>(imap[router_iname])[2] &&
					 arp.dpa[3] == get<2>(imap[router_iname])[3]){
 
					printf("Sending reply for arp header\n");
					send_arp_reply(get<1>(imap[router_iname]), eth, arp, get<0>(imap[router_iname]), buf);
				
				// arp request is for another host / router
				} else {
					printf("Sending arp request to find mac for next hop\n");
					// look up hash for interface
					// send_arp_request(poop_dollar, ifethaddr, ifipaddr, arp.dpa); 


				}
			}
			
			// ICMP packet
		} else {

			printf("I've got an ICMP packet\n");
			
			struct ether_header eth_1;
			struct iphdr ip_1;
			struct icmphdr icmp_1;

			memcpy(&eth_1, &buf, sizeof(struct ether_header));

			memcpy(&ip_1, &buf[sizeof(struct ether_header)], sizeof(struct iphdr));

			memcpy(&icmp_1, &buf[sizeof(struct ether_header) + sizeof(struct iphdr)], sizeof(struct icmphdr));

			unsigned char* packetaddr = (unsigned char *) &ip_1.daddr;
			unsigned char* routeraddr = get<2>(imap[router_iname]);
			if(packetaddr[0] == routeraddr[0] &&
				 packetaddr[1] == routeraddr[1] &&
				 packetaddr[2] == routeraddr[2] &&
				 packetaddr[3] == routeraddr[3]){
				
				printf("ICMP packet for this router\n\n");
					
				memcpy(eth_1.ether_dhost, eth_1.ether_shost, 6);
				memcpy(eth_1.ether_shost, get<1>(imap[router_iname]), 6);
		
				uint32_t temp = ip_1.daddr;
				ip_1.daddr = ip_1.saddr;
				ip_1.saddr = temp;
			
				printf("Incoming Checksum: %04x\n", ntohs(icmp_1.checksum));
				icmp_1.checksum = 0;
				icmp_1.type = 0;
			
				char resp[98];

				memcpy(&resp, &eth_1, sizeof(struct ether_header));
				memcpy(&resp[sizeof(struct ether_header)], &ip_1, sizeof(struct iphdr));
				memcpy(&resp[sizeof(struct ether_header) + sizeof(struct iphdr)], &icmp_1, sizeof(struct icmphdr));
			
				icmp_1.checksum = checksum(resp, sizeof(resp));
				memcpy(&resp[sizeof(struct ether_header) + sizeof(struct iphdr)], &icmp_1, sizeof(struct icmphdr));

				// copy data portion
				memcpy(&resp[42], &buf[42], 56);

				printf("Calculated checksum: %04x\n", ntohs(icmp_1.checksum));
				int c = send(get<0>(imap[router_iname]), resp, 98, 0);
				printf("Bytes sent: %d\n", c);	

		} else {

			printf("\nICMP for a different router/host\n");
			
			printf("Sending ARP request\n");
			unsigned char* ip_daddr =  (unsigned char*)&ip_1.daddr;
			unsigned char pre[3];

			pre[0] = ip_daddr[0];
			pre[1] = ip_daddr[1];
			pre[2] = ip_daddr[2];
			pre[3] = 0;
	
			printf("%02d : %02d : %02d\n", pre[0], pre[1], pre[2]);
	
			uint32_t prefix = *(uint32_t *)pre;
			// find interface in router table
			printf("Interface %s\n", rmap[prefix].second.c_str());
			printf("Interface r1-eth0 IP: %02d : %02d : %02d : %02d\n", 
					get<2>(imap["r1-eth0"])[0],
					get<2>(imap["r1-eth0"])[1],
					get<2>(imap["r1-eth0"])[2],
					get<2>(imap["r1-eth0"])[3]
			);
			printf("Interface r1-eth1 IP: %02d : %02d : %02d : %02d\n", 
					get<2>(imap["r1-eth1"])[0],
					get<2>(imap["r1-eth1"])[1],
					get<2>(imap["r1-eth1"])[2],
					get<2>(imap["r1-eth1"])[3]
			);
			printf("Interface r1-eth2 IP: %02d : %02d : %02d : %02d\n", 
					get<2>(imap["r1-eth2"])[0],
					get<2>(imap["r1-eth2"])[1],
					get<2>(imap["r1-eth2"])[2],
					get<2>(imap["r1-eth2"])[3]
			);

			// if next hop has value use it
			if(rmap[prefix].first != "-"){
				printf("Found next hop.\n");
				printf("IP: %s", rmap[prefix].first.c_str());
				uint32_t addr = inet_addr(rmap[prefix].first.c_str());
				ip_daddr = (unsigned char *) &addr;
			}
			// socket number
			// eth source address
			// ip source address
			// ip destination address
			send_arp_request(get<0>(imap[rmap[prefix].second]), 
											 get<1>(imap[rmap[prefix].second]),
											 get<2>(imap[rmap[prefix].second]),
											 ip_daddr
										);
			
		}
	}
		//what else to do is up to you, you can send packets with send,
		//just like we used for TCP sockets (or you can use sendto, but it
		//is not necessary, since the headers, including all addresses,
		//need to be in the buffer you are sending)

	} // end of while(1)
	//exit
	return 0;
}
