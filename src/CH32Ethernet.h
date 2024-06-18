#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include "eth.h"
// #include "Dhcp.h"

#include <lwip/ip4_addr.h>

enum EthernetLinkStatus { 
    Unknown, 
    LinkON,
    LinkOFF 
};

class CH32Ethernet {
  public:
    // DHCP
    // Returns 0 if the DHCP configuration failed, and 1 if it succeeded, and 2 if running non-blockingly
    int begin(uint32_t timeout = 60000, uint32_t responseTimeout = 4000, bool blocking = true);

    // Static IP
    void begin(IPAddress local_ip, IPAddress subnet = nullptr, IPAddress gateway = nullptr, IPAddress dns_server = nullptr);

    void setLedPins(uint32_t linkPin, uint32_t actPin, bool activeLow = false);

    // Has to be called often in main loop to ensure correct ethernet processing
    int maintain();

    EthernetLinkStatus linkStatus() {
        return netif_is_link_up(get_netif()) ? LinkON : LinkOFF;
    }
    IPAddress localIP() {
        return IPAddress(ip4_addr_get_u32(&(get_netif()->ip_addr)));
    }
    IPAddress subnetMask() {
        return IPAddress(ip4_addr_get_u32(&(get_netif()->netmask)));
    }
    IPAddress gatewayIP() {
        return IPAddress(ip4_addr_get_u32(&(get_netif()->gw)));
    }
    IPAddress dnsServerIP(); // TODO


    static void ledCallback(uint8_t ledId, uint8_t state);

    // Singleton pattern, for passing function pointers to C callbacks
    CH32Ethernet() {};
    CH32Ethernet(CH32Ethernet const&) = delete;
    void operator=(CH32Ethernet const&) = delete;
    static CH32Ethernet *getInstance() {
        // static CH32Ethernet instance;   // Guaranteed to be destroyed. Instantiated on first use.
        if (instance == nullptr) {
            extern CH32Ethernet Ethernet;
            instance = &Ethernet;
        }
        return instance;
    }

  protected:
    static inline CH32Ethernet *instance = nullptr;
    // DhcpClass *_dhcp;
    uint32_t _lastLoop = 0;
    uint32_t _pinLedLink, _pinLedAct;
    bool _ledsActiveLow;
};

extern CH32Ethernet Ethernet;