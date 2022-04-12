#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <stdbool.h>
#include <pcap/pcap.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>	
#include <netinet/ether.h>	
#include <netinet/ip6.h>
#include <netinet/ip.h>
#include <time.h>
#include <arpa/inet.h>

//Error codes
#define INTERNAL_ERROR 99
#define ARGUMENT_ERROR 21
#define PCAP_ERROR_DEF 22

//Values for arguments with no short version
#define ARP 1000
#define ICMP_arg 2000

//Return values for when some values are NULL
#define INTERFACE_NULL 11

//Useful values
#define SNAPSHOT_LENGHT 65535
#define UDP 17
#define TCP 6
#define ICMP 1
#define ICMP6 58


typedef struct{
    int port;        //-p <port>
    int packet_num;  //-n <num>
    bool tcp;        //tcp flag
    bool udp;        //udp flag
    bool arp;        //arp flag
    bool icmp;       //icmp flag
}my_args;

struct packet_info{
    int packet_type;                //(UDP, TCP, ARP, ICMP)

    char src_ip[50];                //Source IP from ip_header
    char dst_ip[50];                //Destination IP from ip_header
    u_int8_t *src_mac;              //Source MAC address from ethernet header
    u_int8_t *dst_mac;              //Destination MAC address from ethernet header
    u_int32_t frame_len;            //Frame lenght
    u_int16_t src_port;             //Source port
    u_int16_t dst_port;             //Destination port

    //ICMP
    u_int8_t icmp_type;             //ICMP type 
    u_int8_t icmp_code;             //ICMP code

    //ARP
    u_int16_t arp_format_hw;        //Format of hardware address
    u_int16_t arp_format_pro;       //Format of protocol address
    u_int8_t *arphw_sender_mac;     //Sender hardware address
    u_int8_t *arphw_target_mac;     //Target hardware address
    char arp_sender_ip[16];         //Sender IP address
    char arp_target_ip[16];         //Target IP address
    unsigned char arp_len_hw;       //Lenght of hardware address
    unsigned char arp_len_pro;      //Lenght of protocol address
    uint16_t arp_opcode;            //ARP opcode
};


//Prototypes
int arg_parse(int argc, char *argv[], char *interface, my_args *args);
int show_active_interfaces();
int sniff_packets(char *interface, my_args *args);
void process_packet(u_char *args, const struct pcap_pkthdr *p_header, const u_char *p_body);
char *get_timestamp(long int mikro_secs);
struct packet_info process_tcp(const u_char *p_body, bool ipv6);
struct packet_info process_udp(const u_char *p_body, bool ipv6);
struct packet_info process_arp(const u_char *p_body);
struct packet_info process_icmp(const u_char *p_body, bool ipv6);
void print_info(char *timestamp, struct packet_info p_info);
void print_mac_addr(u_int8_t *address, char *type);
void print_data(const struct pcap_pkthdr *p_header, const u_char *p_body);



//MAIN-----------------------------------------------------------------
int main(int argc, char *argv[]){
    //Parameter processing
    char *interface = malloc(30 * sizeof(char));

    my_args args = {.port = -1,
                    .packet_num = 0,
                    .tcp = false,
                    .udp = false,
                    .arp = false,
                    .icmp = false};

    if(interface == NULL){
        fprintf(stderr, "Malloc Error\n");
        return INTERNAL_ERROR;
    }

    int rc = arg_parse(argc, argv, interface, &args); //Parse arguments
    if(rc == ARGUMENT_ERROR){
        free(interface);
        return rc;
    }
    else if(rc == INTERFACE_NULL){
        //IF no interface was specified, show all acitive network interfaces
        free(interface);
        rc = show_active_interfaces();
        return rc;
    }

    //If agrument parsing was succesfull, sniff packets
    rc = sniff_packets(interface, &args);

    free(interface);
    return rc;
}
//---------------------------------------------------------------------


