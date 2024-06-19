#include "CH32EthernetClient.h"
#include "CH32Ethernet.h"
#include "Dns.h"
#include <lwip/tcpip.h>

const uint16_t FREE_BUFFER_THRESHHOLD = (PBUF_POOL_SIZE / 2) * PBUF_POOL_BUFSIZE;

err_t EthernetClient::tcpReceive(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
    EthernetClient* client = reinterpret_cast<EthernetClient*>(arg);
    if (err == ERR_OK && p != nullptr) {
        if (client->_rxBuffer == nullptr) {
            client->_rxBuffer = p;
        }
        else {
            pbuf_cat(client->_rxBuffer, p);
        }
        
        if (client->_rxBuffer->tot_len < FREE_BUFFER_THRESHHOLD) {
            tcp_recved(tpcb, p->tot_len);   // acknowledge data reception, only when enough buffer available
        }
        else {
            client->_remainingTcpDataLen = p->tot_len;  // handle externally, otherwise buffer overflows
        }

        
    }
    else if (err == ERR_OK && p == nullptr) {
        client->_state = 0;
        tcp_close(tpcb);
        client->_pcb = nullptr;
    }
    return ERR_OK;
}

void EthernetClient::tcpError(void* arg, err_t err) {
    EthernetClient* client = reinterpret_cast<EthernetClient*>(arg);
    client->_state = TCP_CLOSING;
    client->_pcb = nullptr;
    if (client->_rxBuffer) {
        pbuf_free(client->_rxBuffer);
        client->_rxBuffer = nullptr;
    }
}

int EthernetClient::connect(const char* host, uint16_t port) {
    // Look up the host first
    int ret = 0;
    DNSClient dns;
    IPAddress remote_addr;

    dns.begin(Ethernet.dnsServerIP());
    ret = dns.getHostByName(host, remote_addr);
    if (ret == 1) {
        return connect(remote_addr, port);
    }
    else {
        return 0;
    }
}

int EthernetClient::connect(IPAddress ip, uint16_t port) {
    if (_pcb) {
        stop();
    }

    _pcb = tcp_new();
    if (!_pcb)
        return 0;

    ip_addr_t ipaddr;
    ip4_addr_set_u32(&ipaddr, ip);
    _state = TCP_NONE;

    tcp_arg(_pcb, this);
    err_t err = tcp_connect(_pcb, &ipaddr, port, [](void* arg, struct tcp_pcb* tpcb, err_t err) {
        EthernetClient* client = reinterpret_cast<EthernetClient*>(arg);
        if (err == ERR_OK && arg != NULL) {
            tcp_recv(tpcb, tcpReceive);
            tcp_err(tpcb, tcpError);
            client->_state = TCP_CONNECTED;
        }
        else {
            client->_state = TCP_NONE;
        }
        return err;
    });

    if (err != ERR_OK) {
        tcp_close(_pcb);
        _pcb = nullptr;
        return 0;
    }

    uint32_t startTime = millis();
    while (_state == TCP_NONE) {
        Ethernet.maintain();
        if ((_state == TCP_CLOSING) || ((millis() - startTime) >= _connectionTimeout)) {
            stop();
            return 0;
        }
    }

    _state = TCP_CONNECTED;
    return 1;
}

size_t EthernetClient::write(uint8_t b) {
    return write(&b, 1);
}

size_t EthernetClient::write(const uint8_t* buf, size_t size) {
    if (!_pcb) {
        return 0;
    }
    err_t err = tcp_write(_pcb, buf, size, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
        tcp_output(_pcb);
        return size;
    }
    return 0;
}

int EthernetClient::available() {
    if (!_pcb || !_rxBuffer) {
        return 0;
    }
    return _rxBuffer->tot_len;
}

int EthernetClient::read() {
    uint8_t c;
    return read(&c, 1) == 1 ? c : -1;
}

int EthernetClient::read(uint8_t* buf, size_t size) {
    if (!_pcb || !_rxBuffer) {
        return 0;
    }

    size_t len = pbuf_copy_partial(_rxBuffer, buf, size, 0);
    _rxBuffer = pbuf_free_header(_rxBuffer, len);

    // only acknowledge TCP reception, when data was _actually_ handled, if the buffer is close to full
    // (otherwise we overflow in ch32_eth_loop, because there's no signal traversing back through lwip to netif.input, when buffer full, apparently)
    if (_remainingTcpDataLen != 0 && _rxBuffer->tot_len < FREE_BUFFER_THRESHHOLD) {
        tcp_recved(_pcb, _remainingTcpDataLen);
        _remainingTcpDataLen = 0;
    }

    if (_rxBuffer->tot_len == 0) {
        pbuf_free(_rxBuffer);
        _rxBuffer = nullptr;
    }

    return len;
}

int EthernetClient::peek() {
    if (!_pcb || !_rxBuffer) {
        return -1;
    }
    uint8_t c;
    if (pbuf_copy_partial(_rxBuffer, &c, 1, 0) == 1) {
        return c;
    }
    return -1;
}

void EthernetClient::flush() {
    // no flushing, lwip should send stuff immediately
    Ethernet.maintain();
}

void EthernetClient::stop() {
    if (_pcb) {
        tcp_close(_pcb);
        _pcb = nullptr;
        _state = TCP_NONE;
    }
    if (_rxBuffer) {
        pbuf_free(_rxBuffer);
        _rxBuffer = nullptr;
    }
}

uint8_t EthernetClient::connected() {
    uint8_t s = status();
    return ((available() && (s == TCP_CLOSING)) || (s == TCP_CONNECTED) || (s == TCP_ACCEPTED));
}

uint8_t EthernetClient::status() {
    return _state;
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

EthernetClient::operator bool() {
    return (_pcb && (_state != TCP_CLOSING));
}

bool EthernetClient::operator==(const EthernetClient& rhs) {
    return _pcb == rhs._pcb;
}

/* This function is not a function defined by Arduino. This is a function
specific to the W5100 architecture. To keep the compatibility we leave it and
returns always 0. */
uint8_t EthernetClient::getSocketNumber() {
    return 0;
}

