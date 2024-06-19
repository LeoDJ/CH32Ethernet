#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <lwip/ip4_addr.h>
#include <lwip/dns.h>
#include <lwip/prot/dhcp.h>
#include "CH32Ethernet.h"

#ifndef TIMEOUT_DNS_REQUEST
#define TIMEOUT_DNS_REQUEST 10000U
#endif

class DNSClient {
  public:
    // Possible return codes from ProcessResponse
    #define SUCCESS          1
    #define TIMED_OUT        -1
    #define INVALID_SERVER   -2
    #define TRUNCATED        -3
    #define INVALID_RESPONSE -4
    
    // ctor
    void begin(const IPAddress &aDNSServer) {
        iDNSServer = aDNSServer;
        if (dhcp_get_state() == DHCP_STATE_OFF && (dns_getserver(0)->addr == ip_addr_any.addr)) {
            dns_init();
            ip_addr_t ip;
            ip4_addr_set_u32(&ip, aDNSServer);
            dns_setserver(0, &ip);
        }
    }

    static void dns_callback(const char* name, const ip_addr_t* ipaddr, void* callback_arg) {
        if (ipaddr != NULL) {
            *((uint32_t*)callback_arg) = ip4_addr_get_u32(ipaddr);
        }
        else {
            *((uint32_t*)callback_arg) = 0;
        }
    }

    /** Resolve the given hostname to an IP address.
        @param aHostname Name to be resolved
        @param aResult IPAddress structure to store the returned IP address
        @result 1 if aIPAddrString was successfully converted to an IP address,
                else error code
    */
    int getHostByName(const char *aHostname, IPAddress &aResult) {
        ip4_addr_t ipResult;
        // check and return parsed IPv4 from string
        if (ip4addr_aton(aHostname, &ipResult)) {
            aResult = ipResult.addr;
            return SUCCESS;
        }
        // Check we've got a valid DNS server to use
        if (iDNSServer == INADDR_NONE) {
            return INVALID_SERVER;
        }

        err_t err;
        ip_addr_t iphost;
        uint32_t result;
        err = dns_gethostbyname(aHostname, &iphost, &dns_callback, &result);
        switch (err) {
            case ERR_OK:
                aResult = IPAddress(ip4_addr_get_u32(&iphost));
                return 1;
            case ERR_INPROGRESS: {
                uint32_t start = millis();
                while (result == 0) {
                    Ethernet.maintain();
                    if (millis() - start >= TIMEOUT_DNS_REQUEST) {
                        return TIMED_OUT;
                    }
                }
                if (result == 0) {
                    return INVALID_SERVER;
                }
                else {
                    aResult = IPAddress(result);
                    return SUCCESS;
                }
            }
            case ERR_ARG:
                return INVALID_RESPONSE;
            default:
                return INVALID_RESPONSE;

        }

    }

  protected:
    uint16_t BuildRequest(const char *aName);
    uint16_t ProcessResponse(uint16_t aTimeout, IPAddress &aAddress);


    IPAddress iDNSServer;
};