//Parse arguments------------------------------------------------------
int arg_parse(int argc, char *argv[], char *interface, my_args *args){
    bool interface_bool = false;

    static struct option long_options[] = {
        {"interface", optional_argument, 0, 'i'},
        {"tcp", 0, 0, 't'},
        {"udp", 0, 0, 'u'},
        {"arp", 0, 0, ARP},
        {"icmp", 0, 0, ICMP_arg},
    };

    int c;
    while((c = getopt_long(argc, argv, "i::p:n:tuh", long_options, NULL)) != EOF){
        switch(c){
            case 'i':
                if(optarg == NULL && optind < argc && argv[optind][0] != '-') optarg = argv[optind++];
                if(optarg != NULL){
                    if(strlen(optarg) > 29){
                        fprintf(stderr, "Interface parameter cannot be longer than 29 characters!\n");
                        return ARGUMENT_ERROR;
                    }
                    strcpy(interface, optarg);
                    interface_bool = true;
                }
                break;

            case 'p':
                //Check if given arguments is a whole number
                for(int i = 0; optarg[i] != '\0'; i++){ 
                    if(!isdigit(optarg[i])){
                        fprintf(stderr, "Port has to be a whole positive number!\n");
                        return ARGUMENT_ERROR;
                    }
                }

                //Check if the port is within limits
                args->port = atoi(optarg);
                if(args->port < 0 || args->port > 65535){ 
                    fprintf(stderr, "Port has to be in range of 0 - 65535!\n");
                    return ARGUMENT_ERROR;
                }
                break;

            case 'n':
                //Check if given arguments is a whole number
                for(int i = 0; optarg[i] != '\0'; i++){ 
                    if(!isdigit(optarg[i])){
                        fprintf(stderr, "Number has to be a whole positive number!\n");
                        return ARGUMENT_ERROR;
                    }
                }

                args->packet_num = strtol(optarg, NULL, 10);
                break;

            case 't':
                args->tcp = true;
                break;

            case 'u':
                args->udp = true;
                break;

            case ARP:
                args->arp = true;
                break;

            case ICMP_arg:
                args->icmp = true;
                break;

            case 'h':
            default:
                puts("\nipk-sniffer.c is used for processing and filtering inbound and outbound packets and printing information about them.\n");
                puts("usage: [sudo] ./ipk-sniffer [-h] [-i | --interface <interface>] [-p <port>] [-t | --tcp] [-u | --udp]");
                puts("                            [--icmp] [--arp] [-n <num>]\n");
                puts("-h : prints help");
                puts("-i | --interface <interface> : specifies the interface, on which packets will be caught,");
                puts("                               if empty or missing, list of all active interfaces will be printed");
                puts("-p <port> : only packets with specified port number will be caught");
                puts("-t | --tcp : only tcp packets will be caught (ICMP and ARP will also be caught)");
                puts("-u | --udp : only udp packets will be caught (ICMP and ARP will also be caught)");
                puts("--icmp : only icmp packets will be caught (UDP and TCP will also be caught)");
                puts("--arp : only arp packets will be caught (UDP and TCP will also be caught)");
                puts("-n <num> : specifies the number of packets to be caught, default = 1");
                return ARGUMENT_ERROR;
        }   
    }

    if(args->packet_num == 0) args->packet_num = 1; //If no number was specified, use the dafault value (1)
    if(args->tcp == args->udp) args->tcp = args->udp = false; //If TCP and UDP were both true, set the to false (same functionality)
    if(interface_bool == false) return INTERFACE_NULL; //If no interface was specified, set it ot NULL

    return 0;
}
//---------------------------------------------------------------------



//Show network interfaces----------------------------------------------
int show_active_interfaces(){
    pcap_if_t *alldevsp;
    char errbuf[200];

    if(pcap_findalldevs(&alldevsp, errbuf) == PCAP_ERROR){ //Get all interfaces
        fprintf(stderr, "PCAP error: findalldevs: %s!\n", errbuf);
        return PCAP_ERROR_DEF;
    }

    while(alldevsp != NULL){ //Print interfaces
        if(alldevsp->description == NULL) printf("%s\n", alldevsp->name);
        else printf("%s : %s\n" , alldevsp->name , alldevsp->description);
        alldevsp = alldevsp->next;
    }

    return 0;
}
//---------------------------------------------------------------------


