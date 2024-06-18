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

  protected:
    uint32_t _lastLoop = 0;
};

extern CH32Ethernet Ethernet;