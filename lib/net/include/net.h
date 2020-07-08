#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/udp.h>
#include <lwip/init.h>
#include <lwip/pbuf.h>
#include <lwip/stats.h>
#include <lwip/timeouts.h>
#include <lwip/err.h>
#include <netif/etharp.h>

extern "C" {
#include "enchw.h"
#include "enc28j60.h"
}

ip4_addr myip_addr = {0x02BCA8C0UL}; /* 192.168.188.2 */
ip4_addr gw_addr = {0x01BCA8C0UL}; /* 192.168.188.1 */
ip4_addr netmask = {0x00FFFFFFUL}; /* 255.255.255.0 */
ip4_addr dns = {0x08080808UL}; /* 8.8.8.8 */

static netif enc_if;
static enc_device_t enc_hw;

static void mn_poll (netif* netif) {
    auto dev = (enc_device_t*)netif->state;

    if (enc_MII_read(dev, (enc_register_t) ENC_PHSTAT1) & (1 << 2))
        netif_set_link_up(netif);
    else
        netif_set_link_down(netif);

    if (enc_RCR(dev, ENC_EPKTCNT)) {
        pbuf* buf = NULL;
        if (enc_read_received_pbuf(dev, &buf) == 0)
            netif->input(buf, netif);
    }
}

static err_t mn_linkoutput (netif* netif, pbuf* p) {
    enc_device_t* dev = (enc_device_t*)netif->state;
    enc_transmit_pbuf(dev, p);
    return ERR_OK;
}

err_t mn_init (netif* netif) {
    auto dev = (enc_device_t*)netif->state;

    if (enc_setup_basic(dev) != 0 || enc_bist_manual(dev) != 0)
        return ERR_IF;

    enc_ethernet_setup(dev, 4*1024, netif->hwaddr);
    //enc_set_multicast_reception(dev, 1); // always enable?

    netif->output = etharp_output;
    netif->linkoutput = mn_linkoutput;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    return ERR_OK;
}

void mch_net_init () {
    lwip_init();

    enc_if.hwaddr_len = 6; /* demo mac address */
    memcpy(enc_if.hwaddr, "\x00\x01\x02\x03\x04\x05", 6);

    auto p = netif_add(&enc_if, &myip_addr, &netmask, &gw_addr, &enc_hw,
                        mn_init, ethernet_input);
    (void) p;
    assert(p != 0);

    netif_set_default(&enc_if);
    netif_set_up(&enc_if);
}

void mch_net_poll () {
    mn_poll(&enc_if);
}