//Function for setting up sniffing-------------------------------------
int sniff_packets(char *interface, my_args *args){
    char errbuf[100];

    bpf_u_int32 netmask;
    bpf_u_int32 netw_num;

    //Find network address and mask
    if(pcap_lookupnet(interface, &netw_num, &netmask, errbuf) == PCAP_ERROR){
        fprintf(stderr, "PCAP error: pcap_lookupnet: %s!\n", errbuf);
        return PCAP_ERROR_DEF;
    }

    //Open device for sniffing
    pcap_t *handle = pcap_open_live(interface, SNAPSHOT_LENGHT, 0, 1000, errbuf); 
    if(handle == NULL){
        fprintf(stderr, "PCAP error: pcap_open_live: %s!\n", errbuf);
        return PCAP_ERROR_DEF;
    }

    //Filter Construction----------------------------------------------------
    char filter[50];
    struct bpf_program fp;    
    char port_s[15];
    if(args->port != -1) sprintf(port_s, "port %d", args->port); //If port was specified, make a filter contruction
    strcpy(filter, ""); //Inset NULL terminator, so strcat can be used


    if(args->tcp == args->udp){ 
        if(args->port == -1) strcat(filter, "tcp or udp"); //If both tcp and udp are false and port is not specified
        else strcat(filter, port_s); //Else if port was specified, insert just ("port #") -> translates to ("tcp or udp port #")
    }   
    else if(args->tcp == true) strcat(filter, "tcp ");
    else strcat(filter, "udp ");

    if(args->tcp != args->udp && args->port != -1) strcat(filter, port_s); //If only tcp or udp was specified, insert port into the filter

    if(args->arp == true) strcat(filter, " or arp ");
    if(args->icmp == true) strcat(filter, " or icmp or icmp6 ");
    if(args->icmp == false && args->arp == false) strcat(filter, " or arp or icmp or icmp6 ");
    //-----------------------------------------------------------------------

    if(pcap_compile(handle, &fp, filter, 0, netw_num) == PCAP_ERROR){
        fprintf(stderr, "PCAP error: pcap_compile: %s!\n", pcap_geterr(handle));
        return PCAP_ERROR_DEF;
    }

    if(pcap_setfilter(handle, &fp)){
        fprintf(stderr, "PCAP error: pcap_setfilter: %s!\n", pcap_geterr(handle));
        return PCAP_ERROR_DEF;
    }

    printf("FILTER: %s\n", filter);

    pcap_loop(handle, args->packet_num, process_packet, NULL); //Start the loop for catching packets, call process_pacekt for every packet
    return 0;
}
//---------------------------------------------------------------------


//Process every caught packet------------------------------------------
void process_packet(u_char *args, const struct pcap_pkthdr *p_header, const u_char *p_body){
    if(args) args = NULL; //Avoiding unused parameter error
    bool ipv6 = false;
    u_int8_t protocol;
    char *timestamp = get_timestamp(p_header->ts.tv_usec); //Get time of packet arrival
    struct packet_info p_info;

    struct ether_header *eth_header = (struct ether_header *) p_body; //Make the ether header struct
    int type = ntohs(eth_header->ether_type); //Get the type

    if(type == ETHERTYPE_IPV6){ //If its IPv6, set the ipv6 to true and get protocol
        struct ip6_hdr *ip_header = (struct ip6_hdr*)(p_body + sizeof(struct ether_header));
        protocol = ip_header->ip6_ctlun.ip6_un1.ip6_un1_nxt;
        ipv6 = true;
    }
    else if(type == ETHERTYPE_ARP) p_info = process_arp(p_body); //If its ARP process it
    else{ //If its ipv4 get its protocol
        struct ip *ip_header = (struct ip*)(p_body + sizeof(struct ether_header));
        protocol = ip_header->ip_p;
    }

    if(type != ETHERTYPE_ARP){
        switch(protocol){ //Call function based on protocol
            case TCP:
                p_info = process_tcp(p_body, ipv6);
                break;

            case UDP:
                p_info = process_udp(p_body, ipv6);
                break;

            case ICMP:
            case ICMP6:
                p_info = process_icmp(p_body, ipv6);
                break;
        }
    } 

    p_info.frame_len = p_header->len; //Get frame lenght
    p_info.src_mac = eth_header->ether_shost; //Get MAC adresses
    p_info.dst_mac = eth_header->ether_dhost;

    print_info(timestamp, p_info); //Print info from packet_info struct
    print_data(p_header, p_body); //Print data from packet

    return;
}
//---------------------------------------------------------------------


//Get arrival time of the packet---------------------------------------
char *get_timestamp(long int mikro_secs){
    struct timeval time_raw;
    struct tm * time_info;

    char *buffer = malloc(50*sizeof(char));
    char *result = malloc(50*sizeof(char));

    if(buffer == NULL || result == NULL){
        fprintf(stderr, "Malloc Error\n");
        exit(INTERNAL_ERROR);
    }

    gettimeofday(&time_raw, NULL); //Get the current time
    time_info = localtime (&time_raw.tv_sec);

    strftime(buffer, 50, "%FT%X", time_info);
    sprintf(result, "%s.%ld", buffer, mikro_secs/1000); //Add miliseconds

    free(buffer);
    return result;
}
//---------------------------------------------------------------------


//Process TCP packet---------------------------------------------------
struct packet_info process_tcp(const u_char *p_body, bool ipv6){
    struct packet_info info;
    info.packet_type = TCP;
    int header_len;

