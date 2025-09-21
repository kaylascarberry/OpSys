#ifndef SNIFF_H
#define SNIFF_H

#include <sys/types.h>   // brings in u_char/u_short/u_int on glibc
#include <pcap.h>        // ok on Ubuntu after installing libpcap0.8-dev
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

// (your pkt_handler / cmd_sniff go here)

#endif /* SNIFF_H */

// --- Packet printer callback ---
static void pkt_handler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    (void)user;
    if (h->caplen < sizeof(struct ether_header)) return;

    const struct ether_header *eth = (const struct ether_header *)bytes;
    uint16_t et = ntohs(eth->ether_type);

    if (et == ETHERTYPE_IP) {
        const u_char *p = bytes + sizeof(struct ether_header);
        if (h->caplen < sizeof(struct ether_header) + sizeof(struct ip)) return;

        const struct ip *ip = (const struct ip *)p;
        int iphl = ip->ip_hl * 4;
        if (h->caplen < sizeof(struct ether_header) + (unsigned)iphl) return;

        char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip->ip_src, src, sizeof(src));
        inet_ntop(AF_INET, &ip->ip_dst, dst, sizeof(dst));

        const u_char *l4 = p + iphl;
        if (ip->ip_p == IPPROTO_TCP && h->caplen >= (size_t)(l4 - bytes + sizeof(struct tcphdr))) {
            const struct tcphdr *th = (const struct tcphdr *)l4;
            printf(G "TCP " RST "%s:%u -> %s:%u len=%u\n",
                   src, ntohs(th->th_sport), dst, ntohs(th->th_dport), h->len);
        } else if (ip->ip_p == IPPROTO_UDP && h->caplen >= (size_t)(l4 - bytes + sizeof(struct udphdr))) {
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
    int count = -1; // infinite
    int idx = 2;

    // optional numeric count
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

