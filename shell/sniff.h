#ifndef SNIFF_H
#define SNIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include "twin.h"

//When interface is not connected
// Return 0 = OK, 1 = not connected/down, -1 = unknown (no sysfs)
// If not OK, writes a human message into `why`.
static int iface_not_ready(const char *iface, char *why, size_t why_sz)
{
    char path[256], buf[32];    //256 is large enough to store a system path
                                //32 is large enough for tiny file names 

    // Loopback is always OK for sniffing.
    if (strcmp(iface, "lo") == 0) return 0;

    // Prefer carrier: "1"=link, "0"=no link
    //snprintf for a formatted character array as output.
    //its purpose is to safeguard against buffer overflow & writing beyond
    //the allocated memory
    snprintf(path, sizeof(path), "/sys/class/net/%s/carrier", iface);
    FILE *f = fopen(path, "r"); //open a file and create a stream to it
    if (f) {                    //if file is opened succesfully, continue in this block
        if (fgets(buf, sizeof(buf), f)) {   //reads up to the sizeof(buf)-1 from f into buf
            fclose(f);                      //close the stream
            if (buf[0] == '0') {            //if buff has no input,
                snprintf(why, why_sz,
                         "%s: no Ethernet link detected (no carrier). Plug in a cable or choose another interface.",
                         iface);            //print there is not active carrier connection
                return 1;
            }
        } else {
            fclose(f);  //close stream without error message
        }
    }

    // Fallback: operstate 
    snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", iface); //build the system file path that checks the interface's operational state
    f = fopen(path, "r");
    if (f) {
        if (fgets(buf, sizeof(buf), f)) {
            fclose(f);
            buf[strcspn(buf, "\r\n")] = 0;

            // Treat UP as OK; treat DOWN/DORMANT/LOWERLAYERDOWN as not ready.
            // Treat UNKNOWN as OK for non-loopback too (some drivers report this).
            if (strcmp(buf, "up") == 0) return 0;       //up = interface is ready
            if (strcmp(buf, "down") == 0 ||             //down = interface is down
                strcmp(buf, "dormant") == 0 ||          //dormant = L1 is up but not passing traffic
                strcmp(buf, "lowerlayerdown") == 0) {   //lower level link isn't up
                snprintf(why, why_sz, "%s: interface is %s (no active connection).", iface, buf);
                return 1;
            }
            return 0;   //allow sniffing
        }
        fclose(f);      //if fgets fails, close the file
    }
    // If sysfs isn't present, we can't tell—let sniff proceed.
    snprintf(why, why_sz, "%s: cannot determine link state (proceeding anyway).", iface);
    return -1;
}


//Network packet callback
static void pkt_handler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    (void)user; //silence unused parameter warning. compile with -Wall
    if (h->caplen < sizeof(struct ether_header)) return;    //ensure a full ethernet header is captured

    const struct ether_header *eth = (const struct ether_header *)bytes;    //eth[0] is the first bytes of the captured frame 
    uint16_t et = ntohs(eth->ether_type);   //convert to host order

    if (et == ETHERTYPE_IP) {   //if the IP framed carries IPv4, parse more. otherwise print the non-IP traffic EtherType
        const u_char *p = bytes + sizeof(struct ether_header);  //points to the start of the IPv4 header
        if (h->caplen < sizeof(struct ether_header) + sizeof(struct ip)) return;    //check if the IPv4 header is the right size

        const struct ip *ip = (const struct ip *)p; //using <netinet/ip.h> to view the bytes as an IPv4 header
        int iphl = ip->ip_hl * 4;   //32 bit word * 4  to get bytes
        if (h->caplen < sizeof(struct ether_header) + (unsigned)iphl) return;   //make sure that full IPv4 header is captured

        char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip->ip_src, src, sizeof(src));
        inet_ntop(AF_INET, &ip->ip_dst, dst, sizeof(dst));

        const u_char *l4 = p + iphl;
        //check tcp (transmission control protocol)
        if (ip->ip_p == IPPROTO_TCP && h->caplen >= (size_t)(l4 - bytes + sizeof(struct tcphdr))) {
            const struct tcphdr *th = (const struct tcphdr *)l4;
            printf(G "TCP " RST "%s:%u -> %s:%u len=%u\n",
                   src, ntohs(th->th_sport), dst, ntohs(th->th_dport), h->len);
        } 
        //check udp (user datagram protocol)
        else if (ip->ip_p == IPPROTO_UDP && h->caplen >= (size_t)(l4 - bytes + sizeof(struct udphdr))) {
            const struct udphdr *uh = (const struct udphdr *)l4;
            printf(C "UDP " RST "%s:%u -> %s:%u len=%u\n",
                   src, ntohs(uh->uh_sport), dst, ntohs(uh->uh_dport), h->len);
        } else {
            printf("IP proto=%u %s -> %s len=%u\n", ip->ip_p, src, dst, h->len);
        }
    } else {
        printf("EthType=0x%04x len=%u\n", et, h->len);
    }
}

