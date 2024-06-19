#pragma once

// Basic config
#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0
#define LWIP_NOASSERT 1

#define LWIP_DHCP 1
#define LWIP_DNS 1

// Memory
#define MEM_SIZE (16 * 1024)
#define MEM_ALIGNMENT 4

// NETIF
#define LWIP_NETIF_HOSTNAME 1

// Checksums need to be calculated manually for V208
#define CHECKSUM_GEN_IP      1
#define CHECKSUM_GEN_UDP     1
#define CHECKSUM_GEN_TCP     1
#define CHECKSUM_GEN_ICMP    1
#define CHECKSUM_GEN_ICMP6   1
#define CHECKSUM_CHECK_IP    1
#define CHECKSUM_CHECK_UDP   1
#define CHECKSUM_CHECK_TCP   1
#define CHECKSUM_CHECK_ICMP  1
#define CHECKSUM_CHECK_ICMP6 1

// HTTP server returns garbage without this
#define HTTP_IS_DATA_VOLATILE(hs) TCP_WRITE_FLAG_COPY

