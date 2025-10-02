#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "dhcpserver.h"
#include "dnsserver.h"

#include "settings.h"

#define DEV_NAME "Hall Scanner"
#define FW_VERSION "1.0.0"

#define TCP_PORT 80
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
#define HTML_RESULT_SIZE 8192
#define SET_URL_SEGMENT "/settings"
#define LED_GPIO 0
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s" SET_URL_SEGMENT "\n\n"

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    ip_addr_t gw;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[2048];
    char result[HTML_RESULT_SIZE];
    int header_len;
    int result_len;
    int received_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;

extern int wifi_ap_proc(SETTINGS *set);

#endif