    if(ipv6){ //If its ipv6, use the ipv6 header struct
        struct ip6_hdr *ip_header = (struct ip6_hdr*)(p_body + sizeof(struct ether_header));
        header_len = 40;

        inet_ntop(AF_INET6, (void *)&ip_header->ip6_src, info.src_ip, 50); //Get ipv6 adresses
        inet_ntop(AF_INET6, (void *)&ip_header->ip6_dst, info.dst_ip, 50);
    }
    else{ //Else use ipv4 header struct
        struct ip *ip_header = (struct ip*)(p_body + sizeof(struct ether_header));
        header_len = ip_header->ip_hl * 4;

        struct in_addr src = ip_header->ip_src;
        strcpy(info.src_ip, inet_ntoa(src)); //Get IP adresses

        src = ip_header->ip_dst;
        strcpy(info.dst_ip, inet_ntoa(src));
    }

    struct tcphdr *tcp_header = (struct tcphdr*)(p_body + header_len + sizeof(struct ether_header)); //Use lenght from IP header to get TCP header

    info.src_port = ntohs(tcp_header->th_sport); //Get ports
    info.dst_port = ntohs(tcp_header->th_dport);

    printf("--------------------TCP--------------------\n");
    return info;
}
//---------------------------------------------------------------------


//Process UDP packet---------------------------------------------------
struct packet_info process_udp(const u_char *p_body, bool ipv6){
    //Same process as TCP
    struct packet_info info;
    info.packet_type = UDP;
    int header_len;

    if(ipv6){ 
        struct ip6_hdr *ip_header = (struct ip6_hdr*)(p_body + sizeof(struct ether_header));
        header_len = 40;

        inet_ntop(AF_INET6, (void *)&ip_header->ip6_src, info.src_ip, 50);
        inet_ntop(AF_INET6, (void *)&ip_header->ip6_dst, info.dst_ip, 50);
    }
    else{
        struct ip *ip_header = (struct ip*)(p_body + sizeof(struct ether_header));
        header_len = ip_header->ip_hl * 4;

        strcpy(info.src_ip, inet_ntoa(ip_header->ip_src));
        strcpy(info.dst_ip, inet_ntoa(ip_header->ip_dst));
    }

    struct udphdr *udp_header = (struct udphdr*)(p_body + header_len + sizeof(struct ether_header));

    info.src_port = ntohs(udp_header->uh_sport);
    info.dst_port = ntohs(udp_header->uh_dport);

    printf("--------------------UDP--------------------\n");
    return info;
}
//---------------------------------------------------------------------


//Process ARP packet---------------------------------------------------
struct packet_info process_arp(const u_char *p_body){
    struct packet_info info;
    info.packet_type = ETHERTYPE_ARP;

    struct ip *ip_header = (struct ip*)(p_body + sizeof(struct ether_header));
    struct arphdr *arp_header = (struct arphdr*)(p_body + (ip_header->ip_hl * 4) + sizeof(struct ether_header)); //Make structs for ARP
    struct ether_arp *etharp = (struct ether_arp*)(p_body + (ip_header->ip_hl * 4) + sizeof(struct ether_header));

    //Extract info
    info.arp_format_hw = ntohs(arp_header->ar_hrd);
    info.arp_format_pro = ntohs(arp_header->ar_pro);
    info.arp_len_hw = arp_header->ar_hln;
    info.arp_len_pro = arp_header->ar_pln;
    info.arp_opcode = ntohs(arp_header->ar_op);

    sprintf(info.arp_sender_ip, "%d.%d.%d.%d", etharp->arp_spa[0], etharp->arp_spa[1], etharp->arp_spa[2], etharp->arp_spa[3]);
    sprintf(info.arp_target_ip, "%d.%d.%d.%d", etharp->arp_tpa[0], etharp->arp_tpa[1], etharp->arp_tpa[2], etharp->arp_tpa[3]);
    info.arphw_sender_mac = etharp->arp_sha;
    info.arphw_target_mac = etharp->arp_tha;

    printf("--------------------ARP--------------------\n");
    return info;
}
//---------------------------------------------------------------------



//Process ICMP packet--------------------------------------------------
struct packet_info process_icmp(const u_char *p_body, bool ipv6){
    //Same process as TCP
    struct packet_info info;
    info.packet_type = ICMP;
    int header_len;

