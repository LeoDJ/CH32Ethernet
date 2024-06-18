#include "eth.h"
#include <string.h>
#include <lwip/etharp.h>
#include <lwip/init.h>
#include <netif/ethernet.h>

extern ETH_DMADESCTypeDef* pDMARxSet;
extern ETH_DMADESCTypeDef* pDMATxSet;
extern void WCHNET_HandlePhyNegotiation(void);

struct netif gnetif;

volatile bool _hasFrame = 0;
struct pbuf* _pbuf = NULL;

// copied from eth_driver.c/RecDataPolling
uint32_t eth_get_packet(uint8_t** buffer, uint16_t* len) {
    if (pDMARxSet->Status & ETH_DMARxDesc_OWN) {
        return ETH_ERROR;
    }

    // while (!(pDMARxSet->Status & ETH_DMARxDesc_OWN)) {
    if (!(pDMARxSet->Status & ETH_DMARxDesc_ES) && (pDMARxSet->Status & ETH_DMARxDesc_LS) &&
        (pDMARxSet->Status & ETH_DMARxDesc_FS)) {
        /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
        *len = ((pDMARxSet->Status & ETH_DMARxDesc_FL) >> 16) - 4;
        /* Get the addrees of the actual buffer */
        *buffer = (uint8_t*)pDMARxSet->Buffer1Addr;
    }
    pDMARxSet->Status = ETH_DMARxDesc_OWN;
    pDMARxSet = (ETH_DMADESCTypeDef*)pDMARxSet->Buffer2NextDescAddr;

    return ETH_SUCCESS;
    // }
}

// copied from eth_driver.c/MACRAW_Tx, but actually using the TX buffer, because of 4 byte alignment
// (number of DMATxDesc's are hardcoded to 1, so no actual buffering going on.)
uint32_t eth_send_packet(const uint8_t* buffer, uint16_t len) {
    /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
    if (DMATxDescToSet->Status & ETH_DMATxDesc_OWN) {
        /* Return ERROR: OWN bit set */
        return ETH_ERROR;
    }
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;
    memcpy((void*)DMATxDescToSet->Buffer1Addr, buffer, len);
    R16_ETH_ETXLN = len;
    R16_ETH_ETXST = DMATxDescToSet->Buffer1Addr;
    R8_ETH_ECON1 |= RB_ETH_ECON1_TXRTS; // start sending
    /* Update the ETHERNET DMA global Tx descriptor with next Tx descriptor */
    /* Chained Mode */
    /* Selects the next DMA Tx descriptor list for next buffer to send */
    DMATxDescToSet = (ETH_DMADESCTypeDef*)(DMATxDescToSet->Buffer2NextDescAddr);
    /* Return SUCCESS */
    return ETH_SUCCESS;
}

extern "C" {
INTERRUPT(ETH_IRQHandler) {
    WCHNET_ETHIsr();

    if (!_hasFrame) {
        uint8_t* buf;
        uint16_t len;
        if (eth_get_packet(&buf, &len) == ETH_ERROR) {
            return;
        }

        _pbuf = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        pbuf_take(_pbuf, buf, len);
        _hasFrame = 1;
    }
}
}

static err_t ch32_netif_output(struct netif* netif, struct pbuf* p) {
    (void)netif;
    LINK_STATS_INC(link.xmit);

    if (eth_send_packet((const uint8_t*)p->payload, p->tot_len) == ETH_ERROR) {
        return ERR_IF;
    }
    return ERR_OK;
}

err_t ch32_netif_init(struct netif* netif) {
    netif->linkoutput = ch32_netif_output;
    netif->output = etharp_output;
    netif->mtu = MAX_ETH_PAYLOAD;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    netif->hostname = "lwip";
    netif->name[0] = 'c';
    netif->name[1] = 'h';

    WCHNET_GetMacAddr(netif->hwaddr);
    netif->hwaddr_len = 6;

    return ERR_OK;
}

void netifCfg(const ip4_addr_t* ipaddr, const ip4_addr_t* netmask, const ip4_addr_t* gw) {
    netif_remove(&gnetif);

    netif_add(&gnetif, ipaddr, netmask, gw, NULL, &ch32_netif_init, &ethernet_input);

    netif_set_default(&gnetif);

    if (netif_is_link_up(&gnetif)) {
        netif_set_up(&gnetif); // When the netif is fully configured this function must be called
    }
    else {
        netif_set_down(&gnetif); // When the netif link is down this function must be called
    }

#if LWIP_NETIF_LINK_CALLBACK
    netif_set_link_callback(&gnetif, ethernetif_update_config); // TODO
#endif
}

#define IP_ADDR4_U8ARR(ipaddr, u8ptr) IP_ADDR4((ipaddr), (u8ptr)[0], (u8ptr)[1], (u8ptr)[2], (u8ptr)[3])

uint32_t ch32_eth_init(uint8_t* mac, const uint8_t* ip, const uint8_t* gw, const uint8_t* netmask) {
    if ((SystemCoreClock != 60000000) && (SystemCoreClock != 120000000)) {
        printf("[ETH] ERROR! SysClock has to be 60MHz or 120MHz when using Ethernet! \nCurrent SysClock: %d\n",
               SystemCoreClock);
        return ETH_ERROR;
    }
    if (mac == nullptr) {
        uint8_t wchMac[6];
        WCHNET_GetMacAddr(wchMac);
        mac = wchMac;
    }

    ip_addr_t ipAddr, gwAddr, netmaskAddr;
    ip_addr_set_zero_ip4(&ipAddr);
    ip_addr_set_zero_ip4(&gwAddr);
    ip_addr_set_zero_ip4(&netmaskAddr);

    if (ip != nullptr) {
        IP_ADDR4_U8ARR(&ipAddr, ip);
    }

    if (gw != nullptr) {
        IP_ADDR4_U8ARR(&gwAddr, gw);
    }

    if (netmask != nullptr) {
        IP_ADDR4_U8ARR(&netmaskAddr, netmask);
    }

    ETH_Init(mac); // TODO: check if built-in delay causes problems

    lwip_init();

    netifCfg(&ipAddr, &gwAddr, &netmaskAddr);

    return ETH_SUCCESS;
}

void ch32_eth_loop() {
    // TODO: link state foo

    if (_hasFrame) {
        LINK_STATS_INC(link.recv);
        if (gnetif.input(_pbuf, &gnetif) != ERR_OK) {
            pbuf_free(_pbuf);
        }
        _hasFrame = 0;
    }

    // TODO: WCHNET_TimeIsr()?
    // WCHNET_HandlePhyNegotiation();
}