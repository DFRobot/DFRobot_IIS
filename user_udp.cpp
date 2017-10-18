#include "user_udp.h"

#include <string.h>
#include <sys/socket.h>
#include <stdio.h>


static int mysocket;

static struct sockaddr_in remote_addr;   // 远端IP
static unsigned int socklen;

int total_data = 0;
int success_pack = 0;


//---------------------------------------------------------------------------------------------------
//  brief               : 创建一个UDP服务器
//  return              : 1:ok  0:error
//---------------------------------------------------------------------------------------------------
char user_udp_server(void)
{
	struct sockaddr_in server_addr;
	
	printf("Create UDP server, port:%d !\r\n", USER_DEFAULT_PORT);
	
    mysocket = socket(AF_INET, SOCK_DGRAM, 0);     // 创建一个socket
    if (mysocket < 0) 
	{
		return 0;
    }
		
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(USER_DEFAULT_PORT);  // 端口号
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 本地IP,wifi模块的IP
    if (bind(mysocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)   // 绑定本地端口和IP
	{
		close(mysocket);
		
		return 0;
    }
	
    return 1;
}


//---------------------------------------------------------------------------------------------------
//  brief               : 创建一个UDP客户端
//  return              : 1:ok  0:error
//---------------------------------------------------------------------------------------------------
char user_udp_client(void)
{
	//struct sockaddr_in remote_addr;
	
	printf("Create UDP client, connect to: %s  port %d !\r\n", USER_DEFAULT_SERVER_IP, USER_DEFAULT_PORT);
	
    mysocket = socket(AF_INET, SOCK_DGRAM, 0);       // 创建一个socket
    if (mysocket < 0) 
	{
		return 0;
    }	
	
	/*for client remote_addr is also server_addr*/
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(USER_DEFAULT_PORT);
    remote_addr.sin_addr.s_addr = inet_addr(USER_DEFAULT_SERVER_IP);

    return 1;		
}


//---------------------------------------------------------------------------------------------------
//  brief               : UDP收发数据任务
//  pvParameters        :
//  return              : NULL
//---------------------------------------------------------------------------------------------------
void user_send_recv_data(void *pvParameters)
{
	int  len;
    char databuff[USER_DEFAULT_PKTSIZE];
	
	printf("UDP send or receive data task!\r\n");
     
    /*send&receive first packet*/
    socklen = sizeof(remote_addr);
    memset(databuff, USER_PACK_BYTE_IS, USER_DEFAULT_PKTSIZE);

	printf("UDP first receive from:\r\n");
    len = recvfrom(mysocket, databuff, USER_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, &socklen);
	
    if (len > 0) 
	{
		printf("Recv data!\r\n");
		printf("transfer data with %s:%u \r\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
		//xEventGroupSetBits(udp_event_group, UDP_CONNCETED_SUCCESS);
    } 
	else 
	{
		close(mysocket);
    } 
    
    vTaskDelay(500 / portTICK_RATE_MS);
	
	while(1)
	{
	    len = recvfrom(mysocket, databuff, USER_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, &socklen);
		if (len > 0) 
		{
			printf("Recv data: %d\r\n", len);
			printf("transfer data with %s:%u \r\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
			
			printf("Send data: %d\r\n", len);
			sendto(mysocket, databuff, len, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
			//xEventGroupSetBits(udp_event_group, UDP_CONNCETED_SUCCESS);
		}
	}
}


void user_client_send_recv_data(void *pvParameters)
{
	int  len;
    char databuff[USER_DEFAULT_PKTSIZE];
	
	printf("UDP send or receive data task!\r\n");
     
    /*send&receive first packet*/
    socklen = sizeof(remote_addr);
    memset(databuff, USER_PACK_BYTE_IS, USER_DEFAULT_PKTSIZE);

	printf("UDP first send to:\r\n");
	len = sendto(mysocket, databuff, USER_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if (len > 0) 
	{
		//printf("Recv data!\r\n");
		printf("transfer data with %s:%u \r\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
		//xEventGroupSetBits(udp_event_group, UDP_CONNCETED_SUCCESS);
    } 
	else 
	{
		close(mysocket);
    } 
    
    vTaskDelay(500 / portTICK_RATE_MS);
	
	while(1)
	{	
		len = sendto(mysocket, databuff, USER_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
		
		if (len > 0) 
		{
			//printf("Recv data: %d\r\n", len);
			//printf("transfer data with %s:%u \r\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
			
			printf("Send data: %d\r\n", len);
			sendto(mysocket, databuff, len, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
			//xEventGroupSetBits(udp_event_group, UDP_CONNCETED_SUCCESS);
		}
	}
}


