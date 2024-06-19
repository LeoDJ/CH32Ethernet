#pragma once
#include <Client.h>
#include <lwip/tcp.h>

#ifndef MAX_CLIENT
    #define MAX_CLIENT 32
#endif

class EthernetClient : public Client {
  public:
    EthernetClient(){};
    EthernetClient(uint8_t sock){};
    // EthernetClient(tcp_struct_t* tcpClient) : _tcp_client(tcpClient){};

    uint8_t status();
    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char* host, uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t* buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t* buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();
    virtual bool operator==(const bool value) {
        return bool() == value;
    }
    virtual bool operator!=(const bool value) {
        return bool() != value;
    }
    virtual bool operator==(const EthernetClient&);
    virtual bool operator!=(const EthernetClient& rhs) {
        return !this->operator==(rhs);
    };
    uint8_t getSocketNumber();
    virtual uint16_t localPort() {
        return (_pcb->local_port);
    };
    virtual IPAddress remoteIP() {
        return (IPAddress(_pcb->remote_ip.addr));
    };
    virtual uint16_t remotePort() {
        return (_pcb->remote_port);
    };
    void setConnectionTimeout(uint16_t timeout) {
        _connectionTimeout = timeout;
    }

    friend class EthernetServer;

    using Print::write;

    // TCP connection state
    typedef enum {
        TCP_NONE = 0,
        TCP_CONNECTED,
        TCP_RECEIVED,
        TCP_SENT,
        TCP_ACCEPTED,
        TCP_CLOSING,
    } tcp_client_states;

  protected:
    struct tcp_pcb* _pcb = nullptr;
    struct pbuf* _rxBuffer = nullptr;
    uint8_t _state = 0;
    uint16_t _connectionTimeout = 10000;
    uint16_t _remainingTcpDataLen = 0;

    static err_t tcpReceive(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err);
    static void tcpError(void* arg, err_t err);
};