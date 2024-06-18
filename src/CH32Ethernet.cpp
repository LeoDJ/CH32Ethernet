#include "CH32Ethernet.h"

int CH32Ethernet::begin(uint32_t timeout, uint32_t responseTimeout) {
    // TODO: DHCP
    return 0;
}

void CH32Ethernet::begin(IPAddress local_ip, IPAddress subnet, IPAddress gateway, IPAddress dns_server) {
    uint32_t ip = local_ip;
    uint32_t net = subnet;
    uint32_t gw = gateway;
    uint32_t dns = dns_server;

    if (subnet == nullptr) {
        net = IPAddress(255,255,255,0);
    }
    if (gateway == nullptr) {
        gateway = IPAddress(local_ip);
        gateway[3] = 1;
        gw = gateway;
    }
    if (dns_server == nullptr) {
        dns_server = IPAddress(gateway);
        dns = dns_server;
    }
    ch32_eth_init(nullptr, (uint8_t*)&ip, (uint8_t*)&gw, (uint8_t*)&dns);

}

void CH32Ethernet::loop() {
    uint32_t delta = millis() - _lastLoop;
    if (delta > 1) {
        _lastLoop = millis();
        ch32_eth_loop(millis());
    }
}

void CH32Ethernet::setLedPins(uint32_t linkPin, uint32_t actPin, bool activeLow) {
    _pinLedLink = linkPin;
    _pinLedAct = actPin;
    _ledsActiveLow = activeLow;

    pinMode(linkPin, OUTPUT);
    digitalWrite(linkPin, activeLow);
    pinMode(actPin, OUTPUT);
    digitalWrite(actPin, activeLow);

    ch32_eth_setLedCallback(ledCallback);
};

void CH32Ethernet::ledCallback(uint8_t ledId, uint8_t state) {
    bool pinState = state ^ getInstance()->_ledsActiveLow;
    switch (ledId) {
        case ETH_LED_LINK:  digitalWrite(getInstance()->_pinLedLink, pinState);    break;
        case ETH_LED_ACT:   digitalWrite(getInstance()->_pinLedAct, pinState);     break;
    }
}


CH32Ethernet Ethernet;