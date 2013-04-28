// Hacked up one day in March 2013, by Madgarden

#ifndef __UDPNET_H__
#define __UDPNET_H__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined WIN32
#include <winsock.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

typedef struct
{
	int port;
	char address[20];
} UDPNET_ADDRESS;

typedef int SOCKET;

void udp_socket_init(void);
void udp_socket_exit(void);

int udp_create_endpoint(int port);
int udp_close_endpoint(int s);

int udp_send(int s, void *data, int size, const char *address,
	int port);
int udp_receive(int s, void *data, int size, UDPNET_ADDRESS *src);

int udp_error(void);

#endif
