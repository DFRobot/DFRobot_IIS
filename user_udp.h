#ifndef __USER_UDP_H__
#define __USER_UDP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "driver/gpio.h"


#define USER_DEFAULT_PORT        8080                         // UDP server
#define USER_DEFAULT_SERVER_IP   "192.168.1.102"              // UDP client时，远端IP

#define USER_DEFAULT_PKTSIZE     16
#define USER_PACK_BYTE_IS        97      //'a'


char user_udp_server(void);
char user_udp_client(void);
void user_send_recv_data(void *pvParameters);
void user_client_send_recv_data(void *pvParameters);


#ifdef __cplusplus
}
#endif

#endif /*#ifndef __USER_UDP_H__*/