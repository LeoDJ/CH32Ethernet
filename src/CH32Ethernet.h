#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include "eth.h"
// #include "Dhcp.h"

#include <lwip/ip4_addr.h>
#include <lwip/dns.h>

enum EthernetLinkStatus { 
    Unknown, 
    LinkON,
    LinkOFF 
};

enum EthernetHardwareStatus {
	EthernetNoHardware,
	EthernetW5100,
	EthernetW5200,
	EthernetW5500,
    EthernetBuiltinPHY
};

class CH32Ethernet {
  public:
    // DHCP
    // Returns 0 if the DHCP configuration failed, and 1 if it succeeded, and 2 if running non-blockingly
    // Automatically uses internal
    int begin(uint32_t timeout = 60000, uint32_t responseTimeout = 4000, bool blocking = true);
    int begin(uint8_t *mac_address, unsigned long timeout = 60000, unsigned long responseTimeout = 4000, bool blocking = true);

    // Static IP
    void begin(IPAddress local_ip, IPAddress subnet = nullptr, IPAddress gateway = nullptr, IPAddress dns_server = nullptr);
    void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress subnet = nullptr, IPAddress gateway = nullptr, IPAddress dns_server = nullptr);

    void setLedPins(uint32_t linkPin, uint32_t actPin, bool activeLow = false);

    // Has to be called often in main loop to ensure correct ethernet processing
    int maintain();

    // Getters
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
    IPAddress dnsServerIP() {
        return IPAddress(ip4_addr_get_u32(dns_getserver(0)));
    }
    void MACAddress(uint8_t *mac_address) { 
        memcpy(mac_address, get_netif()->hwaddr, 6); 
    }

    // Setters
	void setDnsServerIP(const IPAddress dns_server);

    // Dummy functions, needed for backwards-compatibility to normal Arduino Ethernet library
    static EthernetHardwareStatus hardwareStatus() {    
        return EthernetBuiltinPHY;  // PHY is always connected
    }   
    static void init(uint8_t sspin) {}    
    void setMACAddress(const uint8_t *mac_address) {}
	void setLocalIP(const IPAddress local_ip) {}
	void setSubnetMask(const IPAddress subnet) {}
	void setGatewayIP(const IPAddress gateway) {}
	void setRetransmissionTimeout(uint16_t milliseconds) {}
	void setRetransmissionCount(uint8_t num) {}

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