#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/init.h>
#include <lwip/pbuf.h>
#include <lwip/stats.h>
#include <lwip/timeouts.h>
#include <lwip/sys.h>
#include <lwip/err.h>
#include <netif/etharp.h>

extern "C" {
#include "enchw.h"
#include "enc28j60.h"
}

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;

void enchw_setup (enchw_device_t*) {
    spi.init();
}

void enchw_select (enchw_device_t*) {
    spi.enable();
}

void enchw_unselect (enchw_device_t*) {
    spi.disable();
}

uint8_t enchw_exchangebyte (enchw_device_t*, uint8_t b) {
    return spi.transfer(b);
}

ip4_addr myip_addr = {0x02BCA8C0UL}; /* 192.168.188.2 */
ip4_addr gw_addr = {0x01BCA8C0UL}; /* 192.168.188.1 */
ip4_addr netmask = {0x00FFFFFFUL}; /* 255.255.255.0 */
ip4_addr dns = {0x08080808UL}; /* 8.8.8.8 */

static netif enc_if;
static enc_device_t enc_hw;

void mchdrv_poll (netif* netif) {
    auto dev = (enc_device_t*)netif->state;

    if (enc_MII_read(dev, (enc_register_t) ENC_PHSTAT1) & (1 << 2))
        netif_set_link_up(netif);
    else
        netif_set_link_down(netif);

    if (enc_RCR(dev, ENC_EPKTCNT)) {
        //printf("\t\t receive!\n");
        pbuf* buf = NULL;
        if (enc_read_received_pbuf(dev, &buf) == 0)
            netif->input(buf, netif);
    }
}

static err_t mchdrv_linkoutput (netif* netif, pbuf* p) {
    enc_device_t* dev = (enc_device_t*)netif->state;

    printf("\t\t send!\n");
    enc_transmit_pbuf(dev, p);
    return ERR_OK;
}

err_t mchdrv_init (netif* netif) {
    auto dev = (enc_device_t*)netif->state;

    if (enc_setup_basic(dev) != 0 || enc_bist_manual(dev) != 0)
        return ERR_IF;

    enc_ethernet_setup(dev, 4*1024, netif->hwaddr);
    //enc_set_multicast_reception(dev, 1); // always enable?

    netif->output = etharp_output;
    netif->linkoutput = mchdrv_linkoutput;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP;

    NETIF_SET_CHECKSUM_CTRL(netif,
        NETIF_CHECKSUM_CHECK_IP
        | NETIF_CHECKSUM_CHECK_UDP
        | NETIF_CHECKSUM_CHECK_TCP
        | NETIF_CHECKSUM_CHECK_ICMP
        | NETIF_CHECKSUM_CHECK_ICMP6);

    return ERR_OK;
}

void mch_net_init () {
    lwip_init();

    enc_if.hwaddr_len = 6; /* demo mac address */
    enc_if.hwaddr[0] = 0;
    enc_if.hwaddr[1] = 1;
    enc_if.hwaddr[2] = 2;
    enc_if.hwaddr[3] = 3;
    enc_if.hwaddr[4] = 4;
    enc_if.hwaddr[5] = 5;

    enc_if.name[0] = 'e';
    enc_if.name[1] = '0';
    if (netif_add(&enc_if, &myip_addr, &netmask, &gw_addr, &enc_hw,
                mchdrv_init, ethernet_input) == NULL) {
        //LWIP_ASSERT("mch_net_init: netif_add (mchdrv_init) failed\n", 0);
        assert(false);
    }

    netif_set_hostname(&enc_if, "f103");
    netif_set_default(&enc_if);
    netif_set_up(&enc_if);

    //dhcp_start(&mchdrv_netif);
    dns_setserver(0, &dns);
    
    netif_set_link_up(&enc_if);
}

void mch_net_poll () {
    mchdrv_poll(&enc_if);
}

uint32_t sys_now () {
    return ticks;
}