// Join argv[i..argc-1] with spaces into buf (NUL-terminated)
static void join_args(char *out, size_t outsz, char **argv, int i, int argc)
{
    out[0] = '\0';
    for (int k = i; k < argc; ++k) {
        size_t have = strlen(out);
        if (have + strlen(argv[k]) + 2 >= outsz) break;
        if (have) strncat(out, " ", outsz - have - 1);
        strncat(out, argv[k], outsz - strlen(out) - 1);
    }
}

// sniff <iface> [count] [bpf...]
static int cmd_sniff(int argc, char **argv)
{
    if (argc < 2) {
        printf(RED "usage: sniff <iface> [count] [bpf filter...]\n" RST);
        return 1;
    }
    const char *iface = argv[1];
    int count = -1;
    int idx = 2;

    // check link status and fail fast if no Ethernet connection
    {
        char why[160];
        int st = iface_not_ready(iface, why, sizeof(why));
        if (st == 1) {                    // definitely not ready
            fprintf(stderr, RED "%s\n" RST, why);
            return 1;
        } 
    }
    
    if (argc < 2) {
        printf(RED "usage: sniff <iface> [count] [bpf filter...]\n" RST);
        return 1;
    }

    if (idx < argc) {
        int isnum = 1;
        for (const char *p = argv[idx]; *p; ++p) if (*p < '0' || *p > '9') { isnum = 0; break; }
        if (isnum) { count = atoi(argv[idx]); if (count <= 0) count = -1; idx++; }
    }

    char filter[512];
    join_args(filter, sizeof(filter), argv, idx, argc);

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *pc = pcap_open_live(iface, 65535, 1, 1000, errbuf);
    if (!pc) {
        fprintf(stderr, RED "pcap_open_live(%s): %s\n" RST, iface, errbuf);
        return 2;
    }

    if (filter[0]) {
        struct bpf_program prog;
        if (pcap_compile(pc, &prog, filter, 1, PCAP_NETMASK_UNKNOWN) == -1) {
            fprintf(stderr, RED "pcap_compile: %s\n" RST, pcap_geterr(pc));
            pcap_close(pc);
            return 3;
        }
        if (pcap_setfilter(pc, &prog) == -1) {
            fprintf(stderr, RED "pcap_setfilter: %s\n" RST, pcap_geterr(pc));
            pcap_freecode(&prog);
            pcap_close(pc);
            return 4;
        }
        pcap_freecode(&prog);
    }

    printf(G "[sniffing on %s]%s%s\n" RST, iface, filter[0] ? " filter: " : "", filter);
    int r = pcap_loop(pc, (count < 0 ? -1 : count), pkt_handler, NULL);
    if (r == -1) fprintf(stderr, RED "pcap_loop: %s\n" RST, pcap_geterr(pc));
    pcap_close(pc);
    return (r == -1);
}
#endif