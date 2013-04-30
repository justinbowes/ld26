// Hacked up one day in March 2013, by Madgarden

#ifndef __UDPNET_H__
#define __UDPNET_H__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xpl.h"

#ifdef WIN32
#include <winsock2.h>
#include <errno.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#ifdef WIN32
#define UN_ACCES		WSAEACCES
#define UN_AGAIN		EAGAIN
#define UN_WOULDBLOCK 	WSAEWOULDBLOCK
#define UN_NOBUFS		WSAENOBUFS
#define UN_CONNRESET	WSAECONNRESET
#define UN_MSGSIZE		WSAEMSGSIZE
#define UN_NETDOWN		WSAENETDOWN
#define UN_NETUNREACH	WSAENETUNREACH
#define UN_DESTADDRREQ	WSAEDESTADDRREQ
#define UN_NOTCONN		WSAENOTCONN
#define UN_NOTSOCK		WSAENOTSOCK
#define UN_OPNOTSUPP	WSAEOPNOTSUPP
#define UN_FAULT		WSAEFAULT
#define UN_HOSTUNREACH	WSAEHOSTUNREACH
#define UN_INTR			WSAEINTR
#else
#define UN_AGAIN		EAGAIN
#define UN_WOULDBLOCK 	EWOULDBLOCK
#define UN_NOBUFS		ENOBUFS
#define UN_CONNRESET	ECONNRESET
#define UN_MSGSIZE		EMSGSIZE
#define UN_NETDOWN		ENETDOWN
#define UN_NETUNREACH	ENETUNREACH
#define UN_DESTADDREQ	EDESTADDREQ
#define UN_NOTCONN		ENOTCONN
#define UN_NOTSOCK		ENOTSOCK
#define UN_OPNOTSUPP	EOPNOTSUPP
#endif

typedef struct
{
	int port;
	char address[20];
} UDPNET_ADDRESS;

// typedef unsigned int SOCKET;

void udp_socket_init(void);
void udp_socket_exit(void);

int udp_create_endpoint(int port);
int udp_close_endpoint(int s);

int udp_send(int s, void *data, int size, const char *address,
	int port);
int udp_receive(int s, void *data, int size, UDPNET_ADDRESS *src);

int udp_error(void);

#endif
