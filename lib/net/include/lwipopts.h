#define NO_SYS 1

#define SYS_LIGHTWEIGHT_PROT 0
#include <stdint.h>
typedef uint32_t sys_prot_t;

#define TCP_LISTEN_BACKLOG 1

#define MEM_ALIGNMENT                   4

#define LWIP_ARP 1
#define LWIP_ETHERNET 1
#define LWIP_DNS 1
#define LWIP_DHCP 1
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0
#define LWIP_STATS 0
#define LWIP_ICMP                1
#define TCP_TTL                 255
#define LWIP_NETIF_HOSTNAME             1
