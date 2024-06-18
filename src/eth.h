#pragma once

#include <lwip/netif.h>
#include "ETH_Driver/eth_driver.h"

#define INTERRUPT(name) __attribute__((interrupt("WCH-Interrupt-fast"))) void name(void)

uint32_t ch32_eth_init(uint8_t *mac = nullptr, const uint8_t *ip = nullptr, const uint8_t *gw = nullptr, const uint8_t *netmask = nullptr);

void ch32_eth_loop(uint32_t deltaMs = 0);

// LwIP driver
err_t ch32_netif_init(struct netif *netif);