#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/netif.h>
#include <lwip/init.h>
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

struct ip4_addr mch_myip_addr = {0x02BCA8C0UL}; /* 192.168.188.2 */
struct ip4_addr gw_addr = {0x01BCA8C0UL}; /* 192.168.188.1 */
struct ip4_addr netmask = {0x000000FFUL}; /* 0.0.0.255 */

static struct netif mchdrv_netif;

static enc_device_t mchdrv_hw;

void mchdrv_poll (struct netif *netif) {
    err_t result;
    struct pbuf *buf = NULL;

    uint8_t epktcnt;
    bool linkstate;
    enc_device_t *encdevice = (enc_device_t*)netif->state;

    linkstate = enc_MII_read(encdevice, (enc_register_t) ENC_PHSTAT1) & (1 << 2);

    if (linkstate) netif_set_link_up(netif);
    else netif_set_link_down(netif);

    epktcnt = enc_RCR(encdevice, ENC_EPKTCNT);

    if (epktcnt) {
        if (enc_read_received_pbuf(encdevice, &buf) == 0)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("incoming: %d packages, first read into %x\n", epktcnt, (unsigned int)(buf)));
            result = netif->input(buf, netif);
            LWIP_DEBUGF(NETIF_DEBUG, ("received with result %d\n", result));
        } else {
            /* FIXME: error reporting */
            LWIP_DEBUGF(NETIF_DEBUG, ("didn't receive.\n"));
        }
    }
}

static err_t mchdrv_linkoutput (struct netif *netif, struct pbuf *p) {
    enc_device_t *encdevice = (enc_device_t*)netif->state;
    enc_transmit_pbuf(encdevice, p);
    LWIP_DEBUGF(NETIF_DEBUG, ("sent %d bytes.\n", p->tot_len));
    /* FIXME: evaluate result state */
    return ERR_OK;
}

err_t mchdrv_init (struct netif *netif) {
    int result;
    enc_device_t *encdevice = (enc_device_t*)netif->state;

    LWIP_DEBUGF(NETIF_DEBUG, ("Starting mchdrv_init.\n"));

    result = enc_setup_basic(encdevice);
    if (result != 0)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("Error %d in enc_setup, interface setup aborted.\n", result));
        return ERR_IF;
    }
    result = enc_bist_manual(encdevice);
    if (result != 0)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("Error %d in enc_bist_manual, interface setup aborted.\n", result));
        return ERR_IF;
    }
    enc_ethernet_setup(encdevice, 4*1024, netif->hwaddr);
    /* enabling this unconditonally: there seems not to be a generic way by
     * which protocols indicate their multicast requirements to the netif,
     * going for "always on" for now */
    enc_set_multicast_reception(encdevice, 1);

    netif->output = etharp_output;
    netif->linkoutput = mchdrv_linkoutput;

    netif->mtu = 1500; /** FIXME check with documentation when jumboframes can be ok */

    netif->flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;

    LWIP_DEBUGF(NETIF_DEBUG, ("Driver initialized.\n"));

    return ERR_OK;
}

void mch_net_init () {
    // Initialize LWIP
    lwip_init();

    mchdrv_netif.hwaddr_len = 6;
    /* demo mac address */
    mchdrv_netif.hwaddr[0] = 0;
    mchdrv_netif.hwaddr[1] = 1;
    mchdrv_netif.hwaddr[2] = 2;
    mchdrv_netif.hwaddr[3] = 3;
    mchdrv_netif.hwaddr[4] = 4;
    mchdrv_netif.hwaddr[5] = 5;

    // Add our netif to LWIP (netif_add calls our driver initialization function)
    if (netif_add(&mchdrv_netif, &mch_myip_addr, &netmask, &gw_addr, &mchdrv_hw,
                mchdrv_init, ethernet_input) == NULL) {
        LWIP_ASSERT("mch_net_init: netif_add (mchdrv_init) failed\n", 0);
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