    if(ipv6){
        struct ip6_hdr *ip_header = (struct ip6_hdr*)(p_body + sizeof(struct ether_header));
        header_len = 40;

        inet_ntop(AF_INET6, (void *)&ip_header->ip6_src, info.src_ip, 50);
        inet_ntop(AF_INET6, (void *)&ip_header->ip6_dst, info.dst_ip, 50);
    }
    else{
        struct ip *ip_header = (struct ip*)(p_body + sizeof(struct ether_header));
        header_len = ip_header->ip_hl * 4;

        strcpy(info.src_ip, inet_ntoa(ip_header->ip_src));
        strcpy(info.dst_ip, inet_ntoa(ip_header->ip_dst));
    }

    struct icmphdr *icmp_header = (struct icmphdr*)(p_body + header_len + sizeof(struct ether_header)); //Make ICMP header struct and extract info
    info.icmp_code = icmp_header->code;
    info.icmp_type = icmp_header->type;

    printf("--------------------ICMP--------------------\n");
    return info;
}
//---------------------------------------------------------------------


//Print information from packet_info struct----------------------------
void print_info(char *timestamp, struct packet_info p_info){
    printf("timestamp: %s\n", timestamp);
    print_mac_addr(p_info.src_mac, "src");
    print_mac_addr(p_info.dst_mac, "dst");
    printf("frame lenght: %d bytes\n", p_info.frame_len);

    if(p_info.packet_type != ETHERTYPE_ARP){
        printf("src IP: %s\n", p_info.src_ip);
        printf("dst IP: %s\n", p_info.dst_ip);
    }

    if(p_info.packet_type == ICMP){
        printf("Type: %d\n", p_info.icmp_type);
        printf("Code: %d\n", p_info.icmp_code);
    }

    if(p_info.packet_type == ETHERTYPE_ARP){
        printf("Hardware type: ");
        if(p_info.arp_format_hw == ARPHRD_ETHER) printf("Ethernet (%d)\n", p_info.arp_format_hw);
        else printf("IEEE 802 LAN (%d)\n", p_info.arp_format_hw);
        printf("Protocol type: IPv4 (%d)\n", p_info.arp_format_pro);
        printf("Hardware size: %d\n", p_info.arp_len_hw);
        printf("Protocol size: %d\n", p_info.arp_len_pro);
        printf("Opcode: ");
        if(p_info.arp_opcode == ARPOP_REPLY) printf("reply (%d)\n", p_info.arp_opcode);
        else printf("request (%d)\n", p_info.arp_opcode);

        print_mac_addr(p_info.arphw_sender_mac, "arps");
        printf("Sender IP: %s\n", p_info.arp_sender_ip);
        print_mac_addr(p_info.arphw_target_mac, "arpt");
        printf("Target IP: %s\n", p_info.arp_target_ip);
    }

    if(p_info.packet_type != ICMP && p_info.packet_type != ETHERTYPE_ARP){
        printf("src port: %d\n", p_info.src_port);
        printf("dst port: %d\n\n", p_info.dst_port);
    }
    else printf("\n");
}
//---------------------------------------------------------------------


//Print MAC adress in readable form------------------------------------
void print_mac_addr(u_int8_t *address, char *type){
    if(!strcmp(type, "src")) printf("src MAC: ");
    else if(!strcmp(type, "arps")) printf("Sender HW MAC: ");
    else if(!strcmp(type, "arpt")) printf("Target HW MAC: ");
    else printf("dst MAC: ");

    for(int i = 0; i < 6; i++){
        if(i <= 4) printf("%02x:", address[i]);
        else printf("%02x\n", address[i]);
    }
}
//---------------------------------------------------------------------


//Print Packet data in readable form--------------------------------
void print_data(const struct pcap_pkthdr *p_header, const u_char *p_body){
    printf("DATA:\n");
    unsigned int order_hex = 0;
    int sc = 0;
    int counter = 0;

    for(u_int32_t i = 0; i < p_header->len; i += 16){
        printf("0x%04x: ", order_hex);

        for(u_int32_t j = i; j < i+16; j++){
            if(j >= p_header->len) break;

            if(counter % 8 == 0) printf(" ");
            printf("%02x ", p_body[j]);

            counter++;
        }

        if(counter < 15){
            for(int i = counter; i <= 15; i++){
                if(i % 8 == 0 && i != 0) printf("    ");
                else printf("   ");
            }
        }

        printf(" ");
        for(u_int32_t j = i; j < i+16; j++){
            if(j >= p_header->len) break;

            if(isprint(p_body[j])) printf("%c", p_body[j]);
            else printf(".");

            if(sc == 7) printf(" ");
            if(sc != 0 && sc % 15 == 0) printf("\n");
            sc++;
        }

        sc = 0;
        order_hex += 16;
        counter = 0;
    }
    printf("\n###########################################\n\n");
}
//---------------------------------------------------------------------