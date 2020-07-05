#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/netif.h>
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

SpiHw< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;

void enchw_setup (enchw_device_t *dev) {
    spi.init();
}

void enchw_select (enchw_device_t *dev) {
    spi.enable();
}

void enchw_unselect (enchw_device_t *dev) {
    spi.disable();
}

uint8_t enchw_exchangebyte (enchw_device_t *dev, uint8_t byte) {
    return spi.transfer(byte);
}

ip4_addr myip_addr = {0x02BCA8C0UL}; /* 192.168.188.2 */
ip4_addr gw_addr = {0x01BCA8C0UL}; /* 192.168.188.1 */
ip4_addr netmask = {0x000000FFUL}; /* 0.0.0.255 */

static netif mchdrv_netif;

static enc_device_t mchdrv_hw;

void mchdrv_poll (netif *netif) {
    auto encdevice = (enc_device_t*)netif->state;

    if (enc_MII_read(encdevice, (enc_register_t) ENC_PHSTAT1) & (1 << 2))
        netif_set_link_up(netif);
    else
        netif_set_link_down(netif);

    if (enc_RCR(encdevice, ENC_EPKTCNT)) {
        pbuf *buf = NULL;
        if (enc_read_received_pbuf(encdevice, &buf) == 0)
            netif->input(buf, netif);
    }
}

static err_t mchdrv_linkoutput (netif *netif, pbuf *p) {
    enc_device_t *encdevice = (enc_device_t*)netif->state;

    enc_transmit_pbuf(encdevice, p);
    return ERR_OK;
}

err_t mchdrv_init (netif *netif) {
    auto encdevice = (enc_device_t*)netif->state;

    if (enc_setup_basic(encdevice) != 0 || enc_bist_manual(encdevice) != 0)
        return ERR_IF;

    enc_ethernet_setup(encdevice, 4*1024, netif->hwaddr);
    enc_set_multicast_reception(encdevice, 1); // always enable?

    netif->output = etharp_output;
    netif->linkoutput = mchdrv_linkoutput;
    netif->mtu = 1500;
    netif->flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;

    return ERR_OK;
}

void mch_net_init () {
    lwip_init();

    mchdrv_netif.hwaddr_len = 6; /* demo mac address */
    mchdrv_netif.hwaddr[0] = 0;
    mchdrv_netif.hwaddr[1] = 1;
    mchdrv_netif.hwaddr[2] = 2;
    mchdrv_netif.hwaddr[3] = 3;
    mchdrv_netif.hwaddr[4] = 4;
    mchdrv_netif.hwaddr[5] = 5;

    if (netif_add(&mchdrv_netif, &myip_addr, &netmask, &gw_addr, &mchdrv_hw,
                mchdrv_init, ethernet_input) == NULL) {
        //LWIP_ASSERT("mch_net_init: netif_add (mchdrv_init) failed\n", 0);
        assert(false);
    }

    netif_set_default(&mchdrv_netif);
    netif_set_up(&mchdrv_netif);
}

void mch_net_poll () {
    mchdrv_poll(&mchdrv_netif);
}

extern "C" uint32_t sys_now () {
    return ticks;
}
