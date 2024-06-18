#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include "eth.h"

class CH32Ethernet {
  public:
    // Initialise the Ethernet with the internal provided MAC address and gain the rest of the
    // configuration through DHCP.
    // Returns 0 if the DHCP configuration failed, and 1 if it succeeded
    int begin(uint32_t timeout = 60000, uint32_t responseTimeout = 4000);

    void begin(IPAddress local_ip, IPAddress subnet = nullptr, IPAddress gateway = nullptr, IPAddress dns_server = nullptr);

    // Has to be called often in main loop to ensure correct ethernet processing
    void loop();

    static void ledCallback(uint8_t ledId, uint8_t state);

    void setLedPins(uint32_t linkPin, uint32_t actPin, bool activeLow = false);

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
    uint32_t _lastLoop = 0;
    uint32_t _pinLedLink, _pinLedAct;
    bool _ledsActiveLow;
};

extern CH32Ethernet Ethernet;